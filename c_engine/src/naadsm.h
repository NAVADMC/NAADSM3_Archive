/** @file naadsm.h
 *
 * @author Aaron Reeves <Aaron.Reeves@colostate.edu><br>
 *   Animal Population Health Institute<br>
 *   College of Veterinary Medicine and Biomedical Sciences<br>
 *   Colorado State University<br>
 *   Fort Collins, CO 80523<br>
 *   USA
 * @version 0.1
 * @date June 2005
 *
 * Copyright &copy; 2005 - 2009 Animal Population Health Institute,
 * Colorado State University
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef naadsm_H
#define naadsm_H

#if defined(DLL_EXPORTS)
# define DLL_API __declspec( dllexport )
#elif defined(DLL_IMPORTS)
# define DLL_API __declspec( dllimport )
#else
# define DLL_API
#endif

#include <glib.h>

#include "herd.h"
#include "zone.h"


/*  Defines for premature stopping of the simulation  */
#define STOP_NORMAL              ((guint) 0x0000 )
#define STOP_ON_DISEASE_END      ((guint) 0x0001 )
#define STOP_ON_FIRST_DETECTION  ((guint) 0x0002 )

#define get_stop_on_disease_end( x )     ((guint)( ((guint)x) & STOP_ON_DISEASE_END ))
#define get_stop_on_first_detection( x ) ((guint)( ((guint)x) & STOP_ON_FIRST_DETECTION ))


/* Enums used by the library to interact with calling applications.   */
/* ------------------------------------------------------------------ */
/* Used to indicate success of failure of exposures, traces, and detection by herd exams */
typedef enum {
  NAADSM_SuccessUnspecified,
  NAADSM_SuccessTrue,
  NAADSM_SuccessFalse
} NAADSM_success;

/* Used to indicate trace direction  */
typedef enum {
  NAADSM_TraceNeither,
  NAADSM_TraceForwardOrOut,
  NAADSM_TraceBackOrIn
} NAADSM_trace_direction;
extern const char *NAADSM_trace_direction_name[];
extern const char *NAADSM_trace_direction_abbrev[];

/* Used to indicate type of exposure, contact, or infection  */
typedef enum {
  NAADSM_UnspecifiedInfectionType,
  NAADSM_DirectContact,
  NAADSM_IndirectContact,
  NAADSM_AirborneSpread,
  NAADSM_InitiallyInfected
} NAADSM_contact_type;
#define NAADSM_NCONTACT_TYPES 5
extern const char *NAADSM_contact_type_name[];
extern const char *NAADSM_contact_type_abbrev[];

/* Used to indicate diagnostic test results  */
typedef enum {
  NAADSM_TestUnspecified,
  NAADSM_TestTruePositive,
  NAADSM_TestTrueNegative,
  NAADSM_TestFalsePositive,
  NAADSM_TestFalseNegative
} NAADSM_test_result;

/* Used to indicate reasons for detection  */
typedef enum {
  NAADSM_DetectionReasonUnspecified,
  NAADSM_DetectionClinicalSigns,
  NAADSM_DetectionDiagnosticTest,
  NAADSM_NDETECTION_REASONS
} NAADSM_detection_reason;
extern const char *NAADSM_detection_reason_abbrev[];


/* Used to indicate reasons for control activities  */
typedef enum {
  NAADSM_ControlReasonUnspecified,
  NAADSM_ControlRing,
  NAADSM_ControlTraceForwardDirect,
  NAADSM_ControlTraceForwardIndirect,
  NAADSM_ControlTraceBackDirect,
  NAADSM_ControlTraceBackIndirect,
  NAADSM_ControlDetection,
  NAADSM_ControlInitialState,
  NAADSM_NCONTROL_REASONS
} NAADSM_control_reason;
extern const char *NAADSM_control_reason_name[];
extern const char *NAADSM_control_reason_abbrev[];

/* Used when a herd's actual disease state changes */
/* FIXME: Consider combining with the appropriate enum type used internally */
typedef enum {
  NAADSM_StateSusceptible,
  NAADSM_StateLatent,
  NAADSM_StateInfectiousSubclinical,
  NAADSM_StateInfectiousClinical,
  NAADSM_StateNaturallyImmune,
  NAADSM_StateVaccineImmune,
  NAADSM_StateDestroyed,
  NAADSM_StateUnspecified
} NAADSM_disease_state;

/* =================================================================================== */
/* FIXME: Consider combining these structs with the similar structs defined in event.h */
/* =================================================================================== */
/** Struct used by callers of the NAADSM library when a herd's actual disease status has changed */
typedef struct
{
  unsigned int herd_index;  /* Index into the herd list of the herd that's changed */
  NAADSM_disease_state status;
}
HRD_update_t;


/** Struct used by callers of the NAADSM library when a herd is infected */
typedef struct
{
  unsigned int herd_index;
  NAADSM_contact_type infection_source_type;
}
HRD_infect_t;


