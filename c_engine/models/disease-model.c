/** @file disease-model.c
 * Module that encapsulates knowledge about a disease.
 *
 * When a herd is infected, this module changes the herd's state to Latent.  It
 * decides how long the herd will remain latent, infectious without clinical
 * signs, infectious with clinical signs, and immune by sampling from the
 * distributions given as parameters.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Grid Computing Research Group<br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.1
 * @date April 2003
 *
 * Copyright &copy; University of Guelph, 2003-2009
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
#define new disease_model_new
#define run disease_model_run
#define reset disease_model_reset
#define events_listened_for disease_model_events_listened_for
#define is_listening_for disease_model_is_listening_for
#define has_pending_actions disease_model_has_pending_actions
#define has_pending_infections disease_model_has_pending_infections
#define to_string disease_model_to_string
#define local_printf disease_model_printf
#define local_fprintf disease_model_fprintf
#define local_free disease_model_free
#define handle_infection_event disease_model_handle_infection_event

#include "model.h"
#include "model_util.h"

#if STDC_HEADERS
#  include <string.h>
#endif

#if HAVE_STRINGS_H
#  include <strings.h>
#endif

#if HAVE_MATH_H
#  include <math.h>
#endif

#include "disease-model.h"

#if !HAVE_ROUND && HAVE_RINT
#  define round rint
#endif

/* Temporary fix -- "round" and "rint" are in the math library on Red Hat 7.3,
 * but they're #defined so AC_CHECK_FUNCS doesn't find them. */
double round (double x);

/** This must match an element name in the DTD. */
#define MODEL_NAME "disease-model"



#define NEVENTS_LISTENED_FOR 1
EVT_event_type_t events_listened_for[] = { EVT_Infection };



/* Specialized information for this model. */
typedef struct
{
  gboolean *production_type;
  GPtrArray *production_types;
  PDF_dist_t *latent_period;
  PDF_dist_t *infectious_subclinical_period;
  PDF_dist_t *infectious_clinical_period;
  PDF_dist_t *immunity_period;
  REL_chart_t *prevalence;
}
local_data_t;



/**
 * Attaches the relevant prevalence chart to each herd structure.
 *
 * @param self the model.
 * @param herds the herd list.
 */
void
attach_prevalence_charts (struct naadsm_model_t_ *self,
			  HRD_herd_list_t *herds)
{
  local_data_t *local_data;
  HRD_herd_t *herd;
  unsigned int nherds;
  unsigned int i;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,
	 "----- ENTER attach_prevalence_charts (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  
  nherds = HRD_herd_list_length (herds);
  for (i = 0; i < nherds; i++)
    {
      herd = HRD_herd_list_get (herds, i);
      if (local_data->production_type[herd->production_type] == TRUE)
	herd->prevalence_curve = local_data->prevalence;
    }

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,
	 "----- EXIT attach_prevalence_charts (%s)", MODEL_NAME);
#endif
  return;
}



/**
 * Responds to an infection event by changing the herd's state from susceptible
 * to infected.
 *
 * @param self the model.
 * @param event an infection event.
 * @param rng a random number generator.
 */
void
handle_infection_event (struct naadsm_model_t_ *self,
                        EVT_infection_event_t * event, RAN_gen_t * rng)
{
  local_data_t *local_data;
  HRD_herd_t *herd;
  int latent_period, infectious_subclinical_period, infectious_clinical_period, immunity_period;
  unsigned int day_in_disease_cycle;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER handle_infection_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  herd = event->infected_herd;
  if (local_data->production_type[herd->production_type] == FALSE)
    {
#if DEBUG
      g_debug ("unit is %s, sub-model does not apply", herd->production_type_name);
#endif
      goto end;
    }

  day_in_disease_cycle = 0;

  /* Latent period. */
  latent_period = (int) round (PDF_random (local_data->latent_period, rng));
  if (latent_period < 0)
    {
#if DEBUG
      g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,
             "%s distribution returned %i for latent period, using 0 instead",
             PDF_dist_type_name[local_data->latent_period->type], latent_period);
#endif
      latent_period = 0;
    }
  if (event->override_initial_state == Latent)
    {
      if (event->override_days_in_state > 0 && event->override_days_left_in_state > 0)
        {
          /* Override both the days elapsed and the days left in the latent
           * period. */
          latent_period = event->override_days_in_state + event->override_days_left_in_state;
          day_in_disease_cycle = event->override_days_in_state;
        }
      else if (event->override_days_in_state > 0)
        {
          /* Override just the days elapsed in the latent period.  If the given
           * value is greater than the sampled length of the latent period,
           * extend the latent period. */
          latent_period = MAX(latent_period, event->override_days_in_state);
          day_in_disease_cycle = event->override_days_in_state;
        }
      else if (event->override_days_left_in_state > 0)
        {
          /* Override just the days left in the latent period.  If the given
           * value is greater than the sampled length of the latent period,
           * extend the latent period. */
          latent_period = MAX(latent_period, event->override_days_left_in_state);
          day_in_disease_cycle = latent_period - event->override_days_left_in_state;
        }
    }
