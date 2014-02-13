/** @file test-monitor.c
 * Records information on diagnostic testing: how many units are tested, for
 * reasons, and how many true positives, false positives, etc. occur.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Grid Computing Research Group<br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @date July 2009
 *
 * Copyright &copy; University of Guelph, 2009
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
#define new test_monitor_new
#define run test_monitor_run
#define reset test_monitor_reset
#define events_listened_for test_monitor_events_listened_for
#define is_listening_for test_monitor_is_listening_for
#define has_pending_actions test_monitor_has_pending_actions
#define has_pending_infections test_monitor_has_pending_infections
#define to_string test_monitor_to_string
#define local_printf test_monitor_printf
#define local_fprintf test_monitor_fprintf
#define local_free test_monitor_free
#define handle_before_any_simulations_event test_monitor_handle_before_any_simulations_event
#define handle_test_event test_monitor_handle_test_event
#define handle_test_result_event test_monitor_handle_test_result_event

#include "model.h"
#include "naadsm.h"

#if STDC_HEADERS
#  include <string.h>
#endif

#include "test-monitor.h"

/** This must match an element name in the DTD. */
#define MODEL_NAME "test-monitor"



#define NEVENTS_LISTENED_FOR 3
EVT_event_type_t events_listened_for[] = { EVT_BeforeAnySimulations,
EVT_Test, EVT_TestResult };



