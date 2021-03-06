/** @file economic-model.c
 * Module that tallies costs of an outbreak.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Grid Computing Research Group<br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.2
 * @date June 2003
 *
 * Copyright &copy; University of Guelph, 2003-2006
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * @todo Add a baseline vaccination capacity and increase in cost past the
 *   baseline.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

/* To avoid name clashes when multiple modules have the same interface. */
#define is_singleton economic_model_is_singleton
#define new economic_model_new
#define set_params economic_model_set_params
#define run economic_model_run
#define reset economic_model_reset
#define events_listened_for economic_model_events_listened_for
#define is_listening_for economic_model_is_listening_for
#define has_pending_actions economic_model_has_pending_actions
#define has_pending_infections economic_model_has_pending_infections
#define to_string economic_model_to_string
#define local_printf economic_model_printf
#define local_fprintf economic_model_fprintf
#define local_free economic_model_free
#define handle_new_day_event economic_model_handle_new_day_event
#define handle_vaccination_event economic_model_handle_vaccination_event
#define handle_destruction_event economic_model_handle_destruction_event

#include "model.h"
#include "model_util.h"

#if STDC_HEADERS
#  include <string.h>
#endif

#if HAVE_STRINGS_H
#  include <strings.h>
#endif

#include "economic-model.h"

/** This must match an element name in the DTD. */
#define MODEL_NAME "economic-model"



#define NEVENTS_LISTENED_FOR 3
EVT_event_type_t events_listened_for[] =
{
  EVT_NewDay,
  EVT_Vaccination,
  EVT_Destruction
};



/* Parameters for the cost of destruction. */
typedef struct
{
  double appraisal;
  double euthanasia;
  double indemnification;
  double carcass_disposal;
  double cleaning_disinfecting;
}
destruction_cost_data_t;



/* Parameters for the cost of vaccination. */
typedef struct
{
  double vaccination_fixed;
  double vaccination;
  unsigned int baseline_capacity, capacity_used;
  double extra_vaccination;
}
vaccination_cost_data_t;



/* Specialized information for this model. */
typedef struct
{
  GPtrArray *production_types; /**< Each item in the list is a char *. */
  ZON_zone_list_t *zones;

  /* Array [production type] of *destruction_cost_data_t.  If an entry
   * is NULL, no parameters are specified for that production type. */
  destruction_cost_data_t **destruction_cost_params;
  /* Array [production type] of *vaccination_cost_data_t.  If an entry
   * is NULL, no parameters are specified for that production type. */
  vaccination_cost_data_t **vaccination_cost_params;
  /* Array [zone type][production type] of double.  If a row is NULL,
   * no parameters are specified for that zone type.  If the
   * surveillance cost is ever reformulated to have more parameters,
   * create a structure as was done for destruction and vaccination
   * costs. */
  double **surveillance_cost_param;

  /*
  RPT_reporting_t *total_cost;
  RPT_reporting_t *appraisal_cost;
  RPT_reporting_t *euthanasia_cost;
  RPT_reporting_t *indemnification_cost;
  RPT_reporting_t *carcass_disposal_cost;
  RPT_reporting_t *cleaning_disinfecting_cost;
  RPT_reporting_t *vaccination_cost;
  RPT_reporting_t *surveillance_cost;
  */
  RPT_reporting_t *cumul_total_cost;
  RPT_reporting_t *cumul_appraisal_cost;
  RPT_reporting_t *cumul_euthanasia_cost;
  RPT_reporting_t *cumul_indemnification_cost;
  RPT_reporting_t *cumul_carcass_disposal_cost;
  RPT_reporting_t *cumul_cleaning_disinfecting_cost;
  RPT_reporting_t *cumul_destruction_subtotal;
  RPT_reporting_t *cumul_vaccination_setup_cost;
  RPT_reporting_t *cumul_vaccination_cost;
  RPT_reporting_t *cumul_vaccination_subtotal;
  RPT_reporting_t *cumul_surveillance_cost;
}
local_data_t;



/**
 * Responds to a new day event by updating the reporting variables.
 *
 * @param self the model.
 * @param herds a list of herds.
 * @param zones a list of zones.
 * @param event a new day event.
 */
