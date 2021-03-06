/** @file trace-destruction-model.c
 * Module that simulates a policy of destroying units that have been found
 * through trace-out or trace-in.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Grid Computing Research Group<br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.1
 * @date August 2007
 *
 * Copyright &copy; University of Guelph, 2007-2008
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
#define new trace_destruction_model_new
#define run trace_destruction_model_run
#define reset trace_destruction_model_reset
#define events_listened_for trace_destruction_model_events_listened_for
#define is_listening_for trace_destruction_model_is_listening_for
#define has_pending_actions trace_destruction_model_has_pending_actions
#define has_pending_infections trace_destruction_model_has_pending_infections
#define to_string trace_destruction_model_to_string
#define local_printf trace_destruction_model_printf
#define local_fprintf trace_destruction_model_fprintf
#define local_free trace_destruction_model_free
#define handle_before_any_simulations_event trace_destruction_model_handle_before_any_simulations_event
#define handle_trace_result_event trace_destruction_model_handle_trace_result_event

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

#include "trace-destruction-model.h"

#if !HAVE_ROUND && HAVE_RINT
#  define round rint
#endif

/* Temporary fix -- "round" and "rint" are in the math library on Red Hat 7.3,
 * but they're #defined so AC_CHECK_FUNCS doesn't find them. */
double round (double x);

#include "naadsm.h"

/** This must match an element name in the DTD. */
#define MODEL_NAME "trace-destruction-model"



#define NEVENTS_LISTENED_FOR 2
EVT_event_type_t events_listened_for[] =
  { EVT_BeforeAnySimulations, EVT_TraceResult };



/** Specialized information for this model. */
typedef struct
{
  NAADSM_contact_type contact_type;
  NAADSM_trace_direction direction;
  gboolean *production_type;
  GPtrArray *production_types;
  int priority;
}
local_data_t;



/**
 * Before any simulations, this module declares all the reasons for which it
 * may request a destruction.
 *
 * @param self this module.
 * @param queue for any new events the model creates.
 */
