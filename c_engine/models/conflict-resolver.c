/** @file conflict-resolver.c
 * A special module, always loaded, that encapsulates the list of units.  It
 * gathers requests for changes to units and disambiguates the results of
 * (potentially) conflicting requests.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.1
 * @date January 2005
 *
 * Copyright &copy; University of Guelph, 2005-2009
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

/* To avoid name clashes when multiple modules have the same interface. */
#define is_singleton conflict_resolver_is_singleton
#define new conflict_resolver_new
#define set_params conflict_resolver_set_params
#define run conflict_resolver_run
#define reset conflict_resolver_reset
#define events_listened_for conflict_resolver_events_listened_for
#define is_listening_for conflict_resolver_is_listening_for
#define has_pending_actions conflict_resolver_has_pending_actions
#define has_pending_infections conflict_resolver_has_pending_infections
#define to_string conflict_resolver_to_string
#define local_printf conflict_resolver_printf
#define local_fprintf conflict_resolver_fprintf
#define local_free conflict_resolver_free
#define handle_before_each_simulation_event conflict_resolver_handle_before_each_simulation_event
#define handle_midnight_event conflict_resolver_handle_midnight_event
#define handle_new_day_event conflict_resolver_handle_new_day_event
#define handle_declaration_of_vaccine_delay_event conflict_resolver_handle_declaration_of_vaccine_delay_event
#define handle_attempt_to_infect_event conflict_resolver_handle_attempt_to_infect_event
#define handle_vaccination_event conflict_resolver_handle_vaccination_event
#define handle_destruction_event conflict_resolver_handle_destruction_event
#define handle_end_of_day_event conflict_resolver_handle_end_of_day_event
#define EVT_free_event_as_GFunc conflict_resolver_EVT_free_event_as_GFunc
#define resolve_conflicts conflict_resolver_resolve_conflicts

#include "model.h"

#if STDC_HEADERS
#  include <string.h>
#endif

#if HAVE_STRINGS_H
#  include <strings.h>
#endif

#if HAVE_MATH_H
#  include <math.h>
#endif

#include "general.h"
#include "conflict-resolver.h"

#ifdef COGRID
double trunc ( double x )
{
  return floor( x );
}
#else
/* Temporary fix -- missing from math header file? */
double trunc (double);
#endif

#define MODEL_NAME "conflict-resolver"



#define NEVENTS_LISTENED_FOR 7
EVT_event_type_t events_listened_for[] = { EVT_BeforeEachSimulation, EVT_Midnight,
  EVT_DeclarationOfVaccineDelay,
  EVT_AttemptToInfect, EVT_Vaccination, EVT_Destruction,
  EVT_EndOfDay
};



/* Specialized information for this model. */
typedef struct
{
  GHashTable *attempts_to_infect; /**< Gathers attempts to infect.  Keys are
    indices into the herd list (unsigned int), values are GSLists. */
  GHashTable *vacc_or_dest; /**< Gathers vaccinations and/or destructions that
    may conflict with infections.  Keys are herds (HRD_herd_t *), values are
    unimportant (presence of a key is all we ever test). */
  GPtrArray *production_types;
  gboolean *vaccine_0_delay; /**< An array of flags, one for each production
    type.  The flag is TRUE if the delay to vaccine immunity for that
    production type is 0. */
}
local_data_t;



/**
 * Wraps EVT_free_event so that it can be used in GLib calls.
 *
 * @param data a pointer to an EVT_event_t structure, but cast to a gpointer.
 * @param user_data not used, pass NULL.
 */
void
EVT_free_event_as_GFunc (gpointer data, gpointer user_data)
{
  EVT_free_event ((EVT_event_t *) data);
}



/**
 * Responds to a declaration of vaccine delay by recording whether the delay is
 * 0 for this production type.  This information is needed so that this module
 * can handle a special case that occurs when the vaccine delay is 0.
 *
 * @param self the model.
 * @param event a declaration of vaccine delay event.
 */
void
handle_declaration_of_vaccine_delay_event (struct naadsm_model_t_ *self,
                                           EVT_declaration_of_vaccine_delay_event_t *event)
{
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER handle_declaration_of_vaccine_delay_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  /* We're only interested if the delay is 0. */
  if (event->delay == 0)
    {
      local_data->vaccine_0_delay[event->production_type] = TRUE;
#if DEBUG
      g_debug ("production type \"%s\" has 0 vaccine delay",
               event->production_type_name);
#endif
    }

#if DEBUG
  g_debug ("----- EXIT handle_declaration_of_vaccine_delay_event (%s)", MODEL_NAME);
#endif
  return;
}