#if DEBUG
  g_debug ("latent period will last %i days", latent_period);
#endif

  /* Infectious subclinical period. */
  infectious_subclinical_period =
    (int) round (PDF_random (local_data->infectious_subclinical_period, rng));
  if (infectious_subclinical_period < 0)
    {
#if DEBUG
      g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,
             "%s distribution returned %i for infectious subclinical period, using 0 instead",
             PDF_dist_type_name[local_data->infectious_subclinical_period->type],
             infectious_subclinical_period);
#endif
      infectious_subclinical_period = 0;
    }
  if (event->override_initial_state == InfectiousSubclinical)
    {
      day_in_disease_cycle = latent_period;
      if (event->override_days_in_state > 0 && event->override_days_left_in_state > 0)
        {
          /* Override both the days elapsed and the days left in the infectious
           * subclinical period. */
          infectious_subclinical_period = event->override_days_in_state + event->override_days_left_in_state;
          day_in_disease_cycle += event->override_days_in_state;
        }
      else if (event->override_days_in_state > 0)
        {
          /* Override just the days elapsed in the infectious subclinical
           * period.  If the given value is greater than the sampled length of
           * the infectious subclinical period, extend the infectious
           * subclinical period. */
          infectious_subclinical_period = MAX(infectious_subclinical_period, event->override_days_in_state);
          day_in_disease_cycle += event->override_days_in_state;
        }
      else if (event->override_days_left_in_state > 0)
        {
          /* Override just the days left in the infectious subclinical period.
           * If the given value is greater than the sampled length of the
           * infectious subclinical period, extend the infectious subclinical
           * period. */
          infectious_subclinical_period = MAX(infectious_subclinical_period, event->override_days_left_in_state);
          day_in_disease_cycle += (infectious_subclinical_period - event->override_days_left_in_state);
        }
    }
#if DEBUG
  g_debug ("infectiousness (with no visible signs) will last %i days", infectious_subclinical_period);
#endif

  /* Infectious clinical period. */
  infectious_clinical_period =
    (int) round (PDF_random (local_data->infectious_clinical_period, rng));
  if (infectious_clinical_period < 0)
    {
#if DEBUG
      g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,
             "%s distribution returned %i for infectious clinical period, using 0 instead",
             PDF_dist_type_name[local_data->infectious_clinical_period->type],
             infectious_clinical_period);
#endif
      infectious_clinical_period = 0;
    }
  if (event->override_initial_state == InfectiousClinical)
    {
      day_in_disease_cycle = latent_period + infectious_subclinical_period;
      if (event->override_days_in_state > 0 && event->override_days_left_in_state > 0)
        {
          /* Override both the days elapsed and the days left in the infectious
           * clinical period. */
          infectious_clinical_period = event->override_days_in_state + event->override_days_left_in_state;
          day_in_disease_cycle += event->override_days_in_state;
        }
      else if (event->override_days_in_state > 0)
        {
          /* Override just the days elapsed in the infectious clinical period.
           * If the given value is greater than the sampled length of the
           * infectious clinical period, extend the infectious clinical period. */
          infectious_clinical_period = MAX(infectious_clinical_period, event->override_days_in_state);
          day_in_disease_cycle += event->override_days_in_state;
        }
      else if (event->override_days_left_in_state > 0)
        {
          /* Override just the days left in the infectious clinical period.  If
           * the given value is greater than the sampled length of the
           * infectious clinical period, extend the infectious clinical period. */
          infectious_clinical_period = MAX(infectious_clinical_period, event->override_days_left_in_state);
          day_in_disease_cycle += (infectious_clinical_period - event->override_days_left_in_state);
        }
    }
#if DEBUG
  g_debug ("infectiousness (with visible signs) will last %i days", infectious_clinical_period);