void
handle_before_any_simulations_event (struct naadsm_model_t_ *self,
                                     EVT_event_queue_t * queue)
{
  local_data_t *local_data;
  GPtrArray *reasons;

#if DEBUG
  g_debug ("----- ENTER handle_before_any_simulations_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  reasons = g_ptr_array_sized_new (1);
  if (local_data->direction == NAADSM_TraceForwardOrOut)
    {
      if (local_data->contact_type == NAADSM_DirectContact)
        g_ptr_array_add (reasons, "DirFwd");
      else
        g_ptr_array_add (reasons, "IndFwd");
    }
  else /* trace-in */
    {
      if (local_data->contact_type == NAADSM_DirectContact)
        g_ptr_array_add (reasons, "DirBack");
      else
        g_ptr_array_add (reasons, "IndBack");
    }
  EVT_event_enqueue (queue, EVT_new_declaration_of_destruction_reasons_event (reasons));

  /* Note that we don't clean up the GPtrArray.  It will be freed along with
   * the declaration event after all interested sub-models have processed the
   * event. */

#if DEBUG
  g_debug ("----- EXIT handle_before_any_simulations_event (%s)", MODEL_NAME);
#endif
  return;
}



/**
 * Responds to a trace result by ordering destruction actions.
 *
 * @param self the model.
 * @param herds the list of herds.
 * @param event a trace result event.
 * @param rng a random number generator.
 * @param queue for any new events the model creates.
 */
void
handle_trace_result_event (struct naadsm_model_t_ *self, HRD_herd_list_t * herds,
                           EVT_trace_result_event_t * event, RAN_gen_t * rng, EVT_event_queue_t * queue)
{
  local_data_t *local_data;
  HRD_herd_t *herd;
  EVT_event_t *destr_event;

#if DEBUG
  g_debug ("----- ENTER handle_trace_result_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  /* Some trace result events describe a contact that the trace investigation
   * failed to find.  Those are tracked in simulation statistics, but this
   * module is not interested in them. */
  if (event->traced == FALSE)
    goto end;

  /* This module is parameterized by production type, contact type, and trace
   * direction.  For example, it might apply to a cattle unit found by tracing
   * direct contacts out from another unit (the cattle herd was the recipient).
   * Or it might apply to a pig unit found by tracing indirect contacts into
   * another unit (the pig unit was the source).  Make sure the trace result
   * event we have received is one we are interested in. */
  if (event->contact_type != local_data->contact_type)
    goto end;

  if (event->direction != local_data->direction)
    goto end;

  if (local_data->direction == NAADSM_TraceForwardOrOut)
    herd = event->exposed_herd;
  else
    herd = event->exposing_herd;

  if (local_data->production_type[herd->production_type] == FALSE
      || herd->status == Destroyed
      #ifdef RIVERTON
      || herd->status == NaturallyImmune
      #endif
  )
    goto end;

  /* Now that we know this is a trace result we are interested in, issue
   * destruction requests. */
  if (local_data->direction == NAADSM_TraceForwardOrOut)
    {
      if (event->contact_type == NAADSM_DirectContact)
        destr_event = EVT_new_request_for_destruction_event (herd, event->day, "DirFwd", local_data->priority);
      else /* indirect */
        destr_event = EVT_new_request_for_destruction_event (herd, event->day, "IndFwd", local_data->priority);
    }
  else
    {
      if (event->contact_type == NAADSM_DirectContact)
        destr_event = EVT_new_request_for_destruction_event (herd, event->day, "DirBack", local_data->priority);
      else /* indirect */
        destr_event = EVT_new_request_for_destruction_event (herd, event->day, "IndBack", local_data->priority);
    }
#if DEBUG
  g_debug ("requesting destruction of unit \"%s\"", herd->official_id);
#endif
  EVT_event_enqueue (queue, destr_event);

end:
#if DEBUG
  g_debug ("----- EXIT handle_trace_result_event (%s)", MODEL_NAME);
#endif
  return;
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
    case EVT_TraceResult:
      handle_trace_result_event (self, herds, &(event->u.trace_result), rng, queue);
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
  gboolean already_names;
  unsigned int i;
  char *chararray;
  local_data_t *local_data;

  local_data = (local_data_t *) (self->model_data);
  s = g_string_new (NULL);
  g_string_sprintf (s, "<%s for ", MODEL_NAME);
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
  g_string_append_printf (s, " units found by %s %s",
    NAADSM_contact_type_name[local_data->contact_type],
    NAADSM_trace_direction_name[local_data->direction]);

  g_string_append_printf (s, "\n  priority=%i>", local_data->priority);

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
 * Frees this model.  Does not free the contact type name or production type
 * names.
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
  g_free (local_data->production_type);
  g_free (local_data);
  g_ptr_array_free (self->outputs, TRUE);
  g_free (self);

#if DEBUG
  g_debug ("----- EXIT free (%s)", MODEL_NAME);
#endif
}



/**
 * Returns a new trace destruction model.
 */
naadsm_model_t *
new (scew_element * params, HRD_herd_list_t * herds, projPJ projection,
     ZON_zone_list_t * zones)
{
  naadsm_model_t *self;
  local_data_t *local_data;
  scew_element const *e;
  scew_attribute *attr;
  XML_Char const *attr_text;
  gboolean success;

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

#if DEBUG
  g_debug ("setting contact type");
#endif
  attr = scew_attribute_by_name (params, "contact-type");
  g_assert (attr != NULL);
  attr_text = scew_attribute_value (attr);
  if (strcmp (attr_text, "direct") == 0)
    local_data->contact_type = NAADSM_DirectContact;
  else if (strcmp (attr_text, "indirect") == 0)
    local_data->contact_type = NAADSM_IndirectContact;
  else
    g_assert_not_reached ();

#if DEBUG
  g_debug ("setting trace direction");
#endif
  attr = scew_attribute_by_name (params, "direction");
  g_assert (attr != NULL);
  attr_text = scew_attribute_value (attr);
  if (strcmp (attr_text, "out") == 0)
    local_data->direction = NAADSM_TraceForwardOrOut;
  else if (strcmp (attr_text, "in") == 0)
    local_data->direction = NAADSM_TraceBackOrIn;
  else
    g_assert_not_reached ();

#if DEBUG
  g_debug ("setting production types");
#endif
  local_data->production_types = herds->production_type_names;
  local_data->production_type =
    naadsm_read_prodtype_attribute (params, "production-type", herds->production_type_names);

  e = scew_element_by_name (params, "priority");
  if (e != NULL)
    {
      local_data->priority = (int) round (PAR_get_unitless (e, &success));
      if (success == FALSE)
        {
          g_warning ("%s: setting priority to 1", MODEL_NAME);
          local_data->priority = 1;
        }
      if (local_data->priority < 1)
        {
          g_warning ("%s: priority cannot be less than 1, setting to 1", MODEL_NAME);
          local_data->priority = 1;
        }
    }
  else
    {
      g_warning ("%s: priority missing, setting to 1", MODEL_NAME);
      local_data->priority = 1;
    }

#if DEBUG
  g_debug ("----- EXIT new (%s)", MODEL_NAME);
#endif

  return self;
}

/* end of file trace-destruction-model.c */
