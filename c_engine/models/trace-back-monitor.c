/** @file trace-back-monitor.c
 * Tracks the number of attempted and successful trace backs.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Grid Computing Research Group<br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.1
 * @date October 2005
 *
 * Copyright &copy; University of Guelph, 2005-2009
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */


/*
 * NOTE: This module is DEPRECATED, and is included only for purposes of backward
 * compatibility with parameter files from NAADSM 3.0 - 3.1.x.  Any new 
 * development should be done elsewhere: see trace-monitor.
*/


#if HAVE_CONFIG_H
#  include <config.h>
#endif

/* To avoid name clashes when multiple modules have the same interface. */
#define new trace_back_monitor_new
#define run trace_back_monitor_run
#define reset trace_back_monitor_reset
#define events_listened_for trace_back_monitor_events_listened_for
#define is_listening_for trace_back_monitor_is_listening_for
#define has_pending_actions trace_back_monitor_has_pending_actions
#define has_pending_infections trace_back_monitor_has_pending_infections
#define to_string trace_back_monitor_to_string
#define local_printf trace_back_monitor_printf
#define local_fprintf trace_back_monitor_fprintf
#define local_free trace_back_monitor_free
#define handle_before_any_simulations_event trace_back_monitor_handle_before_any_simulations_event
#define handle_new_day_event trace_back_monitor_handle_new_day_event
#define handle_trace_result_event trace_back_monitor_handle_trace_result_event

#include "model.h"

#if STDC_HEADERS
#  include <string.h>
#endif

#include "trace-back-monitor.h"

#include "naadsm.h"

/** This must match an element name in the DTD. */
#define MODEL_NAME "trace-back-monitor"



#define NEVENTS_LISTENED_FOR 3
EVT_event_type_t events_listened_for[] = { EVT_BeforeAnySimulations, EVT_NewDay, EVT_TraceResult };



/** Specialized information for this model. */
typedef struct
{
  GPtrArray *production_types;
  RPT_reporting_t *nunits_potentially_traced;
  RPT_reporting_t *nunits_potentially_traced_by_contacttype;
  RPT_reporting_t *nunits_potentially_traced_by_prodtype;
  RPT_reporting_t *nunits_potentially_traced_by_contacttype_and_prodtype;
  RPT_reporting_t *cumul_nunits_potentially_traced;
  RPT_reporting_t *cumul_nunits_potentially_traced_by_contacttype;
  RPT_reporting_t *cumul_nunits_potentially_traced_by_prodtype;
  RPT_reporting_t *cumul_nunits_potentially_traced_by_contacttype_and_prodtype;
  RPT_reporting_t *nunits_traced;
  RPT_reporting_t *nunits_traced_by_contacttype;
  RPT_reporting_t *nunits_traced_by_prodtype;
  RPT_reporting_t *nunits_traced_by_contacttype_and_prodtype;
  RPT_reporting_t *cumul_nunits_traced;
  RPT_reporting_t *cumul_nunits_traced_by_contacttype;
  RPT_reporting_t *cumul_nunits_traced_by_prodtype;
  RPT_reporting_t *cumul_nunits_traced_by_contacttype_and_prodtype;
  /* These two arrays are needed to form variable names like "trcUDirp" and
   * "trcUIndPigsp". */
  char *contact_type_name_with_p[NAADSM_NCONTACT_TYPES];
  char **production_type_name_with_p;
}
local_data_t;



/**
 * Before any simulations, this module announces the output variables it is
 * recording.
 *
 * @param self this module.
 * @param queue for any new events this function creates.
 */
void
handle_before_any_simulations_event (struct naadsm_model_t_ *self,
                                     EVT_event_queue_t *queue)
{
  unsigned int n, i;
  RPT_reporting_t *output;
  GPtrArray *outputs = NULL;

  n = self->outputs->len;
  for (i = 0; i < n; i++)
    {
      output = (RPT_reporting_t *) g_ptr_array_index (self->outputs, i);
      if (output->frequency != RPT_never)
        {
          if (outputs == NULL)
            outputs = g_ptr_array_new();
          g_ptr_array_add (outputs, output);
        }
    }

  if (outputs != NULL)
    EVT_event_enqueue (queue, EVT_new_declaration_of_outputs_event (outputs));
  /* We don't free the pointer array, that will be done when the event is freed
   * after all interested modules have processed it. */

  return;
}