/** Struct used by callers of the NAADSM library when a detection occurs. */
typedef struct
{
  unsigned int herd_index;
  NAADSM_detection_reason reason;
  NAADSM_test_result test_result;
}
HRD_detect_t;


/** Struct used by callers of the NAADSM library when a herd is destroyed or vaccinated. */
typedef struct
{
  unsigned int herd_index;
  NAADSM_control_reason reason;
  int day_commitment_made;
}
HRD_control_t;

  
/** Struct used by callers of the NAADSM library when an exposure
 * has occurred.  If an "attempt to infect" event is generated, 
 * the attempt is considered successful.
 *
 * Note that exposures may preceed infection by some period
 * of time.
*/
typedef struct
{
  unsigned int src_index;
  NAADSM_disease_state src_status;
  unsigned int dest_index;
  NAADSM_disease_state dest_status;
  int initiated_day;
  int finalized_day;
  NAADSM_success is_adequate;
  NAADSM_contact_type exposure_method;
}
HRD_expose_t;


/** Struct used by callers of the NAADSM library when a herd is traced. */
typedef struct
{
  unsigned int identified_index;
  NAADSM_disease_state identified_status;
  unsigned int origin_index;
  NAADSM_disease_state origin_status;
  int day;
  int initiated_day;
  NAADSM_success success;
  NAADSM_trace_direction trace_type;
  NAADSM_contact_type contact_type;
}
HRD_trace_t;


/** Struct used by callers of the NAADSM library when a herd is examined after tracing. */
typedef struct
{
  int herd_index;
  NAADSM_trace_direction trace_type;
  NAADSM_contact_type contact_type;
  NAADSM_success disease_detected;
}
HRD_exam_t;


/** Struct used by callers of the NAADSM library when a herd is diagnostically tested after tracing. */
typedef struct
{
  int herd_index;
  NAADSM_test_result test_result;
  NAADSM_trace_direction trace_type;
  NAADSM_contact_type contact_type;
}
HRD_test_t;          
       
          
/** Notification for the GUI that a herd's zone designation has changed. */
typedef struct
{
  unsigned int herd_index;
  unsigned int zone_level;  
}
HRD_zone_t;


/* Function to start the simulation */
#ifdef USE_SC_GUILIB
DLL_API void
run_sim_main (const char *herd_file,
              const char *parameter_file,
              const char *output_file,
              double fixed_rng_value, int verbosity, int seed, char *production_type_file);
#else
DLL_API void
run_sim_main (const char *herd_file,
              const char *parameter_file,
              const char *output_file,
              double fixed_rng_value, int verbosity, int seed);
#endif


/* Functions for version tracking */
/* ------------------------------ */
/** Returns the current version of this application. */
DLL_API char *current_version (void);

/** Returns the version of the model specification that
this application or DLL is intended to comply with */
DLL_API char *specification_version (void);


/* Function pointer types */
/*------------------------*/
typedef void (*TFnVoid_1_CharP) (char *);
typedef void (*TFnVoid_1_Int) (int);
typedef void (*TFnVoid_1_THRDUpdate) (HRD_update_t);
typedef void (*TFnVoid_1_THRDInfect) (HRD_infect_t);
typedef void (*TFnVoid_1_THRDDetect) (HRD_detect_t);
typedef void (*TFnVoid_1_THRDControl) (HRD_control_t);
typedef void (*TFnVoid_1_THRDExpose) (HRD_expose_t);
typedef void (*TFnVoid_1_THRDTrace) (HRD_trace_t);
typedef void (*TFnVoid_1_THRDExam) (HRD_exam_t);
typedef void (*TFnVoid_1_THRDTest) (HRD_test_t);
typedef void (*TFnVoid_1_THRDZone) (HRD_zone_t);
typedef void (*TFnVoid_0) (void);
typedef int (*TFnInt_0) (void);
typedef void (*TFnVoid_1_THRDPerimeterList) (ZON_zone_list_t *);
typedef void (*TFnVoid_2_Int_Double) (int, double);
typedef void (*TFnVoid_5_Int_Int_Int_Int_Int) (int, int, int, int, int);

/* Function pointers */
/*-------------------*/
/* For the display of debugging information in the GUI */
TFnVoid_1_CharP naadsm_printf;
TFnVoid_1_CharP naadsm_debug;

/* For key simulation- and iteration-level events */
TFnVoid_0 naadsm_sim_start;
TFnVoid_1_Int naadsm_iteration_start;
TFnVoid_1_Int naadsm_day_start;
TFnVoid_1_Int naadsm_day_complete;
TFnVoid_1_Int naadsm_disease_end;
TFnVoid_1_Int naadsm_outbreak_end;
TFnVoid_1_Int naadsm_iteration_complete;
TFnVoid_1_Int naadsm_sim_complete;

/* Used to determine whether the user wants to interrupt a running simulation */
TFnInt_0 naadsm_simulation_stop;

