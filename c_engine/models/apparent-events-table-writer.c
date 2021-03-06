/** @file apparent-events-table-writer.c
 * Writes out a table of detections, vaccinations, and destructions in comma-
 * separated value (csv) format.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 *
 * Copyright &copy; University of Guelph, 2009
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * @todo check errno after opening the file for writing.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

/* To avoid name clashes when multiple modules have the same interface. */
#define is_singleton apparent_events_table_writer_is_singleton
#define new apparent_events_table_writer_new
#define run apparent_events_table_writer_run
#define reset apparent_events_table_writer_reset
#define events_listened_for apparent_events_table_writer_events_listened_for
#define is_listening_for apparent_events_table_writer_is_listening_for
#define has_pending_actions apparent_events_table_writer_has_pending_actions
#define has_pending_infections apparent_events_table_writer_has_pending_infections
#define to_string apparent_events_table_writer_to_string
#define local_printf apparent_events_table_writer_printf
#define local_fprintf apparent_events_table_writer_fprintf
#define local_free apparent_events_table_writer_free
#define handle_before_any_simulations_event apparent_events_table_writer_handle_before_any_simulations_event
#define handle_before_each_simulation_event apparent_events_table_writer_handle_before_each_simulation_event
#define handle_detection_event apparent_events_table_writer_handle_detection_event
#define handle_vaccination_event apparent_events_table_writer_handle_vaccination_event
#define handle_destruction_event apparent_events_table_writer_handle_destruction_event

#include "model.h"
#include "model_util.h"

#if STDC_HEADERS
#  include <string.h>
#endif

#if HAVE_MATH_H
#  include <math.h>
#endif

#if HAVE_STRINGS_H
#  include <strings.h>
#endif

#include "apparent-events-table-writer.h"

/** This must match an element name in the DTD. */
#define MODEL_NAME "apparent-events-table-writer"



#define NEVENTS_LISTENED_FOR 5
EVT_event_type_t events_listened_for[] = { EVT_BeforeAnySimulations,
  EVT_BeforeEachSimulation, EVT_Detection, EVT_Vaccination, EVT_Destruction };



/* Specialized information for this model. */
typedef struct
{
  char *filename; /* with the .csv extension */
  FILE *stream; /* The open file. */
  gboolean stream_is_stdout;
  int run_number;
}
local_data_t;



/**
 * Before any simulations, this module sets the run number to zero, opens its
 * output file and writes the table header.
 *
 * @param self the model.
 */
void
handle_before_any_simulations_event (struct naadsm_model_t_ * self)
{
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER handle_before_any_simulations_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  if (!local_data->stream_is_stdout)
    {
      errno = 0;
      local_data->stream = fopen (local_data->filename, "w");
      if (errno != 0)
        {
          g_error ("%s: %s error when attempting to open file \"%s\"",
                   MODEL_NAME, strerror(errno), local_data->filename);
        }
    }
  fprintf (local_data->stream, "Run,Day,Type,Reason,ID,Production type,Size,Lat,Lon,Zone\n");
  fflush (local_data->stream);

  /* This count will be incremented for each new simulation. */
  local_data->run_number = 0;

#if DEBUG
  g_debug ("----- EXIT handle_before_any_simulations_event (%s)", MODEL_NAME);
#endif
}



/**
 * Before each simulation, this module increments its "run number".
 *
 * @param self the model.
 */
void
handle_before_each_simulation_event (struct naadsm_model_t_ * self)
{
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER handle_before_each_simulation_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  local_data->run_number++;

#if DEBUG
  g_debug ("----- EXIT handle_before_each_simulation_event (%s)", MODEL_NAME);
#endif
}



/**
 * Responds to a detection event by writing a table row.
 *
 * @param self the model.
 * @param event a detection event.
 * @param zones the list of zones.
 */