void
handle_new_day_event (struct naadsm_model_t_ *self, HRD_herd_list_t * herds,
                      ZON_zone_list_t * zones, EVT_new_day_event_t * event)
{
  local_data_t *local_data = (local_data_t *) (self->model_data);

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER handle_new_day_event (%s)", MODEL_NAME);
#endif

  /* Zero the daily counts. */
  /*
  RPT_reporting_zero (local_data->total_cost);
  RPT_reporting_zero (local_data->appraisal_cost);
  RPT_reporting_zero (local_data->euthanasia_cost);
  RPT_reporting_zero (local_data->indemnification_cost);
  RPT_reporting_zero (local_data->carcass_disposal_cost);
  RPT_reporting_zero (local_data->cleaning_disinfecting_cost);
  RPT_reporting_zero (local_data->vaccination_cost);
  RPT_reporting_zero (local_data->surveillance_cost);
  */

  /* This is an expensive cost calculation, so only do it if the user
   * will see any benefit from it. */
  if ((local_data->surveillance_cost_param) &&
      (/*(local_data->surveillance_cost->frequency != RPT_never ) ||
       (local_data->total_cost->frequency != RPT_never) ||*/
       (local_data->cumul_surveillance_cost->frequency != RPT_never ) ||
       (local_data->cumul_total_cost->frequency != RPT_never)))
    {
      double **surveillance_cost_param = local_data->surveillance_cost_param;
      unsigned int nherds = zones->membership_length;
      unsigned int i;
      double cost = 0;
      
      for (i = 0; i < nherds; i++)
        {
          ZON_zone_t *zone = zones->membership[i]->parent;
          unsigned int zone_index = zone->level - 1;
          HRD_herd_t *herd = HRD_herd_list_get (herds, i);

          if (surveillance_cost_param[zone_index] &&
              /* TODO.  This should probably be the authority's view
               * of whether the herd has been destroyed or not. */
              (herd->status != Destroyed))
            {
              cost =
                surveillance_cost_param[zone_index][herd->production_type] *
                (double) (herd->size);
              /*
              RPT_reporting_add_real (local_data->surveillance_cost, cost, NULL);
              RPT_reporting_add_real (local_data->total_cost, cost, NULL);
              */
              RPT_reporting_add_real (local_data->cumul_surveillance_cost, cost, NULL);
              RPT_reporting_add_real (local_data->cumul_total_cost, cost, NULL);
            }
        }
    }

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT handle_new_day_event (%s)", MODEL_NAME);
#endif
}



/**
 * Responds to a vaccination event by computing its cost.
 *
 * @param self the model.
 * @param event a vaccination event.
 */
void
handle_vaccination_event (struct naadsm_model_t_ *self, EVT_vaccination_event_t * event)
{
  local_data_t *local_data;
  HRD_herd_t *herd;
  double cost, sum = 0;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER handle_vaccination_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  herd = event->herd;

  if (local_data->vaccination_cost_params &&
      local_data->vaccination_cost_params[herd->production_type])
    {
      vaccination_cost_data_t *params = local_data->vaccination_cost_params[herd->production_type];

      /* Fixed cost for the herd. */
      cost = params->vaccination_fixed;
      RPT_reporting_add_real (local_data->cumul_vaccination_setup_cost, cost, NULL);
      sum += cost;

      /* Per-animal cost. */
      cost = params->vaccination * herd->size;

      if (params->capacity_used > params->baseline_capacity)
        {
          cost += params->extra_vaccination * herd->size;
        }
      else
        {
          params->capacity_used += herd->size;
          if (params->capacity_used > params->baseline_capacity)
            cost += params->extra_vaccination *
              (params->capacity_used - params->baseline_capacity);
        }
      /*
      RPT_reporting_add_real (local_data->vaccination_cost, cost, NULL);
      */
      RPT_reporting_add_real (local_data->cumul_vaccination_cost, cost, NULL);
      sum += cost;

      /*
      RPT_reporting_add_real (local_data->total_cost, cost, NULL);
      */
      RPT_reporting_add_real (local_data->cumul_vaccination_subtotal, sum, NULL);
      RPT_reporting_add_real (local_data->cumul_total_cost, sum, NULL);
    }

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT handle_vaccination_event (%s)", MODEL_NAME);
#endif
}



/**
 * Responds to a destruction event by computing its cost.
 *
 * @param self the model.
 * @param event a destruction event.
 */