#endif

  /* Natural immunity period. */
  immunity_period = (int) round (PDF_random (local_data->immunity_period, rng));
  if (immunity_period < 0)
    {
#if DEBUG
      g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,
             "%s distribution returned %i for immunity period, using 0 instead",
             PDF_dist_type_name[local_data->immunity_period->type], immunity_period);
#endif
      immunity_period = 0;
    }
  if (event->override_initial_state == NaturallyImmune)
    {
      day_in_disease_cycle = latent_period + infectious_subclinical_period + infectious_clinical_period;
      if (event->override_days_in_state > 0 && event->override_days_left_in_state > 0)
        {
          /* Override both the days elapsed and the days left in the immunity
           * period. */
          immunity_period = event->override_days_in_state + event->override_days_left_in_state;
          day_in_disease_cycle += event->override_days_in_state;
        }
      else if (event->override_days_in_state > 0)
        {
          /* Override just the days elapsed in the immunity period.  If the
           * given value is greater than the sampled length of the immunity
           * period, extend the immunity period. */
          immunity_period = MAX(immunity_period, event->override_days_in_state);
          day_in_disease_cycle += event->override_days_in_state;
        }
      else if (event->override_days_left_in_state > 0)
        {
          /* Override just the days left in the immunity period.  If the given
           * value is greater than the sampled length of the immunity period,
           * extend the immunity period. */
          immunity_period = MAX(immunity_period, event->override_days_left_in_state);
          day_in_disease_cycle += (immunity_period - event->override_days_left_in_state);
        }
    }
#if DEBUG
  g_debug ("natural immunity will last %i days", immunity_period);
#endif

  HRD_infect (event->infected_herd, latent_period, infectious_subclinical_period,
              infectious_clinical_period, immunity_period, day_in_disease_cycle);

end:
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT handle_infection_event (%s)", MODEL_NAME);
#endif
  return;
}



/**
 * Runs this model.
 *
 * Side effects: may change the state of one or more herds in list.
 *
 * @param self the model.
 * @param herds a list of herds.
 * @param zones a list of zones.
 * @param event the event that caused the model to run.
 * @param rng a random number generator.
 * @param queue for any new events the model creates.
 */
void
run (struct naadsm_model_t_ *self, HRD_herd_list_t * herds, ZON_zone_list_t * zones,
     EVT_event_t * event, RAN_gen_t * rng, EVT_event_queue_t * queue)
{
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER run (%s)", MODEL_NAME);
#endif

  switch (event->type)
    {
    case EVT_Infection:
      handle_infection_event (self, &(event->u.infection), rng);
      break;
    default:
      g_error
        ("%s has received a %s event, which it does not listen for.  This should never happen.  Please contact the developer.",
         MODEL_NAME, EVT_event_type_name[event->type]);
    }

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT run (%s)", MODEL_NAME);
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
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER reset (%s)", MODEL_NAME);
#endif

  /* Nothing to do. */

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT reset (%s)", MODEL_NAME);
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
  gboolean already_names;
  unsigned int i;
  char *substring, *chararray;
  local_data_t *local_data;

  local_data = (local_data_t *) (self->model_data);
  s = g_string_new (NULL);
  g_string_append_printf (s, "<%s for ", MODEL_NAME);
  already_names = FALSE;
  for (i = 0; i < local_data->production_types->len; i++)
    if (local_data->production_type[i] == TRUE)
      {
        if (already_names)
          g_string_append_printf (s, ",%s",
                                  (char *) g_ptr_array_index (local_data->production_types, i));
        else
          {
            g_string_append_printf (s, "%s",
                                    (char *) g_ptr_array_index (local_data->production_types, i));
            already_names = TRUE;
          }
      }

  substring = PDF_dist_to_string (local_data->latent_period);
  g_string_sprintfa (s, "\n  latent-period=%s\n", substring);
  g_free (substring);

  substring = PDF_dist_to_string (local_data->infectious_subclinical_period);
  g_string_sprintfa (s, "  infectious-subclinical-period=%s\n", substring);
  g_free (substring);

  substring = PDF_dist_to_string (local_data->infectious_clinical_period);
  g_string_sprintfa (s, "  infectious-clinical-period=%s\n", substring);
  g_free (substring);

  substring = PDF_dist_to_string (local_data->immunity_period);
  g_string_sprintfa (s, "  immunity-period=%s", substring);
  g_free (substring);

  if (local_data->prevalence != NULL)
    {
      substring = REL_chart_to_string (local_data->prevalence);
      g_string_append_printf (s, "\n  prevalence=%s", substring);
      g_free (substring);
    }

  g_string_append_c (s, '>');

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

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER fprintf (%s)", MODEL_NAME);
#endif

  s = to_string (self);
  nchars_written = fprintf (stream, "%s", s);
  free (s);

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT fprintf (%s)", MODEL_NAME);
#endif

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
  int nchars_written;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER printf (%s)", MODEL_NAME);
#endif

  nchars_written = local_fprintf (stdout, self);

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT printf (%s)", MODEL_NAME);
#endif

  return nchars_written;
}