/**
 * Before each simulation, this module sets up the units' initial states.
 *
 * @param self this module.
 * @param herds the list of units.
 * @param queue for any new events this module generates.
 */
void
handle_before_each_simulation_event (struct naadsm_model_t_ * self,
                                     HRD_herd_list_t * herds,
                                     EVT_event_queue_t * queue)
{
  unsigned int nherds, i;
  HRD_herd_t *herd;
  EVT_event_t *event;

#if DEBUG
  g_debug ("----- ENTER handle_before_each_simulation_event (%s)", MODEL_NAME);
#endif

  /* Set each unit's initial state.  We don't need to go through the usual
   * conflict resolution steps here. */
  nherds = HRD_herd_list_length (herds);
  for (i = 0; i < nherds; i++)
    {
      herd = HRD_herd_list_get (herds, i);
      HRD_reset (herd);
      switch (herd->initial_status)
        {
        case Susceptible:
          break;
        case Latent:
        case InfectiousSubclinical:
        case InfectiousClinical:
        case NaturallyImmune:
          event = EVT_new_infection_event (NULL, herd, 0, NAADSM_InitiallyInfected);
          event->u.infection.override_initial_state = herd->initial_status;
          event->u.infection.override_days_in_state = herd->days_in_initial_status;
          event->u.infection.override_days_left_in_state = herd->days_left_in_initial_status;
          EVT_event_enqueue (queue, event);
          break;
        case VaccineImmune:
          event = EVT_new_inprogress_immunity_event (herd, 0, "Ini",
                                                     herd->initial_status,
                                                     herd->days_in_initial_status,
                                                     herd->days_left_in_initial_status);
          EVT_event_enqueue (queue, event);
          break;
        case Destroyed:
          HRD_destroy (herd);
          event = EVT_new_destruction_event (herd, 0, "Ini", -1);
          EVT_event_enqueue (queue, event);
          break;
        default:
          g_assert_not_reached ();
        }
    } /* end of loop over units */

#if DEBUG
  g_debug ("----- EXIT handle_before_each_simulation_event (%s)", MODEL_NAME);
#endif

  return;
} 



/**
 * Responds to a "midnight" event by making the herds change state.
 *
 * @param self this module.
 * @param event the "midnight" event.
 * @param herds the list of herds.
 * @param rng a random number generator.
 * @param queue for any new events this module creates.
 */
