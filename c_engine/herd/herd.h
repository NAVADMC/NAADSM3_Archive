/** @file herd.h
 * State information about a herd of animals.
 *
 * A herd contains one production type and has a size, location (x and y),
 * state, and prevalence.  Sub-models may read these data fields, but they
 * should modify a herd only through the functions HRD_infect(),
 * HRD_vaccinate(), HRD_destroy(), HRD_quarantine(), and HRD_lift_quarantine().
 * The first three functions correspond to the action labels on the
 * <a href="herd_8h.html#a42">herd state-transition diagram</a>.
 *
 * Because the events in one simulation day should be considered to happen
 * simultaneously, and because different sub-models may try to make conflicting
 * changes to a herd, the functions named above do not change a herd
 * immediately.  Instead, the request for a change is stored, and conflicts
 * between the change requests are resolved before any changes are applied.
 * See HRD_step() for details.
 *
 * Symbols from this module begin with HRD_.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 *
 * Copyright &copy; University of Guelph, 2003-2010
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef HERD_H
#define HERD_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>

#if STDC_HEADERS
#  include <stdlib.h>
#endif

#include "rel_chart.h"
#include "spatial_search.h"
#include <glib.h>
#include <proj_api.h>

#ifdef USE_SC_GUILIB
#  include <production_type_data.h>
#  include <zone.h>
#endif


/**
 * Production types.
 */
typedef unsigned int HRD_production_type_t;



/**
 * Number of possible states (with respect to a disease) for a herd.
 *
 * @sa HRD_status_t
 */
#define HRD_NSTATES 7

/**
 * Possible states (with respect to a disease) for a herd.  The diagram below
 * is based on one from Mark Schoenbaum's presentation to the North American
 * Animal Health Committee's (NAAHC) Emergency Management Working Group at the
 * Disease Spread Modeling Workshop, Fort Collins, CO July 9-11, 2002.  The
 * single Infectious state has been split into two.
 *
 * @image html state-transition.png
 *
 * @sa HRD_valid_transition
 */
typedef enum
{
  Susceptible, Latent, InfectiousSubclinical, InfectiousClinical,
  NaturallyImmune, VaccineImmune, Destroyed
}
HRD_status_t;
extern const char *HRD_status_name[];


typedef enum
{
  asUnspecified, asUnknown, asDetected, asTraceDirect, asTraceIndirect, asVaccinated, asDestroyed
}
HRD_apparent_status_t;


/**
 * Number of actions/changes that can be made to a herd.
 *
 * @sa HRD_change_request_type_t
 */
#define HRD_NCHANGE_REQUEST_TYPES 5

/**
 * Actions/changes that can be made to a herd.
 */
typedef enum
{
  Infect, Vaccinate, Quarantine, LiftQuarantine, Destroy
}
HRD_change_request_type_t;

/** A request to infect a herd. */
typedef struct
{
  int latent_period;
  int infectious_subclinical_period;
  int infectious_clinical_period;
  int immunity_period;
  unsigned int day_in_disease_cycle;
}
HRD_infect_change_request_t;



/** A request to vaccinate a herd. */
typedef struct
{
  int delay;
  int immunity_period;
}
HRD_vaccinate_change_request_t;



/** A request to quarantine a herd. */
typedef struct
{
  int dummy;
}
HRD_quarantine_change_request_t;



/** A request to lift the quarantine on a herd. */
typedef struct
{
  int dummy;
}
HRD_lift_quarantine_change_request_t;



/** A request to destroy a herd. */
typedef struct
{
  int dummy;
}
HRD_destroy_change_request_t;



/** A supertype for all change requests. */
typedef struct
{
  HRD_change_request_type_t type;
  union
  {
    HRD_infect_change_request_t infect;
    HRD_vaccinate_change_request_t vaccinate;
    HRD_quarantine_change_request_t quarantine;
    HRD_lift_quarantine_change_request_t lift_quarantine;
    HRD_destroy_change_request_t destroy;
  }
  u;
}
HRD_change_request_t;



/** Type of a herd's identifier. */
typedef char *HRD_id_t;


/** Complete state information for a herd. */
typedef struct
{
  unsigned int index;           /**< position in a herd list */
  HRD_production_type_t production_type;  
  char *production_type_name;
  HRD_id_t official_id;         /**< arbitrary identifier string */
  unsigned int size;            /**< number of animals */
  double latitude, longitude;
  double x;                     /**< x-coordinate on a km grid */
  double y;                     /**< y-coordinate on a km grid */
  HRD_status_t status;
  HRD_status_t initial_status;
  int days_in_initial_status;
  int days_left_in_initial_status;
  double prevalence;

  /* Remaining fields should be considered private. */

  gboolean quarantined;
  int days_in_status;

  gboolean in_vaccine_cycle;
  int immunity_start_countdown;
  int immunity_end_countdown;

  gboolean in_disease_cycle;
  int day_in_disease_cycle;
  int infectious_start_countdown;
  int clinical_start_countdown;   
  
  REL_chart_t *prevalence_curve;

  GSList *change_requests;
  
#ifdef USE_SC_GUILIB  
  /*  This field is used on the NAADSM-SC version if the user wants to 
      write out the iteration summaries, as would be found in the NAADSM-PC
      software.  This hooks into some of the "naadsm_*" functions in order to set this
      data.  This allows for easy import of SC data into the PC database.
  */
  GPtrArray *production_types;  /**< Each item is a HRD_production_type_data_t structure */
  gboolean ever_infected;
  int day_first_infected;
  ZON_zone_t *zone;
  guint cum_infected, cum_detected, cum_destroyed, cum_vaccinated;
  HRD_apparent_status_t apparent_status;  
  guint apparent_status_day;
#endif    
}
HRD_herd_t;