/**
 * First thing on a new day, zero the counts.
 *
 * @param self the model.
 */
void
handle_new_day_event (struct naadsm_model_t_ *self)
{
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER handle_new_day_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  /* Zero the daily counts. */
  RPT_reporting_zero (local_data->nunits_potentially_traced);
  RPT_reporting_zero (local_data->nunits_potentially_traced_by_contacttype);
  RPT_reporting_zero (local_data->nunits_potentially_traced_by_prodtype);
  RPT_reporting_zero (local_data->nunits_potentially_traced_by_contacttype_and_prodtype);
  RPT_reporting_zero (local_data->nunits_traced);
  RPT_reporting_zero (local_data->nunits_traced_by_contacttype);
  RPT_reporting_zero (local_data->nunits_traced_by_prodtype);
  RPT_reporting_zero (local_data->nunits_traced_by_contacttype_and_prodtype);

#if DEBUG
  g_debug ("----- EXIT handle_new_day_event (%s)", MODEL_NAME);
#endif
}



/**
 * Responds to a trace result event by recording it.
 *
 * @param self the model.
 * @param event a trace result event.
 */
void
handle_trace_result_event (struct naadsm_model_t_ *self, EVT_trace_result_event_t *event)
{
  local_data_t *local_data;
  HRD_herd_t *identified_herd, *origin_herd;
  const char *contact_type_name;
  const char *drill_down_list[3] = { NULL, NULL, NULL };
  HRD_trace_t trace;
  
  #if DEBUG
    g_debug ("----- ENTER handle_trace_result_event (%s)", MODEL_NAME);
  #endif

  identified_herd = (event->direction == NAADSM_TraceForwardOrOut) ? event->exposed_herd : event->exposing_herd;
  origin_herd = (event->direction == NAADSM_TraceForwardOrOut) ? event->exposing_herd : event->exposed_herd; 
  
  /* Record the trace in the GUI */
  /* --------------------------- */
  trace.day = (int) event->day;
  trace.initiated_day = (int) event->initiated_day;
  
  trace.identified_index = identified_herd->index;
  trace.identified_status = (NAADSM_disease_state) identified_herd->status;
  
  trace.origin_index = origin_herd->index; 
  trace.origin_status = (NAADSM_disease_state) origin_herd->status;
  
  trace.trace_type = event->direction;
  trace.contact_type = event->contact_type;
  if (trace.contact_type != NAADSM_DirectContact
      && trace.contact_type != NAADSM_IndirectContact)
    {
      g_error( "Bad contact type in contact-recorder-model.attempt_trace_event" );
      trace.contact_type = 0;
    }

  if( TRUE == event->traced )
    trace.success = NAADSM_SuccessTrue;
  else
    trace.success = NAADSM_SuccessFalse; 

  #ifdef USE_SC_GUILIB
    sc_trace_herd( event->exposed_herd, trace );
  #else
    if (NULL != naadsm_trace_herd)
      naadsm_trace_herd (trace);
  #endif


  /* Record the trace in the SC version */
  /* ---------------------------------- */
  local_data = (local_data_t *) (self->model_data);
  
  contact_type_name = NAADSM_contact_type_abbrev[event->contact_type];
  drill_down_list[0] = contact_type_name;

  /* Record a potentially traced contact. */
  drill_down_list[1] = local_data->production_type_name_with_p[identified_herd->production_type];
  RPT_reporting_add_integer (local_data->nunits_potentially_traced, 1, NULL);
  if (local_data->nunits_potentially_traced_by_contacttype->frequency != RPT_never)
    RPT_reporting_add_integer1 (local_data->nunits_potentially_traced_by_contacttype,
                                1, local_data->contact_type_name_with_p[event->contact_type]);
  if (local_data->nunits_potentially_traced_by_prodtype->frequency != RPT_never)
    RPT_reporting_add_integer1 (local_data->nunits_potentially_traced_by_prodtype,
                                1, local_data->production_type_name_with_p[identified_herd->production_type]);
  if (local_data->nunits_potentially_traced_by_contacttype_and_prodtype->frequency != RPT_never)
    RPT_reporting_add_integer (local_data->nunits_potentially_traced_by_contacttype_and_prodtype,
                               1, drill_down_list);

  if (event->traced == TRUE)
    {
      /* Record a successfully traced contact. */
      drill_down_list[1] = identified_herd->production_type_name;
      RPT_reporting_add_integer (local_data->nunits_traced, 1, NULL);
      if (local_data->nunits_traced_by_contacttype->frequency != RPT_never)
        RPT_reporting_add_integer1 (local_data->nunits_traced_by_contacttype,
                                    1, contact_type_name);
      if (local_data->nunits_traced_by_prodtype->frequency != RPT_never)
        RPT_reporting_add_integer1 (local_data->nunits_traced_by_prodtype,
                                    1, identified_herd->production_type_name);
      if (local_data->nunits_traced_by_contacttype_and_prodtype->frequency != RPT_never)
        RPT_reporting_add_integer (local_data->nunits_traced_by_contacttype_and_prodtype,
                                   1, drill_down_list);
    }

  #if DEBUG
    g_debug ("----- EXIT handle_trace_result_event (%s)", MODEL_NAME);
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
    case EVT_BeforeAnySimulations:
      handle_before_any_simulations_event (self, queue);
      break;
    case EVT_NewDay:
      handle_new_day_event (self);
      break;
    case EVT_TraceResult:
      handle_trace_result_event (self, &(event->u.trace_result));
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
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER reset (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  RPT_reporting_zero (local_data->cumul_nunits_potentially_traced);
  RPT_reporting_zero (local_data->cumul_nunits_potentially_traced_by_contacttype);
  RPT_reporting_zero (local_data->cumul_nunits_potentially_traced_by_prodtype);
  RPT_reporting_zero (local_data->cumul_nunits_potentially_traced_by_contacttype_and_prodtype);
  RPT_reporting_zero (local_data->cumul_nunits_traced);
  RPT_reporting_zero (local_data->cumul_nunits_traced_by_contacttype);
  RPT_reporting_zero (local_data->cumul_nunits_traced_by_prodtype);
  RPT_reporting_zero (local_data->cumul_nunits_traced_by_contacttype_and_prodtype);

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
  g_string_sprintf (s, "<%s>", MODEL_NAME);

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
 * Frees this model.
 *
 * @param self the model.
 */
void
local_free (struct naadsm_model_t_ *self)
{
  local_data_t *local_data;
  unsigned int i;

#if DEBUG
  g_debug ("----- ENTER free (%s)", MODEL_NAME);
#endif

  /* Free the dynamically-allocated parts. */
  local_data = (local_data_t *) (self->model_data);

  RPT_free_reporting (local_data->nunits_potentially_traced);
  RPT_free_reporting (local_data->nunits_potentially_traced_by_contacttype);
  RPT_free_reporting (local_data->nunits_potentially_traced_by_prodtype);
  RPT_free_reporting (local_data->nunits_potentially_traced_by_contacttype_and_prodtype);
  RPT_free_reporting (local_data->cumul_nunits_potentially_traced);
  RPT_free_reporting (local_data->cumul_nunits_potentially_traced_by_contacttype);
  RPT_free_reporting (local_data->cumul_nunits_potentially_traced_by_prodtype);
  RPT_free_reporting (local_data->cumul_nunits_potentially_traced_by_contacttype_and_prodtype);
  RPT_free_reporting (local_data->nunits_traced);
  RPT_free_reporting (local_data->nunits_traced_by_contacttype);
  RPT_free_reporting (local_data->nunits_traced_by_prodtype);
  RPT_free_reporting (local_data->nunits_traced_by_contacttype_and_prodtype);
  RPT_free_reporting (local_data->cumul_nunits_traced);
  RPT_free_reporting (local_data->cumul_nunits_traced_by_contacttype);
  RPT_free_reporting (local_data->cumul_nunits_traced_by_prodtype);
  RPT_free_reporting (local_data->cumul_nunits_traced_by_contacttype_and_prodtype);

  for (i = NAADSM_DirectContact; i <= NAADSM_IndirectContact; i++)
    g_free (local_data->contact_type_name_with_p[i]);
  for (i = 0; i < local_data->production_types->len; i++)
    g_free (local_data->production_type_name_with_p[i]);
  g_free (local_data->production_type_name_with_p);
  
  g_free (local_data);
  g_ptr_array_free (self->outputs, TRUE);
  g_free (self);

#if DEBUG
  g_debug ("----- EXIT free (%s)", MODEL_NAME);
#endif
}



/**
 * Returns a new trace back monitor.
 */
naadsm_model_t *
new (scew_element * params, HRD_herd_list_t * herds, projPJ projections,
     ZON_zone_list_t * zones)
{
  naadsm_model_t *m;
  local_data_t *local_data;
  scew_element *e, **ee;
  unsigned int n;
  const XML_Char *variable_name;
  RPT_frequency_t freq;
  gboolean success;
  gboolean broken_down;
  unsigned int i, j;      /* loop counters */
  const char *contact_type_name;
  char *prodtype_name;
  const char *drill_down_list[3] = { NULL, NULL, NULL };

#if DEBUG
  g_debug ("----- ENTER new (%s)", MODEL_NAME);
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

  local_data->nunits_potentially_traced =
    RPT_new_reporting ("trnUAllp", RPT_integer, RPT_never);
  local_data->nunits_potentially_traced_by_contacttype =
    RPT_new_reporting ("trnU", RPT_group, RPT_never);
  local_data->nunits_potentially_traced_by_prodtype =
    RPT_new_reporting ("trnU", RPT_group, RPT_never);
  local_data->nunits_potentially_traced_by_contacttype_and_prodtype =
    RPT_new_reporting ("trnU", RPT_group, RPT_never);
  local_data->cumul_nunits_potentially_traced =
    RPT_new_reporting ("trcUAllp", RPT_integer, RPT_never);
  local_data->cumul_nunits_potentially_traced_by_contacttype =
    RPT_new_reporting ("trcU", RPT_group, RPT_never);
  local_data->cumul_nunits_potentially_traced_by_prodtype =
    RPT_new_reporting ("trcU", RPT_group, RPT_never);
  local_data->cumul_nunits_potentially_traced_by_contacttype_and_prodtype =
    RPT_new_reporting ("trcU", RPT_group, RPT_never);
  local_data->nunits_traced =
    RPT_new_reporting ("trnUAll", RPT_integer, RPT_never);
  local_data->nunits_traced_by_contacttype =
    RPT_new_reporting ("trnU", RPT_group, RPT_never);
  local_data->nunits_traced_by_prodtype =
    RPT_new_reporting ("trnU", RPT_group, RPT_never);
  local_data->nunits_traced_by_contacttype_and_prodtype =
    RPT_new_reporting ("trnU", RPT_group, RPT_never);
  local_data->cumul_nunits_traced =
    RPT_new_reporting ("trcUAll", RPT_integer, RPT_never);
  local_data->cumul_nunits_traced_by_contacttype =
    RPT_new_reporting ("trcU", RPT_group, RPT_never);
  local_data->cumul_nunits_traced_by_prodtype =
    RPT_new_reporting ("trcU", RPT_group, RPT_never);
  local_data->cumul_nunits_traced_by_contacttype_and_prodtype =
    RPT_new_reporting ("trcU", RPT_group, RPT_never);
  g_ptr_array_add (m->outputs, local_data->nunits_potentially_traced);
  g_ptr_array_add (m->outputs, local_data->nunits_potentially_traced_by_contacttype);
  g_ptr_array_add (m->outputs, local_data->nunits_potentially_traced_by_prodtype);
  g_ptr_array_add (m->outputs, local_data->nunits_potentially_traced_by_contacttype_and_prodtype);
  g_ptr_array_add (m->outputs, local_data->cumul_nunits_potentially_traced);
  g_ptr_array_add (m->outputs, local_data->cumul_nunits_potentially_traced_by_contacttype);
  g_ptr_array_add (m->outputs, local_data->cumul_nunits_potentially_traced_by_prodtype);
  g_ptr_array_add (m->outputs, local_data->cumul_nunits_potentially_traced_by_contacttype_and_prodtype);
  g_ptr_array_add (m->outputs, local_data->nunits_traced);
  g_ptr_array_add (m->outputs, local_data->nunits_traced_by_contacttype);
  g_ptr_array_add (m->outputs, local_data->nunits_traced_by_prodtype);
  g_ptr_array_add (m->outputs, local_data->nunits_traced_by_contacttype_and_prodtype);
  g_ptr_array_add (m->outputs, local_data->cumul_nunits_traced);
  g_ptr_array_add (m->outputs, local_data->cumul_nunits_traced_by_contacttype);
  g_ptr_array_add (m->outputs, local_data->cumul_nunits_traced_by_prodtype);
  g_ptr_array_add (m->outputs, local_data->cumul_nunits_traced_by_contacttype_and_prodtype);

  /* Set the reporting frequency for the output variables. */
  ee = scew_element_list (params, "output", &n);
#if DEBUG
  g_debug ("%u output variables", n);
#endif
  for (i = 0; i < n; i++)
    {
      e = ee[i];
      variable_name = scew_element_contents (scew_element_by_name (e, "variable-name"));
      freq = RPT_string_to_frequency (scew_element_contents
                                      (scew_element_by_name (e, "frequency")));
      broken_down = PAR_get_boolean (scew_element_by_name (e, "broken-down"), &success);
      if (!success)
      	broken_down = FALSE;
      broken_down = broken_down || (g_strstr_len (variable_name, -1, "-by-") != NULL); 
      /* Starting at version 3.2 we accept either the old, verbose output
       * variable names or the shorter ones used in NAADSM/PC. */
      if (strcmp (variable_name, "trnUp") == 0
          || strncmp (variable_name, "num-contacts-potentially-traced", 31) == 0)
        {
          RPT_reporting_set_frequency (local_data->nunits_potentially_traced, freq);
          if (broken_down)
            {
              RPT_reporting_set_frequency (local_data->nunits_potentially_traced_by_contacttype, freq);
              RPT_reporting_set_frequency (local_data->nunits_potentially_traced_by_prodtype, freq);
              RPT_reporting_set_frequency (local_data->nunits_potentially_traced_by_contacttype_and_prodtype, freq);
            }
        }
      else if (strcmp (variable_name, "trnU") == 0
               || strncmp (variable_name, "num-contacts-traced", 19) == 0)
        {
          RPT_reporting_set_frequency (local_data->nunits_traced, freq);
          if (broken_down)
            {
              RPT_reporting_set_frequency (local_data->nunits_traced_by_contacttype, freq);
              RPT_reporting_set_frequency (local_data->nunits_traced_by_prodtype, freq);
              RPT_reporting_set_frequency (local_data->nunits_traced_by_contacttype_and_prodtype, freq);
            }
        }
      else if (strcmp (variable_name, "trcUp") == 0
               || strncmp (variable_name, "cumulative-num-contacts-potentially-traced", 42) == 0)
        {
          RPT_reporting_set_frequency (local_data->cumul_nunits_potentially_traced, freq);
          if (broken_down)
            {
              RPT_reporting_set_frequency (local_data->cumul_nunits_potentially_traced_by_contacttype, freq);
              RPT_reporting_set_frequency (local_data->cumul_nunits_potentially_traced_by_prodtype, freq);
              RPT_reporting_set_frequency (local_data->cumul_nunits_potentially_traced_by_contacttype_and_prodtype, freq);
            }
        }
      else if (strcmp (variable_name, "trcU") == 0
               || strncmp (variable_name, "cumulative-num-contacts-traced", 30) == 0)
        {
          RPT_reporting_set_frequency (local_data->cumul_nunits_traced, freq);
          if (broken_down)
            {
              RPT_reporting_set_frequency (local_data->cumul_nunits_traced_by_contacttype, freq);
              RPT_reporting_set_frequency (local_data->cumul_nunits_traced_by_prodtype, freq);
              RPT_reporting_set_frequency (local_data->cumul_nunits_traced_by_contacttype_and_prodtype, freq);
            }
        }
      else
        g_warning ("no output variable named \"%s\", ignoring", variable_name);        
    }
  free (ee);

  /* Initialize the categories in the output variables. */
  local_data->production_types = herds->production_type_names;
  /* These are the outputs broken down by contact type. */
  for (i = NAADSM_DirectContact; i <= NAADSM_IndirectContact; i++)
    {
      /* Make a copy of each contact type name, but with "p" appended.  We need
       * this to form variables like "trcUDirp" and "trcUIndp". */
      local_data->contact_type_name_with_p[i] =
        g_strdup_printf ("%sp", NAADSM_contact_type_abbrev[i]);
      
      contact_type_name = local_data->contact_type_name_with_p[i];
      RPT_reporting_add_integer1 (local_data->nunits_potentially_traced_by_contacttype, 0, contact_type_name);
      RPT_reporting_add_integer1 (local_data->cumul_nunits_potentially_traced_by_contacttype, 0, contact_type_name);
      contact_type_name = NAADSM_contact_type_abbrev[i];
      RPT_reporting_add_integer1 (local_data->nunits_traced_by_contacttype, 0, contact_type_name);
      RPT_reporting_add_integer1 (local_data->cumul_nunits_traced_by_contacttype, 0, contact_type_name);
    }
  /* These are the outputs broken down by production type. */
  local_data->production_type_name_with_p = g_new0 (char *, local_data->production_types->len);
  for (i = 0; i < local_data->production_types->len; i++)
    {
      /* Make a copy of each production type name, but with "p" appended.  We
       * need this to form variables like "trcUCattlep". */
       local_data->production_type_name_with_p[i] =
         g_strdup_printf ("%sp", (char *) g_ptr_array_index (local_data->production_types, i));
      
      prodtype_name = local_data->production_type_name_with_p[i];
      RPT_reporting_add_integer1 (local_data->nunits_potentially_traced_by_prodtype, 0, prodtype_name);
      RPT_reporting_add_integer1 (local_data->cumul_nunits_potentially_traced_by_prodtype, 0, prodtype_name);
      prodtype_name = (char *) g_ptr_array_index (local_data->production_types, i);
      RPT_reporting_add_integer1 (local_data->nunits_traced_by_prodtype, 0, prodtype_name);
      RPT_reporting_add_integer1 (local_data->cumul_nunits_traced_by_prodtype, 0, prodtype_name);
    }
  /* These are the outputs broken down by contact type and production type. */
  for (i = NAADSM_DirectContact; i <= NAADSM_IndirectContact; i++)
    {
      drill_down_list[0] = NAADSM_contact_type_abbrev[i];
      for (j = 0; j < local_data->production_types->len; j++)
        {
          drill_down_list[1] = local_data->production_type_name_with_p[j];
          RPT_reporting_add_integer (local_data->nunits_potentially_traced_by_contacttype_and_prodtype, 0, drill_down_list);
          RPT_reporting_add_integer (local_data->cumul_nunits_potentially_traced_by_contacttype_and_prodtype, 0, drill_down_list);
          drill_down_list[1] = (char *) g_ptr_array_index (local_data->production_types, j);
          RPT_reporting_add_integer (local_data->nunits_traced_by_contacttype_and_prodtype, 0, drill_down_list);
          RPT_reporting_add_integer (local_data->cumul_nunits_traced_by_contacttype_and_prodtype, 0, drill_down_list);
        }
    }

#if DEBUG
  g_debug ("----- EXIT new (%s)", MODEL_NAME);
#endif

  return m;
}

/* end of file trace-back-monitor.c */