/**
 * Frees this model.  Does not free the production type names.
 *
 * @param self the model.
 */
void
local_free (struct naadsm_model_t_ *self)
{
  local_data_t *local_data;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER free (%s)", MODEL_NAME);
#endif

  /* Free the dynamically-allocated parts. */
  local_data = (local_data_t *) (self->model_data);
  g_free (local_data->production_type);
  PDF_free_dist (local_data->latent_period);
  PDF_free_dist (local_data->infectious_subclinical_period);
  PDF_free_dist (local_data->infectious_clinical_period);
  PDF_free_dist (local_data->immunity_period);
  if (local_data->prevalence != NULL)
    REL_free_chart (local_data->prevalence);
  g_free (local_data);
  g_ptr_array_free (self->outputs, TRUE);
  g_free (self);

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT free (%s)", MODEL_NAME);
#endif
}



/**
 * Returns a new disease model.
 */
naadsm_model_t *
new (scew_element * params, HRD_herd_list_t * herds, projPJ projection,
     ZON_zone_list_t * zones)
{
  naadsm_model_t *m;
  local_data_t *local_data;
  scew_element const *e;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER new (%s)", MODEL_NAME);
#endif

  m = g_new (naadsm_model_t, 1);
  local_data = g_new (local_data_t, 1);

  m->name = MODEL_NAME;
  m->events_listened_for = events_listened_for;
  m->nevents_listened_for = NEVENTS_LISTENED_FOR;
  m->outputs = g_ptr_array_new ();
  m->model_data = local_data;
  m->run = run;
  m->reset = reset;
  m->is_listening_for = is_listening_for;
  m->has_pending_actions = has_pending_actions;
  m->has_pending_infections = has_pending_infections;
  m->to_string = to_string;
  m->printf = local_printf;
  m->fprintf = local_fprintf;
  m->free = local_free;

  /* Make sure the right XML subtree was sent. */
  g_assert (strcmp (scew_element_name (params), MODEL_NAME) == 0);

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "setting production types");
#endif
  local_data->production_types = herds->production_type_names;
  local_data->production_type =
    naadsm_read_prodtype_attribute (params, "production-type", herds->production_type_names);

  e = scew_element_by_name (params, "latent-period");
  g_assert (e != NULL);
  local_data->latent_period = PAR_get_PDF (e);

  e = scew_element_by_name (params, "infectious-subclinical-period");
  g_assert (e != NULL);
  local_data->infectious_subclinical_period = PAR_get_PDF (e);

  e = scew_element_by_name (params, "infectious-clinical-period");
  g_assert (e != NULL);
  local_data->infectious_clinical_period = PAR_get_PDF (e);

  e = scew_element_by_name (params, "immunity-period");
  g_assert (e != NULL);
  local_data->immunity_period = PAR_get_PDF (e);

  e = scew_element_by_name (params, "prevalence");
  if (e != NULL)
    {
      local_data->prevalence = PAR_get_relationship_chart (e);
      if (REL_chart_min (local_data->prevalence) < 0)
        {
          g_error ("Y-values <0 are not allowed in a prevalence chart");
        }
      if (REL_chart_max (local_data->prevalence) > 1)
        {
          g_error ("Y-values >1 are not allowed in a prevalence chart");
        }
      /* Scale and translate so that the x-values go from 0 to 1. */
      REL_chart_set_domain (local_data->prevalence, 0, 1);
    }
  else
    {
      /* Don't use prevalence, use the old behaviour. */
      local_data->prevalence = NULL;
    }

  /* Attach the relevant prevalence chart to each herd structure. */
  attach_prevalence_charts (m, herds);

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT new (%s)", MODEL_NAME);
#endif

  return m;
}

/* end of file disease-model.c */