void
handle_midnight_event (struct naadsm_model_t_ *self,
                       EVT_midnight_event_t * event,
                       HRD_herd_list_t * herds,
                       RAN_gen_t * rng,
                       EVT_event_queue_t * queue)
{
  local_data_t *local_data;
  unsigned int nherds, i;
  HRD_herd_t *herd;

#if DEBUG
  g_debug ("----- ENTER handle_midnight_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  nherds = HRD_herd_list_length (herds);
  for (i = 0; i < nherds; i++)
    {
      herd = HRD_herd_list_get (herds, i);
      /* _iteration is a global variable defined in general.c */
      HRD_step (herd, _iteration.infectious_herds);
    }

#if DEBUG
  g_debug ("----- EXIT handle_midnight_event (%s)", MODEL_NAME);
#endif
}



/**
 * Responds to an attempt to infect event by recording it.
 *
 * @param self the model.
 * @param event an attempt to infect event.
 */
void
handle_attempt_to_infect_event (struct naadsm_model_t_ *self, EVT_event_t * event)
{
  local_data_t *local_data;
  HRD_herd_t *herd;

#if DEBUG
  g_debug ("----- ENTER handle_attempt_to_infect_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  herd = event->u.attempt_to_infect.infected_herd;
  g_hash_table_insert (local_data->attempts_to_infect,
                       GUINT_TO_POINTER(herd->index),
                       g_slist_prepend (g_hash_table_lookup (local_data->attempts_to_infect, GUINT_TO_POINTER(herd->index)),
                                        EVT_clone_event (event)));
#if DEBUG
  g_debug ("now %u attempt(s) to infect unit \"%s\"",
           g_slist_length (g_hash_table_lookup (local_data->attempts_to_infect, GUINT_TO_POINTER(herd->index))), herd->official_id);
#endif

#if DEBUG
  g_debug ("----- EXIT handle_attempt_to_infect_event (%s)", MODEL_NAME);
#endif

}



/**
 * Responds to a vaccination event by recording that the unit was vaccinated,
 * but only if units of that production type have a zero delay to immunity.
 *
 * @param self the model.
 * @param event a vaccination event.
 */
void
handle_vaccination_event (struct naadsm_model_t_ * self,
                          EVT_vaccination_event_t * event)
{
  local_data_t *local_data;
  HRD_herd_t *herd;

#if DEBUG
  g_debug ("----- ENTER handle_vaccination_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  herd = event->herd;
  if (local_data->vaccine_0_delay[herd->production_type] == TRUE)
    {
#if DEBUG
      /* There should never be more than one Vaccination event, or both
       * Vaccination and Destruction events, for a unit on a single day. */
      g_assert (g_hash_table_lookup (local_data->vacc_or_dest, herd) == NULL);
#endif
      g_hash_table_insert (local_data->vacc_or_dest, herd, herd);
    }

#if DEBUG
  g_debug ("----- EXIT handle_vaccination_event (%s)", MODEL_NAME);
#endif
}



/**
 * Responds to a destruction event by recording the unit that was destroyed.
 *
 * @param self the model.
 * @param event a destruction event.
 */
void
handle_destruction_event (struct naadsm_model_t_ * self,
                          EVT_destruction_event_t * event)
{
  local_data_t *local_data;
  HRD_herd_t *herd;

#if DEBUG
  g_debug ("----- ENTER handle_destruction_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  herd = event->herd;
#if DEBUG
  /* There should never be more than one Destruction event, or both Vaccination
   * and Destruction events, for a unit on a single day. */
  g_assert (g_hash_table_lookup (local_data->vacc_or_dest, herd) == NULL);
#endif
  g_hash_table_insert (local_data->vacc_or_dest, herd, herd);

#if DEBUG
  g_debug ("----- EXIT handle_destruction_event (%s)", MODEL_NAME);
#endif
}



/**
 * A struct to hold arguments for the function resolve_conflicts below.
 * Because that function is typed as a GHFunc, arguments need to be grouped
 * together and passed as a single pointer named user_data.
 */
typedef struct
{
  HRD_herd_list_t *herds;
  GHashTable *vacc_or_dest;
  RAN_gen_t *rng;
  EVT_event_queue_t *queue;
}
resolve_conflicts_args_t;



/**
 * Resolve any conflicts between infection and vaccinatior or destruction for a
 * single unit.  This function is typed as a GHFunc so that it can be called
 * for each key (herd index) in the attempts_to_infect table.
 *
 * @param key a herd (HRD_herd_t * cast to a gpointer).
 * @param value a list of attempts to infect (GSList * in which each list node
 *   contains an EVT_event_t *, cast to a gpointer).
 * @param user_data pointer to a resolve_conflicts_args_t structure.
 */  
void
resolve_conflicts (gpointer key, gpointer value, gpointer user_data)
{
  resolve_conflicts_args_t *args;
  HRD_herd_t *herd;
  GSList *attempts_to_infect;
  unsigned int n;
  unsigned int attempt_num;
  EVT_event_t *attempt, *e;

  args = (resolve_conflicts_args_t *) user_data;
  herd = HRD_herd_list_get (args->herds, GPOINTER_TO_UINT(key));
  attempts_to_infect = (GSList *) value;

  /* If vaccination (with 0 delay to immunity) or destruction has occurred,
   * cancel the infection with probability 1/2.  Note that vaccination and
   * destruction will not both occur. */
  if (g_hash_table_lookup (args->vacc_or_dest, herd) != NULL
      && RAN_num (args->rng) < 0.5)
    {
#if DEBUG
      g_debug ("vaccination or destruction cancels infection for unit \"%s\"", herd->official_id);
#endif
      ;
    }
  else if (!herd->in_disease_cycle)
    {
      /* An infection is going to go ahead.  If there is more than one
       * competing cause of infection, choose one randomly. */
      n = g_slist_length (attempts_to_infect);
      if (n > 1)
        {
          attempt_num = (unsigned int) trunc (RAN_num (args->rng) * n);
          attempt = (EVT_event_t *) (g_slist_nth (attempts_to_infect, attempt_num)->data);
        }
      else
        {
          attempt = (EVT_event_t *) (attempts_to_infect->data);
        }
      e = EVT_new_infection_event (attempt->u.attempt_to_infect.infecting_herd,
                                   herd,
                                   attempt->u.attempt_to_infect.day,
                                   attempt->u.attempt_to_infect.contact_type);
      e->u.infection.override_initial_state =
        attempt->u.attempt_to_infect.override_initial_state;
      e->u.infection.override_days_in_state =
        attempt->u.attempt_to_infect.override_days_in_state;
      e->u.infection.override_days_left_in_state =
        attempt->u.attempt_to_infect.override_days_left_in_state;
      EVT_event_enqueue (args->queue, e);
    }
  /* Free the list of attempts to infect this herd.  This leaves a dangling
   * pointer in the hash table, but that's OK, because the hash table will be
   * cleared out at the end of handle_end_of_day_event. */
  g_slist_foreach (attempts_to_infect, EVT_free_event_as_GFunc, NULL);
  g_slist_free (attempts_to_infect);
  return;
}



/**
 * Responds to an end of day event by resolving competing requests for changes
 * and making one unambiguous change to the herd.
 *
 * @param self the model.
 * @param herds the list of herds.
 * @param rng a random number generator.
 * @param queue for any new events the model creates.
 */
void
handle_end_of_day_event (struct naadsm_model_t_ *self, HRD_herd_list_t * herds,
                         RAN_gen_t * rng, EVT_event_queue_t * queue)
{
  local_data_t *local_data;
  resolve_conflicts_args_t resolve_conflicts_args;

#if DEBUG
  g_debug ("----- ENTER handle_end_of_day_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  resolve_conflicts_args.herds = herds;
  resolve_conflicts_args.vacc_or_dest = local_data->vacc_or_dest;
  resolve_conflicts_args.rng = rng;
  resolve_conflicts_args.queue = queue;
  g_hash_table_foreach (local_data->attempts_to_infect, resolve_conflicts, &resolve_conflicts_args);
  /* Each value (list of attempts to infect a particular herd) in the
   * attempts_to_infect table got freed by resolve_conflicts.  But we still
   * need to clear all the keys (herd indices) from that table. */
  g_hash_table_remove_all (local_data->attempts_to_infect);
  g_hash_table_remove_all (local_data->vacc_or_dest);

#if DEBUG
  g_debug ("----- EXIT handle_end_of_day_event (%s)", MODEL_NAME);
#endif
}



/**
 * Runs this model.
 *
 * @param self the model.
 * @param herds a herd list.
 * @param zones a zone list.
 * @param event the event that caused the model to run.
 * @param rng a random number generator.
 * @param queue for any new events the model creates.
 */
void
run (struct naadsm_model_t_ *self, HRD_herd_list_t * herds, ZON_zone_list_t * zones,
     EVT_event_t * event, RAN_gen_t * rng, EVT_event_queue_t * queue)
{
#if DEBUG
  g_debug ("----- ENTER run (%s)", MODEL_NAME);
#endif

  switch (event->type)
    {
    case EVT_BeforeEachSimulation:
      handle_before_each_simulation_event (self, herds, queue);
      break;
    case EVT_Midnight:
      handle_midnight_event (self, &(event->u.midnight), herds, rng, queue);
      break;
    case EVT_DeclarationOfVaccineDelay:
      handle_declaration_of_vaccine_delay_event (self, &(event->u.declaration_of_vaccine_delay));
      break;
    case EVT_AttemptToInfect:
      handle_attempt_to_infect_event (self, event);
      break;
    case EVT_Vaccination:
      handle_vaccination_event (self, &(event->u.vaccination));
      break;
    case EVT_Destruction:
      handle_destruction_event (self, &(event->u.destruction));
      break;
    case EVT_EndOfDay:
      handle_end_of_day_event (self, herds, rng, queue);
      break;
    default:
      g_error
        ("%s has received a %s event, which it does not listen for.  This should never happen.  Please contact the developer.",
         MODEL_NAME, EVT_event_type_name[event->type]);
    }

#if DEBUG
  g_debug ("----- EXIT run (%s)", MODEL_NAME);
#endif
}



/**
 * Resets this model after a simulation run.
 *
 * @param self the model.
 */
void
reset (struct naadsm_model_t_ *self)
{
#if DEBUG
  g_debug ("----- ENTER reset (%s)", MODEL_NAME);
#endif

  /* Nothing to do. */

#if DEBUG
  g_debug ("----- EXIT reset (%s)", MODEL_NAME);
#endif
}



/**
 * Reports whether this model is listening for a given event type.
 *
 * @param self the model.
 * @param event_type an event type.
 * @return TRUE if the model is listening for the event type.
 */
gboolean
is_listening_for (struct naadsm_model_t_ *self, EVT_event_type_t event_type)
{
  int i;

  for (i = 0; i < self->nevents_listened_for; i++)
    if (self->events_listened_for[i] == event_type)
      return TRUE;
  return FALSE;
}



/**
 * Reports whether this model has any pending actions to carry out.
 *
 * @param self the model.
 * @return TRUE if the model has pending actions.
 */
gboolean
has_pending_actions (struct naadsm_model_t_ * self)
{
  return FALSE;
}



/**
 * Reports whether this model has any pending infections to cause.
 *
 * @param self the model.
 * @return TRUE if the model has pending infections.
 */
gboolean
has_pending_infections (struct naadsm_model_t_ * self)
{
  return FALSE;
}



/**
 * Returns a text representation of this model.
 *
 * @param self the model.
 * @return a string.
 */
char *
to_string (struct naadsm_model_t_ *self)
{
  GString *s;
  char *chararray;

  s = g_string_new (NULL);
  g_string_printf (s, "<%s>", MODEL_NAME);

  /* don't return the wrapper object */
  chararray = s->str;
  g_string_free (s, FALSE);
  return chararray;
}



/**
 * Prints this model to a stream.
 *
 * @param stream a stream to write to.
 * @param self the model.
 * @return the number of characters printed (not including the trailing '\\0').
 */
int
local_fprintf (FILE * stream, struct naadsm_model_t_ *self)
{
  char *s;
  int nchars_written;

  s = to_string (self);
  nchars_written = fprintf (stream, "%s", s);
  free (s);
  return nchars_written;
}



/**
 * Prints this model.
 *
 * @param self the model.
 * @return the number of characters printed (not including the trailing '\\0').
 */
int
local_printf (struct naadsm_model_t_ *self)
{
  return local_fprintf (stdout, self);
}



/**
 * Frees this model.  Does not free the production type name.
 *
 * @param self the model.
 */
void
local_free (struct naadsm_model_t_ *self)
{
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER free (%s)", MODEL_NAME);
#endif

  /* Free the dynamically-allocated parts. */
  local_data = (local_data_t *) (self->model_data);

  g_hash_table_destroy (local_data->attempts_to_infect);
  g_hash_table_destroy (local_data->vacc_or_dest);

  /* Free the list of vaccine delay flags. */
  g_free (local_data->vaccine_0_delay);

  g_free (local_data);
  g_ptr_array_free (self->outputs, TRUE);
  g_free (self);

#if DEBUG
  g_debug ("----- EXIT free (%s)", MODEL_NAME);
#endif
}



/**
 * Returns whether this model is a singleton or not.
 */
gboolean
is_singleton (void)
{
  return TRUE;
}



/**
 * Adds a set of parameters to a conflict resolver model.
 */
void
set_params (struct naadsm_model_t_ *self, PAR_parameter_t * params)
{
#if DEBUG
  g_debug ("----- ENTER set_params (%s)", MODEL_NAME);
#endif

  /* Nothing to do. */

#if DEBUG
  g_debug ("----- EXIT set_params (%s)", MODEL_NAME);
#endif
  return;
}



/**
 * Returns a new conflict resolver model.
 */
naadsm_model_t *
new (scew_element * params, HRD_herd_list_t * herds, projPJ projection,
     ZON_zone_list_t * zones)
{
  naadsm_model_t *self;
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER new (%s)", MODEL_NAME);
#endif

  self = g_new (naadsm_model_t, 1);
  local_data = g_new (local_data_t, 1);

  self->name = MODEL_NAME;
  self->events_listened_for = events_listened_for;
  self->nevents_listened_for = NEVENTS_LISTENED_FOR;
  self->outputs = g_ptr_array_new ();
  self->model_data = local_data;
  self->set_params = set_params;
  self->run = run;
  self->reset = reset;
  self->is_listening_for = is_listening_for;
  self->has_pending_actions = has_pending_actions;
  self->has_pending_infections = has_pending_infections;
  self->to_string = to_string;
  self->printf = local_printf;
  self->fprintf = local_fprintf;
  self->free = local_free;

  local_data->attempts_to_infect = g_hash_table_new (g_direct_hash, g_direct_equal);
  local_data->vacc_or_dest = g_hash_table_new (g_direct_hash, g_direct_equal);
  local_data->production_types = herds->production_type_names;
  local_data->vaccine_0_delay = g_new0 (gboolean, herds->production_type_names->len);

#if DEBUG
  g_debug ("----- EXIT new (%s)", MODEL_NAME);
#endif

  return self;
}

/* end of file conflict-resolver.c */