void
handle_destruction_event (struct naadsm_model_t_ *self, EVT_destruction_event_t * event)
{
  local_data_t *local_data;
  HRD_herd_t *herd;
  unsigned int size;
  double cost, sum = 0;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER handle_destruction_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  herd = event->herd;
  size = herd->size;

  if (local_data->destruction_cost_params &&
      local_data->destruction_cost_params[herd->production_type])
    {
      destruction_cost_data_t *params = local_data->destruction_cost_params[herd->production_type];

      cost = params->appraisal;
      /* RPT_reporting_add_real (local_data->appraisal_cost, cost, NULL); */
      RPT_reporting_add_real (local_data->cumul_appraisal_cost, cost, NULL);
      sum += cost;

      cost = size * params->euthanasia;
      /* RPT_reporting_add_real (local_data->euthanasia_cost, cost, NULL); */
      RPT_reporting_add_real (local_data->cumul_euthanasia_cost, cost, NULL);
      sum += cost;

      cost = size * params->indemnification;
      /* RPT_reporting_add_real (local_data->indemnification_cost, cost, NULL); */
      RPT_reporting_add_real (local_data->cumul_indemnification_cost, cost, NULL);
      sum += cost;

      cost = size * params->carcass_disposal;
      /* RPT_reporting_add_real (local_data->carcass_disposal_cost, cost, NULL); */
      RPT_reporting_add_real (local_data->cumul_carcass_disposal_cost, cost, NULL);
      sum += cost;

      cost = params->cleaning_disinfecting;
      /* RPT_reporting_add_real (local_data->cleaning_disinfecting_cost, cost, NULL); */
      RPT_reporting_add_real (local_data->cumul_cleaning_disinfecting_cost, cost, NULL);
      sum += cost;

      /* RPT_reporting_add_real (local_data->total_cost, sum, NULL); */
      RPT_reporting_add_real (local_data->cumul_destruction_subtotal, sum, NULL);
      RPT_reporting_add_real (local_data->cumul_total_cost, sum, NULL);
    }

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT handle_destruction_event (%s)", MODEL_NAME);
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
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER run (%s)", MODEL_NAME);
#endif

  switch (event->type)
    {
    case EVT_NewDay:
      handle_new_day_event (self, herds, zones, &(event->u.new_day));
      break;
    case EVT_Vaccination:
      handle_vaccination_event (self, &(event->u.vaccination));
      break;
    case EVT_Destruction:
      handle_destruction_event (self, &(event->u.destruction));
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
  local_data_t *local_data = (local_data_t *) (self->model_data);
  unsigned int nprod_types = local_data->production_types->len;
  unsigned int i;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER reset (%s)", MODEL_NAME);
#endif

  /*
  RPT_reporting_zero (local_data->total_cost);
  RPT_reporting_zero (local_data->appraisal_cost);
  RPT_reporting_zero (local_data->euthanasia_cost);
  RPT_reporting_zero (local_data->indemnification_cost);
  RPT_reporting_zero (local_data->carcass_disposal_cost);
  RPT_reporting_zero (local_data->cleaning_disinfecting_cost);
  RPT_reporting_zero (local_data->vaccination_cost);
  RPT_reporting_zero (local_data->surveillance_cost);
  */
  RPT_reporting_zero (local_data->cumul_total_cost);
  RPT_reporting_zero (local_data->cumul_appraisal_cost);
  RPT_reporting_zero (local_data->cumul_euthanasia_cost);
  RPT_reporting_zero (local_data->cumul_indemnification_cost);
  RPT_reporting_zero (local_data->cumul_carcass_disposal_cost);
  RPT_reporting_zero (local_data->cumul_cleaning_disinfecting_cost);
  RPT_reporting_zero (local_data->cumul_destruction_subtotal);
  RPT_reporting_zero (local_data->cumul_vaccination_setup_cost);
  RPT_reporting_zero (local_data->cumul_vaccination_cost);
  RPT_reporting_zero (local_data->cumul_vaccination_subtotal);
  RPT_reporting_zero (local_data->cumul_surveillance_cost);

  if (local_data->vaccination_cost_params)
    {
      for (i = 0; i < nprod_types; i++)
        {
          if (local_data->vaccination_cost_params[i])
            {
              local_data->vaccination_cost_params[i]->capacity_used = 0;
            }
        }
    }

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
  unsigned int i, j;
  char *chararray;
  local_data_t *local_data = (local_data_t *) (self->model_data);
  unsigned int nzones = ZON_zone_list_length (local_data->zones);
  unsigned int nprod_types = local_data->production_types->len;
  destruction_cost_data_t **destruction_cost_params = local_data->destruction_cost_params;
  vaccination_cost_data_t **vaccination_cost_params = local_data->vaccination_cost_params;
  double **surveillance_cost_param = local_data->surveillance_cost_param;

  s = g_string_new (NULL);
  g_string_sprintf (s, "<%s", MODEL_NAME);
  for (i = 0; i < nprod_types; i++)
    {
      if ((destruction_cost_params &&
           destruction_cost_params[i]) ||
          (vaccination_cost_params &&
           vaccination_cost_params[i]))
        {
          g_string_append_printf (s, "\n  for %s",
                                  (char *) g_ptr_array_index (local_data->production_types, i));

          if (destruction_cost_params &&
              destruction_cost_params[i])
            {
              destruction_cost_data_t *params = local_data->destruction_cost_params[i];

              g_string_sprintfa (s, "\n    appraisal (per unit)=%g\n", params->appraisal);
              g_string_sprintfa (s, "    euthanasia (per animal)=%g\n", params->euthanasia);
              g_string_sprintfa (s, "    indemnification (per animal)=%g\n", params->indemnification);
              g_string_sprintfa (s, "    carcass-disposal (per animal)=%g\n", params->carcass_disposal);
              g_string_sprintfa (s, "    cleaning-disinfecting (per unit)=%g", params->cleaning_disinfecting);
            }

          if (vaccination_cost_params &&
              vaccination_cost_params[i])
            {
              vaccination_cost_data_t *params = local_data->vaccination_cost_params[i];

              g_string_sprintfa (s, "\n    vaccination-fixed (per unit)=%g\n", params->vaccination_fixed);
              g_string_sprintfa (s, "    vaccination (per animal)=%g\n", params->vaccination);
              g_string_sprintfa (s, "    baseline-capacity=%u\n", params->baseline_capacity);
              g_string_sprintfa (s, "    additional-vaccination (per animal)=%g", params->extra_vaccination);
            }
        }
    }

  if (surveillance_cost_param)
    {
      for (i = 0; i < nzones; i++)
        {
          if (surveillance_cost_param[i])
            {
              for (j = 0; j < nprod_types; j++)
                {
                  if (surveillance_cost_param[i][j] != 0)
                    {
                      g_string_append_printf (s, "\n for %s in %s",
                                              (char *) g_ptr_array_index (local_data->production_types, j),
                                              ZON_zone_list_get (local_data->zones, i)->name);
                      g_string_sprintfa (s, "\n  surveillance (per animal, per day)=%g", surveillance_cost_param[i][j]);
                    }
                }
            }
        }
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
  local_data_t *local_data = (local_data_t *) (self->model_data);
  unsigned int nzones = ZON_zone_list_length (local_data->zones);
  unsigned int nprod_types = local_data->production_types->len;
  unsigned int i;

#if DEBUG
  g_debug ("----- ENTER free (%s)", MODEL_NAME);
#endif

  /* Free the dynamically-allocated parts. */
  if (local_data->destruction_cost_params)
    {
      for (i = 0; i < nprod_types; i++)
        {
          if (local_data->destruction_cost_params[i])
            {
              g_free (local_data->destruction_cost_params[i]);
              local_data->destruction_cost_params[i] = NULL;
            }
        }
      g_free (local_data->destruction_cost_params);
      local_data->destruction_cost_params = NULL;
    }

  if (local_data->vaccination_cost_params)
    {
      for (i = 0; i < nprod_types; i++)
        {
          if (local_data->vaccination_cost_params[i])
            {
              g_free (local_data->vaccination_cost_params[i]);
              local_data->vaccination_cost_params[i] = NULL;
            }
        }
      g_free (local_data->vaccination_cost_params);
      local_data->vaccination_cost_params = NULL;
    }

  if (local_data->surveillance_cost_param)
    {
      for (i = 0; i < nzones; i++)
        {
          if (local_data->surveillance_cost_param[i])
            {
              g_free (local_data->surveillance_cost_param[i]);
              local_data->surveillance_cost_param[i] = NULL;
            }
        }
      g_free (local_data->surveillance_cost_param);
      local_data->surveillance_cost_param = NULL;
    }

  /*
  RPT_free_reporting (local_data->total_cost);
  RPT_free_reporting (local_data->appraisal_cost);
  RPT_free_reporting (local_data->euthanasia_cost);
  RPT_free_reporting (local_data->indemnification_cost);
  RPT_free_reporting (local_data->carcass_disposal_cost);
  RPT_free_reporting (local_data->cleaning_disinfecting_cost);
  RPT_free_reporting (local_data->vaccination_cost);
  RPT_free_reporting (local_data->surveillance_cost);
  */
  RPT_free_reporting (local_data->cumul_total_cost);
  RPT_free_reporting (local_data->cumul_appraisal_cost);
  RPT_free_reporting (local_data->cumul_euthanasia_cost);
  RPT_free_reporting (local_data->cumul_indemnification_cost);
  RPT_free_reporting (local_data->cumul_carcass_disposal_cost);
  RPT_free_reporting (local_data->cumul_cleaning_disinfecting_cost);
  RPT_free_reporting (local_data->cumul_destruction_subtotal);
  RPT_free_reporting (local_data->cumul_vaccination_setup_cost);
  RPT_free_reporting (local_data->cumul_vaccination_cost);
  RPT_free_reporting (local_data->cumul_vaccination_subtotal);
  RPT_free_reporting (local_data->cumul_surveillance_cost);

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
 * Adds a set of parameters to a contact spread model.
 */
void
set_params (struct naadsm_model_t_ *self, PAR_parameter_t * params)
{
  local_data_t *local_data = (local_data_t *) (self->model_data);
  unsigned int nzones = ZON_zone_list_length (local_data->zones);
  unsigned int nprod_types = local_data->production_types->len;
  scew_element *e, **ee;
  gboolean success;
  gboolean has_destruction_cost_params = FALSE;
  gboolean has_vaccination_cost_params = FALSE;
  gboolean has_surveillance_cost_param = FALSE;
  destruction_cost_data_t destruction_cost_params = {};
  vaccination_cost_data_t vaccination_cost_params = {};
  double surveillance_cost_param = 0;
  gboolean *production_type = NULL;
  gboolean *zone = NULL;
  unsigned int i, j;
  unsigned int noutputs;
  RPT_reporting_t *output;
  const XML_Char *variable_name;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER set_params (%s)", MODEL_NAME);
#endif

  /* Make sure the right XML subtree was sent. */
  g_assert (strcmp (scew_element_name (params), MODEL_NAME) == 0);

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "setting production types");
#endif

  /* Destruction Cost Parameters */
  e = scew_element_by_name (params, "appraisal");
  if (e != NULL)
    {
      destruction_cost_params.appraisal = PAR_get_money (e, &success);
      if (success)
        {
          has_destruction_cost_params = TRUE;
        }
      else
        {
          g_warning ("%s: setting per-unit appraisal cost to 0", MODEL_NAME);
          destruction_cost_params.appraisal = 0;
        }
    }
  else
    {
      g_warning ("%s: per-unit appraisal cost missing, setting to 0", MODEL_NAME);
      destruction_cost_params.appraisal = 0;
    }

  e = scew_element_by_name (params, "euthanasia");
  if (e != NULL)
    {
      destruction_cost_params.euthanasia = PAR_get_money (e, &success);
      if (success)
        {
          has_destruction_cost_params = TRUE;
        }
      else
        {
          g_warning ("%s: setting per-animal euthanasia cost to 0", MODEL_NAME);
          destruction_cost_params.euthanasia = 0;
        }
    }
  else
    {
      g_warning ("%s: per-animal euthanasia cost missing, setting to 0", MODEL_NAME);
      destruction_cost_params.euthanasia = 0;
    }

  e = scew_element_by_name (params, "indemnification");
  if (e != NULL)
    {
      destruction_cost_params.indemnification = PAR_get_money (e, &success);
      if (success)
        {
          has_destruction_cost_params = TRUE;
        }
      else
        {
          g_warning ("%s: setting per-animal indemnification cost to 0", MODEL_NAME);
          destruction_cost_params.indemnification = 0;
        }
    }
  else
    {
      g_warning ("%s: per-animal indemnification cost missing, setting to 0", MODEL_NAME);
      destruction_cost_params.indemnification = 0;
    }

  e = scew_element_by_name (params, "carcass-disposal");
  if (e != NULL)
    {
      destruction_cost_params.carcass_disposal = PAR_get_money (e, &success);
      if (success)
        {
          has_destruction_cost_params = TRUE;
        }
      else
        {
          g_warning ("%s: setting per-animal carcass disposal cost to 0", MODEL_NAME);
          destruction_cost_params.carcass_disposal = 0;
        }
    }
  else
    {
      g_warning ("%s: per-animal carcass disposal cost missing, setting to 0", MODEL_NAME);
      destruction_cost_params.carcass_disposal = 0;
    }

  e = scew_element_by_name (params, "cleaning-disinfecting");
  if (e != NULL)
    {
      destruction_cost_params.cleaning_disinfecting = PAR_get_money (e, &success);
      if (success)
        {
          has_destruction_cost_params = TRUE;
        }
      else
        {
          g_warning ("%s: setting per-unit cleaning and disinfecting cost to 0", MODEL_NAME);
          destruction_cost_params.cleaning_disinfecting = 0;
        }
    }
  else
    {
      g_warning ("%s: per-unit cleaning and disinfecting cost missing, setting to 0", MODEL_NAME);
      destruction_cost_params.cleaning_disinfecting = 0;
    }

  /* Vaccination Cost Parameters */
  e = scew_element_by_name (params, "vaccination-fixed");
  if (e != NULL)
    {
      vaccination_cost_params.vaccination_fixed = PAR_get_money (e, &success);
      if (success)
        {
          has_vaccination_cost_params = TRUE;
        }
      else
        {
          g_warning ("%s: setting per-unit vaccination cost to 0", MODEL_NAME);
          vaccination_cost_params.vaccination_fixed = 0;
        }
    }
  else
    {
      g_warning ("%s: per-unit vaccination cost missing, setting to 0", MODEL_NAME);
      vaccination_cost_params.vaccination_fixed = 0;
    }

  e = scew_element_by_name (params, "vaccination");
  if (e != NULL)
    {
      vaccination_cost_params.vaccination = PAR_get_money (e, &success);
      if (success)
        {
          has_vaccination_cost_params = TRUE;
        }
      else
        {
          g_warning ("%s: setting per-animal vaccination cost to 0", MODEL_NAME);
          vaccination_cost_params.vaccination = 0;
        }
    }
  else
    {
      g_warning ("%s: per-animal vaccination cost missing, setting to 0", MODEL_NAME);
      vaccination_cost_params.vaccination = 0;
    }

  e = scew_element_by_name (params, "baseline-vaccination-capacity");
  if (e != NULL)
    {
      vaccination_cost_params.baseline_capacity = (unsigned int) PAR_get_unitless (e, &success);
      if (success)
        {
          has_vaccination_cost_params = TRUE;
        }
      else
        {
          g_warning ("%s: setting baseline vaccination capacity to 1,000,000", MODEL_NAME);
          vaccination_cost_params.baseline_capacity = 1000000;
        }
    }
  else
    {
      g_warning ("%s: baseline vaccination capacity missing, setting to 1,000,000", MODEL_NAME);
      vaccination_cost_params.baseline_capacity = 1000000;
    }

  e = scew_element_by_name (params, "additional-vaccination");
  if (e != NULL)
    {
      vaccination_cost_params.extra_vaccination = PAR_get_money (e, &success);
      if (success)
        {
          has_vaccination_cost_params = TRUE;
        }
      else
        {
          g_warning ("%s: setting additional per-animal vaccination cost to 0", MODEL_NAME);
          vaccination_cost_params.extra_vaccination = 0;
        }
    }
  else
    {
      g_warning ("%s: additional per-animal vaccination cost missing, setting to 0", MODEL_NAME);
      vaccination_cost_params.extra_vaccination = 0;
    }

  /* No vaccinations have been performed yet. */
  vaccination_cost_params.capacity_used = 0;

  /* Surveillance Cost Parameters */
  e = scew_element_by_name (params, "surveillance");
  if (e != NULL)
    {
      surveillance_cost_param = PAR_get_money (e, &success);
      if (success)
        {
          has_surveillance_cost_param = TRUE;
        }
      else
        {
          g_warning ("%s: setting per-animal zone surveillance cost to 0", MODEL_NAME);
          surveillance_cost_param = 0;
        }
    }
  else
    {
      g_warning ("%s: per-animal zone surveillance cost missing, setting to 0", MODEL_NAME);
      surveillance_cost_param = 0;
    }

  /* Set the reporting frequency for the output variables. */
  ee = scew_element_list (params, "output", &noutputs);
#if DEBUG
  g_debug ("%i output variables", noutputs);
#endif
  for (i = 0; i < noutputs; i++)
    {
      e = ee[i];
      variable_name = scew_element_contents (scew_element_by_name (e, "variable-name"));
      /* Do the outputs include a variable with this name? */
      for (j = 0; j < self->outputs->len; j++)
        {
          output = (RPT_reporting_t *) g_ptr_array_index (self->outputs, j);
          if (strcmp (output->name, variable_name) == 0)
            break;
        }
      if (j == self->outputs->len)
        g_warning ("no output variable named \"%s\", ignoring", variable_name);
      else
        {
          RPT_reporting_set_frequency (output,
                                       RPT_string_to_frequency (scew_element_contents
                                                                (scew_element_by_name
                                                                 (e, "frequency"))));
#if DEBUG
          g_debug ("report \"%s\" %s", variable_name, RPT_frequency_name[output->frequency]);
#endif
        }
    }
  free (ee);

  /* Read zone and production type attributes to determine which
   * entries should be filled in. */
  if (scew_attribute_by_name (params, "zone") != NULL)
    zone = naadsm_read_zone_attribute (params, local_data->zones);
  else
    zone = NULL;
  production_type =
    naadsm_read_prodtype_attribute (params, "production-type", local_data->production_types);

  /* Copy the parameters into the selected zone and production types. */
  for (i = 0; i < nprod_types; i++)
    {
      if (production_type[i])
        {
          if (has_destruction_cost_params)
            {
              if (local_data->destruction_cost_params == NULL)
                {
                  local_data->destruction_cost_params =
                    g_new0 (destruction_cost_data_t *, nprod_types);
                  g_assert (local_data->destruction_cost_params != NULL);
                }
              if (local_data->destruction_cost_params[i] == NULL)
                {
                  local_data->destruction_cost_params[i] =
                    g_new0 (destruction_cost_data_t, 1);
                  g_assert (local_data->destruction_cost_params[i] != NULL);
                }
              memcpy (local_data->destruction_cost_params[i],
                      &destruction_cost_params,
                      sizeof (destruction_cost_data_t));
            }
          if (has_vaccination_cost_params)
            {
              if (local_data->vaccination_cost_params == NULL)
                {
                  local_data->vaccination_cost_params =
                    g_new0 (vaccination_cost_data_t *, nprod_types);
                  g_assert (local_data->vaccination_cost_params != NULL);
                }
              if (local_data->vaccination_cost_params[i] == NULL)
                {
                  local_data->vaccination_cost_params[i] =
                    g_new0 (vaccination_cost_data_t, 1);
                  g_assert (local_data->vaccination_cost_params[i] != NULL);
                }
              memcpy (local_data->vaccination_cost_params[i],
                      &vaccination_cost_params,
                      sizeof (vaccination_cost_data_t));
            }

          if (has_surveillance_cost_param)
            {
              if (zone)
                {
                  for (j = 0; j < nzones; j++)
                    {
                      if (zone[j])
                        {
                          if (local_data->surveillance_cost_param == NULL)
                            {
                              local_data->surveillance_cost_param =
                                g_new0 (double *, nzones);
                              g_assert (local_data->surveillance_cost_param != NULL);
                            }
                          if (local_data->surveillance_cost_param[j] == NULL)
                            {
                              local_data->surveillance_cost_param[j] =
                                g_new0 (double, nprod_types);
                              g_assert (local_data->surveillance_cost_param[j] != NULL);
                            }
                          local_data->surveillance_cost_param[j][i] = surveillance_cost_param;
                        }
                    }
                }
              else
                {
                  g_warning ("%s: ignoring given surveillance cost, because no zone was specified", MODEL_NAME);
                }
            }
        }
    }

  g_free (production_type);
  if (zone != NULL)
    g_free (zone);

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT set_params (%s)", MODEL_NAME);
#endif

  return;
}



/**
 * Returns a new economic model.
 */
naadsm_model_t *
new (scew_element * params, HRD_herd_list_t * herds, projPJ projection,
     ZON_zone_list_t * zones)
{
  naadsm_model_t *m;
  local_data_t *local_data;

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
  m->set_params = set_params;
  m->run = run;
  m->reset = reset;
  m->is_listening_for = is_listening_for;
  m->has_pending_actions = has_pending_actions;
  m->has_pending_infections = has_pending_infections;
  m->to_string = to_string;
  m->printf = local_printf;
  m->fprintf = local_fprintf;
  m->free = local_free;

  /* local_data->destruction_cost_params, ->vaccination_cost_params
   * hold 1D arrays of pointers to parameter blocks (indexed by
   * [production_type]).  local_data->surveillance_cost_param holds a
   * 2D array of pointers to parameter blocks (indexed by
   * [zone_type][production_type]).  Initially, all pointers are
   * NULL.  Pointers will be created as needed in the set_params
   * function. */
  local_data->production_types = herds->production_type_names;
  local_data->zones = zones;
  local_data->destruction_cost_params = NULL;
  local_data->vaccination_cost_params = NULL;
  local_data->surveillance_cost_param = NULL;

  /*
  local_data->total_cost = RPT_new_reporting ("total-cost", RPT_real, RPT_never);
  local_data->appraisal_cost =
    RPT_new_reporting ("appraisal-cost", RPT_real, RPT_never);
  local_data->euthanasia_cost =
    RPT_new_reporting ("euthanasia-cost", RPT_real, RPT_never);
  local_data->indemnification_cost =
    RPT_new_reporting ("indemnification-cost", RPT_real, RPT_never);
  local_data->carcass_disposal_cost =
    RPT_new_reporting ("carcass-disposal-cost", RPT_real, RPT_never);
  local_data->cleaning_disinfecting_cost =
    RPT_new_reporting ("cleaning-and-disinfecting-cost", RPT_real, RPT_never);
  local_data->vaccination_cost =
    RPT_new_reporting ("vaccination-cost", RPT_real, RPT_never);
  local_data->surveillance_cost =
    RPT_new_reporting ("surveillance-cost", RPT_real, RPT_never);
  */
  local_data->cumul_total_cost =
    RPT_new_reporting ("costsTotal", RPT_real, RPT_never);
  local_data->cumul_appraisal_cost =
    RPT_new_reporting ("destrAppraisal", RPT_real, RPT_never);
  local_data->cumul_euthanasia_cost =
    RPT_new_reporting ("destrEuthanasia", RPT_real, RPT_never);
  local_data->cumul_indemnification_cost =
    RPT_new_reporting ("destrIndemnification", RPT_real, RPT_never);
  local_data->cumul_carcass_disposal_cost =
    RPT_new_reporting ("destrDisposal", RPT_real, RPT_never);
  local_data->cumul_cleaning_disinfecting_cost =
    RPT_new_reporting ("destrCleaning", RPT_real, RPT_never);
  local_data->cumul_destruction_subtotal =
    RPT_new_reporting ("destrSubtotal", RPT_real, RPT_never);
  local_data->cumul_vaccination_setup_cost =
    RPT_new_reporting ("vaccSetup", RPT_real, RPT_never);
  local_data->cumul_vaccination_cost =
    RPT_new_reporting ("vaccVaccination", RPT_real, RPT_never);
  local_data->cumul_vaccination_subtotal =
    RPT_new_reporting ("vaccSubtotal", RPT_real, RPT_never);
  local_data->cumul_surveillance_cost =
    RPT_new_reporting ("costSurveillance", RPT_real, RPT_never);
  /*
  g_ptr_array_add (m->outputs, local_data->total_cost);
  g_ptr_array_add (m->outputs, local_data->appraisal_cost);
  g_ptr_array_add (m->outputs, local_data->euthanasia_cost);
  g_ptr_array_add (m->outputs, local_data->indemnification_cost);
  g_ptr_array_add (m->outputs, local_data->carcass_disposal_cost);
  g_ptr_array_add (m->outputs, local_data->cleaning_disinfecting_cost);
  g_ptr_array_add (m->outputs, local_data->vaccination_cost);
  g_ptr_array_add (m->outputs, local_data->surveillance_cost);
  */
  g_ptr_array_add (m->outputs, local_data->cumul_total_cost);
  g_ptr_array_add (m->outputs, local_data->cumul_appraisal_cost);
  g_ptr_array_add (m->outputs, local_data->cumul_euthanasia_cost);
  g_ptr_array_add (m->outputs, local_data->cumul_indemnification_cost);
  g_ptr_array_add (m->outputs, local_data->cumul_carcass_disposal_cost);
  g_ptr_array_add (m->outputs, local_data->cumul_cleaning_disinfecting_cost);
  g_ptr_array_add (m->outputs, local_data->cumul_destruction_subtotal);
  g_ptr_array_add (m->outputs, local_data->cumul_vaccination_setup_cost);
  g_ptr_array_add (m->outputs, local_data->cumul_vaccination_cost);
  g_ptr_array_add (m->outputs, local_data->cumul_vaccination_subtotal);
  g_ptr_array_add (m->outputs, local_data->cumul_surveillance_cost);

  /* Send the XML subtree to the init function to read the production type
   * combination specific parameters. */
  m->set_params (m, params);

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT new (%s)", MODEL_NAME);
#endif

  return m;
}

/* end of file economic-model.c */