/* Specialized information for this model. */
typedef struct
{
  GPtrArray *production_types;
  RPT_reporting_t *cumul_nunits_tested;
  RPT_reporting_t *cumul_nunits_tested_by_reason;
  RPT_reporting_t *cumul_nunits_tested_by_prodtype;
  RPT_reporting_t *cumul_nunits_tested_by_reason_and_prodtype;
  RPT_reporting_t *cumul_nunits_truepos;
  RPT_reporting_t *cumul_nunits_truepos_by_prodtype;
  RPT_reporting_t *cumul_nunits_trueneg;
  RPT_reporting_t *cumul_nunits_trueneg_by_prodtype;
  RPT_reporting_t *cumul_nunits_falsepos;
  RPT_reporting_t *cumul_nunits_falsepos_by_prodtype;
  RPT_reporting_t *cumul_nunits_falseneg;
  RPT_reporting_t *cumul_nunits_falseneg_by_prodtype;
  RPT_reporting_t *cumul_nanimals_tested;
  RPT_reporting_t *cumul_nanimals_tested_by_reason;
  RPT_reporting_t *cumul_nanimals_tested_by_prodtype;
  RPT_reporting_t *cumul_nanimals_tested_by_reason_and_prodtype;
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
 * Records a test.
 *
 * @param self the model.
 * @param event a test event.
 */
void
handle_test_event (struct naadsm_model_t_ *self, EVT_test_event_t * event)
{
  local_data_t *local_data;
  HRD_herd_t *herd;
  const char *reason;
  const char *drill_down_list[3] = { NULL, NULL, NULL };

#if DEBUG
  g_debug ("----- ENTER handle_test_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  herd = event->herd;
  reason = NAADSM_control_reason_abbrev[event->reason];

  RPT_reporting_add_integer (local_data->cumul_nunits_tested, 1, NULL);
  RPT_reporting_add_integer1 (local_data->cumul_nunits_tested_by_reason, 1, reason);
  RPT_reporting_add_integer1 (local_data->cumul_nunits_tested_by_prodtype, 1, herd->production_type_name);
  RPT_reporting_add_integer (local_data->cumul_nanimals_tested, herd->size, NULL);
  RPT_reporting_add_integer1 (local_data->cumul_nanimals_tested_by_reason, herd->size, reason);
  RPT_reporting_add_integer1 (local_data->cumul_nanimals_tested_by_prodtype, herd->size, herd->production_type_name);
  drill_down_list[0] = reason;
  drill_down_list[1] = herd->production_type_name;
  if (local_data->cumul_nunits_tested_by_reason_and_prodtype->frequency != RPT_never)
    RPT_reporting_add_integer (local_data->cumul_nunits_tested_by_reason_and_prodtype, 1, drill_down_list);
  if (local_data->cumul_nanimals_tested_by_reason_and_prodtype->frequency != RPT_never)
    RPT_reporting_add_integer (local_data->cumul_nanimals_tested_by_reason_and_prodtype, herd->size, drill_down_list);

#if DEBUG
  g_debug ("----- EXIT handle_test_event (%s)", MODEL_NAME);
#endif
}



/**
 * Records a test result.
 *
 * @param self the model.
 * @param event a test result event.
 */
void
handle_test_result_event (struct naadsm_model_t_ * self,
                          EVT_test_result_event_t * event)
{
  local_data_t *local_data;
  HRD_herd_t *herd;
  HRD_test_t test;

#if DEBUG
  g_debug ("----- ENTER handle_test_result_event (%s)", MODEL_NAME);
#endif

  /* Record the test in the GUI */
  /* -------------------------- */
  test.herd_index = event->herd->index;

  if( event->reason == NAADSM_ControlTraceForwardDirect )
    {
      test.contact_type = NAADSM_DirectContact;
      test.trace_type = NAADSM_TraceForwardOrOut;  
    }
  else if( event->reason == NAADSM_ControlTraceBackDirect )
    {
      test.contact_type = NAADSM_DirectContact;
      test.trace_type = NAADSM_TraceBackOrIn;    
    }
  else if( event->reason == NAADSM_ControlTraceForwardIndirect )
    {
      test.contact_type = NAADSM_IndirectContact;
      test.trace_type = NAADSM_TraceForwardOrOut;    
    }
  else if( event->reason == NAADSM_ControlTraceBackIndirect )
    {
      test.contact_type = NAADSM_IndirectContact;
      test.trace_type = NAADSM_TraceBackOrIn;    
    }
  else
    {
      g_error( "Unrecognized event reason (%s) in test-monitor.handle_test_result_event",
               NAADSM_control_reason_name[event->reason] );  
    } 

  if( event->positive && event->correct )
    test.test_result = NAADSM_TestTruePositive;
  else if( event->positive && !(event->correct) )
    test.test_result = NAADSM_TestFalsePositive;
  else if( !(event->positive) && event->correct )
    test.test_result = NAADSM_TestTrueNegative;
  else if( !(event->positive) && !(event->correct) )
    test.test_result = NAADSM_TestFalseNegative;
    
  #ifdef USE_SC_GUILIB
    sc_test_herd( event->herd, test );
  #else
    if (NULL != naadsm_test_herd) 
      naadsm_test_herd (test);
  #endif

  /* Record the test in the SC version */
  /* --------------------------------- */
  local_data = (local_data_t *) (self->model_data);
  herd = event->herd;

  if (event->positive)
    {
      if (event->correct)
        {
          RPT_reporting_add_integer (local_data->cumul_nunits_truepos, 1, NULL);
          RPT_reporting_add_integer1 (local_data->cumul_nunits_truepos_by_prodtype, 1, herd->production_type_name);
        }
      else
        {
          RPT_reporting_add_integer (local_data->cumul_nunits_falsepos, 1, NULL);
          RPT_reporting_add_integer1 (local_data->cumul_nunits_falsepos_by_prodtype, 1, herd->production_type_name);
        }
    }
  else /* test result was negative */
    {
      if (event->correct)
        {
          RPT_reporting_add_integer (local_data->cumul_nunits_trueneg, 1, NULL);
          RPT_reporting_add_integer1 (local_data->cumul_nunits_trueneg_by_prodtype, 1, herd->production_type_name);
        }
      else
        {
          RPT_reporting_add_integer (local_data->cumul_nunits_falseneg, 1, NULL);
          RPT_reporting_add_integer1 (local_data->cumul_nunits_falseneg_by_prodtype, 1, herd->production_type_name);
        }
    }

#if DEBUG
  g_debug ("----- EXIT handle_test_result_event (%s)", MODEL_NAME);
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
    case EVT_BeforeAnySimulations:
      handle_before_any_simulations_event (self, queue);
      break;
    case EVT_Test:
      handle_test_event (self, &(event->u.test));
      break;
    case EVT_TestResult:
      handle_test_result_event (self, &(event->u.test_result));
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
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER reset (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  RPT_reporting_zero (local_data->cumul_nunits_tested);
  RPT_reporting_zero (local_data->cumul_nunits_tested_by_reason);
  RPT_reporting_zero (local_data->cumul_nunits_tested_by_prodtype);
  RPT_reporting_zero (local_data->cumul_nunits_tested_by_reason_and_prodtype);
  RPT_reporting_zero (local_data->cumul_nunits_truepos);
  RPT_reporting_zero (local_data->cumul_nunits_truepos_by_prodtype);
  RPT_reporting_zero (local_data->cumul_nunits_trueneg);
  RPT_reporting_zero (local_data->cumul_nunits_trueneg_by_prodtype);
  RPT_reporting_zero (local_data->cumul_nunits_falsepos);
  RPT_reporting_zero (local_data->cumul_nunits_falsepos_by_prodtype);
  RPT_reporting_zero (local_data->cumul_nunits_falseneg);
  RPT_reporting_zero (local_data->cumul_nunits_falseneg_by_prodtype);
  RPT_reporting_zero (local_data->cumul_nanimals_tested);
  RPT_reporting_zero (local_data->cumul_nanimals_tested_by_reason);
  RPT_reporting_zero (local_data->cumul_nanimals_tested_by_prodtype);
  RPT_reporting_zero (local_data->cumul_nanimals_tested_by_reason_and_prodtype);

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

#if DEBUG
  g_debug ("----- ENTER free (%s)", MODEL_NAME);
#endif

  /* Free the dynamically-allocated parts. */
  local_data = (local_data_t *) (self->model_data);
  RPT_free_reporting (local_data->cumul_nunits_tested);
  RPT_free_reporting (local_data->cumul_nunits_tested_by_reason);
  RPT_free_reporting (local_data->cumul_nunits_tested_by_prodtype);
  RPT_free_reporting (local_data->cumul_nunits_tested_by_reason_and_prodtype);
  RPT_free_reporting (local_data->cumul_nunits_truepos);
  RPT_free_reporting (local_data->cumul_nunits_truepos_by_prodtype);
  RPT_free_reporting (local_data->cumul_nunits_trueneg);
  RPT_free_reporting (local_data->cumul_nunits_trueneg_by_prodtype);
  RPT_free_reporting (local_data->cumul_nunits_falsepos);
  RPT_free_reporting (local_data->cumul_nunits_falsepos_by_prodtype);
  RPT_free_reporting (local_data->cumul_nunits_falseneg);
  RPT_free_reporting (local_data->cumul_nunits_falseneg_by_prodtype);
  RPT_free_reporting (local_data->cumul_nanimals_tested);
  RPT_free_reporting (local_data->cumul_nanimals_tested_by_reason);
  RPT_free_reporting (local_data->cumul_nanimals_tested_by_prodtype);
  RPT_free_reporting (local_data->cumul_nanimals_tested_by_reason_and_prodtype);

  g_free (local_data);
  g_ptr_array_free (self->outputs, TRUE);
  g_free (self);

#if DEBUG
  g_debug ("----- EXIT free (%s)", MODEL_NAME);
#endif
}



/**
 * Returns a new test monitor.
 */
naadsm_model_t *
new (scew_element * params, HRD_herd_list_t * herds, projPJ projection,
     ZON_zone_list_t * zones)
{
  naadsm_model_t *self;
  local_data_t *local_data;
  scew_element *e, **ee;
  unsigned int n;
  const XML_Char *variable_name;
  RPT_frequency_t freq;
  gboolean success;
  gboolean broken_down;
  unsigned int i, j;         /* loop counters */
  char *prodtype_name;

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
  self->run = run;
  self->reset = reset;
  self->is_listening_for = is_listening_for;
  self->has_pending_actions = has_pending_actions;
  self->has_pending_infections = has_pending_infections;
  self->to_string = to_string;
  self->printf = local_printf;
  self->fprintf = local_fprintf;
  self->free = local_free;

  /* Make sure the right XML subtree was sent. */
  g_assert (strcmp (scew_element_name (params), MODEL_NAME) == 0);

  local_data->cumul_nunits_tested =
    RPT_new_reporting ("tstcUAll", RPT_integer, RPT_never);
  local_data->cumul_nunits_tested_by_reason =
    RPT_new_reporting ("tstcU", RPT_group, RPT_never);
  local_data->cumul_nunits_tested_by_prodtype =
    RPT_new_reporting ("tstcU", RPT_group, RPT_never);
  local_data->cumul_nunits_tested_by_reason_and_prodtype =
    RPT_new_reporting ("tstcU", RPT_group, RPT_never);
  local_data->cumul_nunits_truepos =
    RPT_new_reporting ("tstcUTruePos", RPT_integer, RPT_never);
  local_data->cumul_nunits_truepos_by_prodtype =
    RPT_new_reporting ("tstcUTruePos", RPT_group, RPT_never);
  local_data->cumul_nunits_trueneg =
    RPT_new_reporting ("tstcUTrueNeg", RPT_integer, RPT_never);
  local_data->cumul_nunits_trueneg_by_prodtype =
    RPT_new_reporting ("tstcUTrueNeg", RPT_group, RPT_never);
  local_data->cumul_nunits_falsepos =
    RPT_new_reporting ("tstcUFalsePos", RPT_integer, RPT_never);
  local_data->cumul_nunits_falsepos_by_prodtype =
    RPT_new_reporting ("tstcUFalsePos", RPT_group, RPT_never);
  local_data->cumul_nunits_falseneg =
    RPT_new_reporting ("tstcUFalseNeg", RPT_integer, RPT_never);
  local_data->cumul_nunits_falseneg_by_prodtype =
    RPT_new_reporting ("tstcUFalseNeg", RPT_group, RPT_never);
  local_data->cumul_nanimals_tested =
    RPT_new_reporting ("tstcAAll", RPT_integer, RPT_never);
  local_data->cumul_nanimals_tested_by_reason =
    RPT_new_reporting ("tstcA", RPT_group, RPT_never);
  local_data->cumul_nanimals_tested_by_prodtype =
    RPT_new_reporting ("tstcA", RPT_group, RPT_never);
  local_data->cumul_nanimals_tested_by_reason_and_prodtype =
    RPT_new_reporting ("tstcA", RPT_group, RPT_never);
  g_ptr_array_add (self->outputs, local_data->cumul_nunits_tested);
  g_ptr_array_add (self->outputs, local_data->cumul_nunits_tested_by_reason);
  g_ptr_array_add (self->outputs, local_data->cumul_nunits_tested_by_prodtype);
  g_ptr_array_add (self->outputs, local_data->cumul_nunits_tested_by_reason_and_prodtype);
  g_ptr_array_add (self->outputs, local_data->cumul_nunits_truepos);
  g_ptr_array_add (self->outputs, local_data->cumul_nunits_truepos_by_prodtype);
  g_ptr_array_add (self->outputs, local_data->cumul_nunits_trueneg);
  g_ptr_array_add (self->outputs, local_data->cumul_nunits_trueneg_by_prodtype);
  g_ptr_array_add (self->outputs, local_data->cumul_nunits_falsepos);
  g_ptr_array_add (self->outputs, local_data->cumul_nunits_falsepos_by_prodtype);
  g_ptr_array_add (self->outputs, local_data->cumul_nunits_falseneg);
  g_ptr_array_add (self->outputs, local_data->cumul_nunits_falseneg_by_prodtype);
  g_ptr_array_add (self->outputs, local_data->cumul_nanimals_tested);
  g_ptr_array_add (self->outputs, local_data->cumul_nanimals_tested_by_reason);
  g_ptr_array_add (self->outputs, local_data->cumul_nanimals_tested_by_prodtype);
  g_ptr_array_add (self->outputs, local_data->cumul_nanimals_tested_by_reason_and_prodtype);

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
      if (strcmp (variable_name, "tstcU") == 0)
        {
          RPT_reporting_set_frequency (local_data->cumul_nunits_tested, freq);
          if (success == TRUE && broken_down == TRUE)
            {
              RPT_reporting_set_frequency (local_data->cumul_nunits_tested_by_reason, freq);
              RPT_reporting_set_frequency (local_data->cumul_nunits_tested_by_prodtype, freq);
              RPT_reporting_set_frequency (local_data->cumul_nunits_tested_by_reason_and_prodtype, freq);
            }
        }
      else if (strcmp (variable_name, "tstcUTruePos") == 0)
        {
          RPT_reporting_set_frequency (local_data->cumul_nunits_truepos, freq);
          if (success == TRUE && broken_down == TRUE)
            RPT_reporting_set_frequency (local_data->cumul_nunits_truepos_by_prodtype, freq);
        }
      else if (strcmp (variable_name, "tstcUTrueNeg") == 0)
        {
          RPT_reporting_set_frequency (local_data->cumul_nunits_trueneg, freq);
          if (success == TRUE && broken_down == TRUE)
            RPT_reporting_set_frequency (local_data->cumul_nunits_trueneg_by_prodtype, freq);
        }
      else if (strcmp (variable_name, "tstcUFalsePos") == 0)
        {
          RPT_reporting_set_frequency (local_data->cumul_nunits_falsepos, freq);
          if (success == TRUE && broken_down == TRUE)
            RPT_reporting_set_frequency (local_data->cumul_nunits_falsepos_by_prodtype, freq);
        }
      else if (strcmp (variable_name, "tstcUFalseNeg") == 0)
        {
          RPT_reporting_set_frequency (local_data->cumul_nunits_falseneg, freq);
          if (success == TRUE && broken_down == TRUE)
            RPT_reporting_set_frequency (local_data->cumul_nunits_falseneg_by_prodtype, freq);
        }
      else if (strcmp (variable_name, "tstcA") == 0)
        {
          RPT_reporting_set_frequency (local_data->cumul_nanimals_tested, freq);
          if (success == TRUE && broken_down == TRUE)
            {
              RPT_reporting_set_frequency (local_data->cumul_nanimals_tested_by_reason, freq);
              RPT_reporting_set_frequency (local_data->cumul_nanimals_tested_by_prodtype, freq);
              RPT_reporting_set_frequency (local_data->cumul_nanimals_tested_by_reason_and_prodtype, freq);
            }
        }
      else
        g_warning ("no output variable named \"%s\", ignoring", variable_name);
    }
  free (ee);

  /* Initialize the categories in the output variables. */
  local_data->production_types = herds->production_type_names;
  for (i = 0; i < local_data->production_types->len; i++)
    {
      prodtype_name = (char *) g_ptr_array_index (local_data->production_types, i);
      RPT_reporting_set_integer1 (local_data->cumul_nunits_tested_by_prodtype, 0, prodtype_name);
      RPT_reporting_set_integer1 (local_data->cumul_nunits_truepos_by_prodtype, 0, prodtype_name);
      RPT_reporting_set_integer1 (local_data->cumul_nunits_trueneg_by_prodtype, 0, prodtype_name);
      RPT_reporting_set_integer1 (local_data->cumul_nunits_falsepos_by_prodtype, 0, prodtype_name);
      RPT_reporting_set_integer1 (local_data->cumul_nunits_falseneg_by_prodtype, 0, prodtype_name);
      RPT_reporting_set_integer1 (local_data->cumul_nanimals_tested_by_prodtype, 0, prodtype_name);
    }
  for (i = 0; i < NAADSM_NCONTROL_REASONS; i++)
    {
      const char *reason;
      const char *drill_down_list[3] = { NULL, NULL, NULL };
      if ((NAADSM_control_reason)i == NAADSM_ControlReasonUnspecified
          || (NAADSM_control_reason)i == NAADSM_ControlRing
          || (NAADSM_control_reason)i == NAADSM_ControlDetection
          || (NAADSM_control_reason)i == NAADSM_ControlInitialState)
        continue;
      reason = NAADSM_control_reason_abbrev[i];
      RPT_reporting_add_integer1 (local_data->cumul_nunits_tested_by_reason, 0, reason);
      RPT_reporting_add_integer1 (local_data->cumul_nanimals_tested_by_reason, 0, reason);
      drill_down_list[0] = reason;
      for (j = 0; j < local_data->production_types->len; j++)
        {
          drill_down_list[1] = (char *) g_ptr_array_index (local_data->production_types, j);
          RPT_reporting_add_integer (local_data->cumul_nunits_tested_by_reason_and_prodtype, 0,
                                     drill_down_list);
          RPT_reporting_add_integer (local_data->cumul_nanimals_tested_by_reason_and_prodtype, 0,
                                     drill_down_list);
        }
    }

#if DEBUG
  g_debug ("----- EXIT new (%s)", MODEL_NAME);
#endif

  return self;
}

/* end of file test-monitor.c */