/* Used to update herd status and related events as an iteration runs */
TFnVoid_1_THRDUpdate naadsm_change_herd_state;
TFnVoid_1_THRDInfect naadsm_infect_herd;
TFnVoid_1_THRDDetect naadsm_detect_herd;
TFnVoid_1_THRDExpose naadsm_expose_herd;
TFnVoid_1_THRDTrace naadsm_trace_herd;
TFnVoid_1_THRDExam naadsm_examine_herd;
TFnVoid_1_THRDTest naadsm_test_herd;
TFnVoid_1_Int naadsm_queue_herd_for_destruction;
TFnVoid_1_THRDControl naadsm_destroy_herd;
TFnVoid_1_Int naadsm_queue_herd_for_vaccination;
TFnVoid_1_THRDControl naadsm_vaccinate_herd;
TFnVoid_1_THRDControl naadsm_cancel_herd_vaccination;
TFnVoid_1_Int naadsm_make_zone_focus;
TFnVoid_1_THRDZone naadsm_record_zone_change;
TFnVoid_2_Int_Double naadsm_record_zone_area;
TFnVoid_2_Int_Double naadsm_record_zone_perimeter;

/* Used by the GUI to access zone information during a running simulation */
TFnVoid_1_THRDPerimeterList naadsm_set_zone_perimeters;

/* Used to write daily herd state output, when desired */
TFnVoid_1_CharP naadsm_show_all_states;

/* Used to write daily herd prevalence output, when desired */
TFnVoid_1_CharP naadsm_show_all_prevalences;

/* Used to display g_warnings, etc., in the GUI */
TFnVoid_1_CharP naadsm_display_g_message;

/* Used to write daily herd zone output, when desired */
/* This function will need to be re-implemented if it is ever needed again. */
/* TFnVoid_1_CharP naadsm_show_all_zones; */

TFnVoid_5_Int_Int_Int_Int_Int naadsm_report_search_hits;


/* Functions used to set the function pointers */
/*---------------------------------------------*/
DLL_API void set_printf (TFnVoid_1_CharP fn);
DLL_API void set_debug (TFnVoid_1_CharP fn);

DLL_API void set_sim_start (TFnVoid_0 fn);
DLL_API void set_iteration_start (TFnVoid_1_Int fn);
DLL_API void set_day_start (TFnVoid_1_Int fn);
DLL_API void set_day_complete (TFnVoid_1_Int fn);
DLL_API void set_disease_end (TFnVoid_1_Int fn);
DLL_API void set_outbreak_end (TFnVoid_1_Int fn);
DLL_API void set_iteration_complete (TFnVoid_1_Int fn);
DLL_API void set_sim_complete (TFnVoid_1_Int fn);

DLL_API void set_change_herd_state (TFnVoid_1_THRDUpdate fn);
DLL_API void set_infect_herd (TFnVoid_1_THRDInfect fn);
DLL_API void set_expose_herd (TFnVoid_1_THRDExpose fn);
DLL_API void set_detect_herd (TFnVoid_1_THRDDetect fn);
DLL_API void set_trace_herd (TFnVoid_1_THRDTrace fn);
DLL_API void set_examine_herd (TFnVoid_1_THRDExam fn);
DLL_API void set_test_herd (TFnVoid_1_THRDTest fn);
DLL_API void set_queue_herd_for_destruction (TFnVoid_1_Int fn);
DLL_API void set_destroy_herd (TFnVoid_1_THRDControl fn);
DLL_API void set_queue_herd_for_vaccination (TFnVoid_1_Int fn);
DLL_API void set_vaccinate_herd (TFnVoid_1_THRDControl fn);
DLL_API void set_cancel_herd_vaccination (TFnVoid_1_THRDControl fn);
DLL_API void set_make_zone_focus( TFnVoid_1_Int fn );
DLL_API void set_record_zone_change (TFnVoid_1_THRDZone fn);
DLL_API void set_record_zone_area (TFnVoid_2_Int_Double fn);
DLL_API void set_record_zone_perimeter (TFnVoid_2_Int_Double fn);

DLL_API void set_set_zone_perimeters( TFnVoid_1_THRDPerimeterList fn);
DLL_API unsigned int get_zone_list_length( ZON_zone_list_t *zones );
DLL_API ZON_zone_t *get_zone_from_list( ZON_zone_list_t * zones, int i);

DLL_API void set_show_all_states (TFnVoid_1_CharP fn);
DLL_API void set_show_all_prevalences (TFnVoid_1_CharP fn);

/* This function will need to be re-implemented if it is ever needed again. */
/* DLL_API void set_show_all_zones (TFnVoid_1_CharP fn); */

DLL_API void set_display_g_message (TFnVoid_1_CharP fn);

DLL_API void set_simulation_stop (TFnInt_0 fn);

DLL_API void set_report_search_hits (TFnVoid_5_Int_Int_Int_Int_Int fn);


/* Function pointer helpers */
/*--------------------------*/
void clear_naadsm_fns (void);
void naadsm_log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data);

#endif /* naadsm_H */