/** A list of herds. */
typedef struct
{
  GArray *list; /**< Each item is a HRD_herd_t structure. */
  GPtrArray *production_type_names; /**< Each pointer is to a regular C string. */
  
#ifdef USE_SC_GUILIB  
  /*  This field is used on the NAADSM-SC version if the user wants to 
      write out the iteration summaries, as would be found in the NAADSM-PC
      software.  This hooks into some of the "naadsm_*" functions in order to set this
      data.  This allows for easy import of SC data into the PC database.
  */
  GPtrArray *production_types;  /**< Each item is a HRD_production_type_data_t structure */
#endif   

  spatial_search_t *spatial_index;

  projPJ projection; /**< The projection used to convert between the latitude,
    longitude and x,y locations of the herds.  Note that the projection object
    works in meters, while the x,y locations are stored in kilometers. */
}
HRD_herd_list_t;



/* Prototypes. */

HRD_herd_list_t *HRD_new_herd_list (void);

#ifdef USE_SC_GUILIB 
  HRD_herd_list_t *HRD_load_herd_list ( const char *filename, GPtrArray *production_types );
  HRD_herd_list_t *HRD_load_herd_list_from_stream (FILE *stream, const char *filename, GPtrArray *production_types);  
#else
  HRD_herd_list_t *HRD_load_herd_list (const char *filename);
  HRD_herd_list_t *HRD_load_herd_list_from_stream (FILE *stream, const char *filename);
#endif


void HRD_free_herd_list (HRD_herd_list_t *);
unsigned int HRD_herd_list_append (HRD_herd_list_t *, HRD_herd_t *);

/**
 * Returns the number of herds in a herd list.
 *
 * @param H a herd list.
 * @return the number of herds in the list.
 */
#define HRD_herd_list_length(H) (H->list->len)

/**
 * Returns the ith herd in a herd list.
 *
 * @param H a herd list.
 * @param I the index of the herd to retrieve.
 * @return the ith herd.
 */
#define HRD_herd_list_get(H,I) (&g_array_index(H->list,HRD_herd_t,I))

unsigned int HRD_herd_list_get_by_status (HRD_herd_list_t *, HRD_status_t, HRD_herd_t ***);
unsigned int HRD_herd_list_get_by_initial_status (HRD_herd_list_t *, HRD_status_t, HRD_herd_t ***);
void HRD_herd_list_project (HRD_herd_list_t *, projPJ);
void HRD_herd_list_build_spatial_index (HRD_herd_list_t *);
char *HRD_herd_list_to_string (HRD_herd_list_t *);
int HRD_printf_herd_list (HRD_herd_list_t *);
int HRD_fprintf_herd_list (FILE *, HRD_herd_list_t *);
char *HRD_herd_list_summary_to_string (HRD_herd_list_t *);
char *HRD_herd_list_prevalence_to_string (HRD_herd_list_t *, unsigned int day);
int HRD_printf_herd_list_summary (HRD_herd_list_t *);
int HRD_fprintf_herd_list_summary (FILE *, HRD_herd_list_t *);

HRD_herd_t *HRD_new_herd (HRD_production_type_t, char *production_type_name,
                          unsigned int size, double x, double y);
void HRD_free_herd (HRD_herd_t *, gboolean free_segment);
char *HRD_herd_to_string (HRD_herd_t *);
int HRD_fprintf_herd (FILE *, HRD_herd_t *);

#define HRD_printf_herd(H) HRD_fprintf_herd(stdout,H)

void HRD_reset (HRD_herd_t *);
void HRD_step (HRD_herd_t *, GHashTable *infectious_herds);
void HRD_infect (HRD_herd_t *, int latent_period,
                 int infectious_subclinical_period,
                 int infectious_clinical_period,
                 int immunity_period,
                 unsigned int day_in_disease_cycle);
void HRD_vaccinate (HRD_herd_t *, int delay, int immunity_period);
void HRD_quarantine (HRD_herd_t *);
void HRD_lift_quarantine (HRD_herd_t *);
void HRD_destroy (HRD_herd_t *);

void HRD_remove_herd_from_infectious_list( HRD_herd_t *, GHashTable * ); 
void HRD_add_herd_to_infectious_list( HRD_herd_t *, GHashTable * );   

#endif /* !HERD_H */