void
handle_detection_event (struct naadsm_model_t_ *self,
                        EVT_detection_event_t * event,
                        ZON_zone_list_t *zones)
{
  local_data_t *local_data;
  ZON_zone_fragment_t *zone, *background_zone;

#if DEBUG
  g_debug ("----- ENTER handle_detection_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  zone = zones->membership[event->herd->index];
  background_zone = ZON_zone_list_get_background (zones);

  /* The data fields are: run, day, type, reason, ID, production type, size,
   * latitude, longitude, zone. */
  fprintf (local_data->stream,
           "%i,%i,Detection,%s,%s,%s,%u,%g,%g,%s\n",
           local_data->run_number,
           event->day,
           NAADSM_detection_reason_abbrev[event->means],
           event->herd->official_id,
           event->herd->production_type_name,
           event->herd->size,
           event->herd->latitude,
           event->herd->longitude,
           ZON_same_zone (zone, background_zone) ? "" : zone->parent->name);

#if DEBUG
  g_debug ("----- EXIT handle_detection_event (%s)", MODEL_NAME);
#endif
  return;
}



/**
 * Responds to a vaccination event by writing a table row.
 *
 * @param self the model.
 * @param event a vaccination event.
 * @param zones the list of zones.
 */
void
handle_vaccination_event (struct naadsm_model_t_ *self,
                          EVT_vaccination_event_t * event,
                          ZON_zone_list_t *zones)
{
  local_data_t *local_data;
  ZON_zone_fragment_t *zone, *background_zone;

#if DEBUG
  g_debug ("----- ENTER handle_vaccination_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  zone = zones->membership[event->herd->index];
  background_zone = ZON_zone_list_get_background (zones);

  /* The data fields are: run, day, type, reason, ID, production type, size,
   * latitude, longitude, zone. */
  fprintf (local_data->stream,
           "%i,%i,Vaccination,%s,%s,%s,%u,%g,%g,%s\n",
           local_data->run_number,
           event->day,
           event->reason,
           event->herd->official_id,
           event->herd->production_type_name,
           event->herd->size,
           event->herd->latitude,
           event->herd->longitude,
           ZON_same_zone (zone, background_zone) ? "" : zone->parent->name);

#if DEBUG
  g_debug ("----- EXIT handle_vaccination_event (%s)", MODEL_NAME);
#endif
  return;
}



/**
 * Responds to a destruction event by writing a table row.
 *
 * @param self the model.
 * @param event a destruction event.
 * @param zones the list of zones.
 */
void
handle_destruction_event (struct naadsm_model_t_ *self,
                          EVT_destruction_event_t * event,
                          ZON_zone_list_t *zones)
{
  local_data_t *local_data;
  ZON_zone_fragment_t *zone, *background_zone;

#if DEBUG
  g_debug ("----- ENTER handle_destruction_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  zone = zones->membership[event->herd->index];
  background_zone = ZON_zone_list_get_background (zones);

  /* The data fields are: run, day, type, reason, ID, production type, size,
   * latitude, longitude, zone. */
  fprintf (local_data->stream,
           "%i,%i,Destruction,%s,%s,%s,%u,%g,%g,%s\n",
           local_data->run_number,
           event->day,
           event->reason,
           event->herd->official_id,
           event->herd->production_type_name,
           event->herd->size,
           event->herd->latitude,
           event->herd->longitude,
           ZON_same_zone (zone, background_zone) ? "" : zone->parent->name);

#if DEBUG
  g_debug ("----- EXIT handle_destruction_event (%s)", MODEL_NAME);
#endif
  return;
}



/**
 * Runs this model.
 *
 * @param self the model.
 * @param herds a list of herds.
 * @param zones a list of zones.
 * @param event the event that caused the model to run.
 * @param rng a random number generator.
 * @param queue for any new events the model creates.
 */
void
run (struct naadsm_model_t_ *self, HRD_herd_list_t * herds,
     ZON_zone_list_t * zones, EVT_event_t * event, RAN_gen_t * rng, EVT_event_queue_t * queue)
{
#if DEBUG
  g_debug ("----- ENTER run (%s)", MODEL_NAME);
#endif

  switch (event->type)
    {
    case EVT_BeforeAnySimulations:
      handle_before_any_simulations_event (self);
      break;
    case EVT_BeforeEachSimulation:
      handle_before_each_simulation_event (self);
      break;
    case EVT_Detection:
      handle_detection_event (self, &(event->u.detection), zones);
      break;
    case EVT_Vaccination:
      handle_vaccination_event (self, &(event->u.vaccination), zones);
      break;
    case EVT_Destruction:
      handle_destruction_event (self, &(event->u.destruction), zones);
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
  return;
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
 * Reports whether this module has any pending infections to cause.
 *
 * @param self this module.
 * @return TRUE if this module has pending infections.
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
  local_data_t *local_data;
  GString *s;
  char *chararray;

  local_data = (local_data_t *) (self->model_data);
  s = g_string_new (NULL);
  g_string_sprintf (s, "<%s filename=\"%s\">", MODEL_NAME, local_data->filename);

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

  local_data = (local_data_t *) (self->model_data);

  /* Flush and close the file. */
  if (local_data->stream_is_stdout)
    fflush (local_data->stream);
  else
    fclose (local_data->stream);

  /* Free the dynamically-allocated parts. */
  g_free (local_data->filename);
  g_free (local_data);
  g_ptr_array_free (self->outputs, TRUE);
  g_free (self);

#if DEBUG
  g_debug ("----- EXIT free (%s)", MODEL_NAME);
#endif
}



/**
 * Returns whether this module is a singleton or not.
 */
gboolean
is_singleton (void)
{
  return TRUE;
}



/**
 * Returns a new apparent events table writer.
 */
naadsm_model_t *
new (scew_element * params, HRD_herd_list_t * herds, projPJ projection,
     ZON_zone_list_t * zones)
{
  naadsm_model_t *self;
  local_data_t *local_data;
  scew_element const *e;

#if DEBUG
  g_debug ("----- ENTER new (%s)", MODEL_NAME);
#endif

  self = g_new (naadsm_model_t, 1);
  local_data = g_new (local_data_t, 1);

  self->name = MODEL_NAME;
  self->events_listened_for = events_listened_for;
  self->nevents_listened_for = NEVENTS_LISTENED_FOR;
  self->outputs = g_ptr_array_sized_new (0);
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

  /* Get the filename for the table.  If the filename is omitted, blank, '-',
   * or 'stdout' (case insensitive), then the table is written to standard
   * output. */
  e = scew_element_by_name (params, "filename");
  if (e == NULL)
    {
      local_data->filename = g_strdup ("stdout"); /* just so we have something
        to display, and to free later */
      local_data->stream = stdout;
      local_data->stream_is_stdout = TRUE;    
    }
  else
    {
      local_data->filename = PAR_get_text (e);
      if (local_data->filename == NULL
          || g_ascii_strcasecmp (local_data->filename, "") == 0
          || g_ascii_strcasecmp (local_data->filename, "-") == 0
          || g_ascii_strcasecmp (local_data->filename, "stdout") == 0)
        {
          local_data->stream = stdout;
          local_data->stream_is_stdout = TRUE;
        }
      else
        {
          char *tmp;
          if (!g_str_has_suffix(local_data->filename, ".csv"))
            {
              tmp = local_data->filename;
              local_data->filename = g_strdup_printf("%s.csv", tmp);
              g_free(tmp);
            }
          tmp = local_data->filename;
          local_data->filename = naadsm_insert_node_number_into_filename (local_data->filename);
          g_free(tmp);
          local_data->stream_is_stdout = FALSE;
        }
    }

#if DEBUG
  g_debug ("----- EXIT new (%s)", MODEL_NAME);
#endif

  return self;
}

/* end of file apparent-events-table-writer.c */
