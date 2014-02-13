/** @file sc_guilib_outputs.h
 *
 * @author Shaun Case <ShaunCase@colostate.edu><br>
 *   Animal Population Health Institute<br>
 *   College of Veterinary Medicine and Biomedical Sciences<br>
 *   Colorado State University<br>
 *   Fort Collins, CO 80523<br>
 *   USA
 * @version 0.1
 * @date January 2009
 *
 * Copyright &copy; 2005 - 2009 Animal Population Health Institute, 
 * Colorado State University
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef SC_naadsm_OUTPUTS_H
#define SC_naadsm_OUTPUTS_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#if STDC_HEADERS
#  include <stdlib.h>
#endif

  #ifdef USE_SC_GUILIB
  
    #include <glib.h>

    #include <general.h>
    
    /*  local .h files, in one of many directories for this project, so use 
        "<>" here and "-I" in CFLAGS */
    #include <production_type_data.h>

    /* For key simulation- and iteration-level events */
    void sc_sim_start( HRD_herd_list_t *herds, GPtrArray *production_types, ZON_zone_list_t *zones );
    void sc_iteration_start( GPtrArray *_production_types, HRD_herd_list_t *herds, unsigned int _run );
    void sc_iteration_complete( ZON_zone_list_t *_zones, HRD_herd_list_t *_herds, GPtrArray *_production_types, unsigned int _run );
    void sc_sim_complete( int _status, HRD_herd_list_t *herds, GPtrArray *production_types, ZON_zone_list_t *zones );
    void sc_disease_end( int _day );    
    void sc_outbreak_end( int _day );
    void sc_day_start( GPtrArray *_production_types );    
    void sc_day_complete( guint _day, guint _run, GPtrArray *_production_types, ZON_zone_list_t *_zones );
        
/**  Not used  
    void sc_day_start(int );
**/
    
    /* Used to update herd status and related events as an iteration runs */
    void sc_change_herd_state(  HRD_herd_t *_herd, HRD_update_t _update );    
    void sc_infect_herd( unsigned short int _day, HRD_herd_t *_herd, HRD_update_t _update );
    void sc_expose_herd( HRD_herd_t *_exposed_herd, HRD_update_t _update );
    void sc_attempt_trace_herd( HRD_herd_t *_exposed_herd, HRD_update_t _update );        
    void sc_detect_herd( unsigned short int _day, HRD_herd_t *_herd, HRD_update_t _update );
    void sc_destroy_herd( unsigned short int _day, HRD_herd_t *_herd, HRD_update_t _update );
    void sc_vaccinate_herd( unsigned short int _day, HRD_herd_t *_herd, HRD_update_t _update );
    void sc_make_zone_focus( unsigned short int _day, HRD_herd_t *_herd );
    void sc_record_zone_area ( unsigned short int _day, ZON_zone_t *_zone );        
    void sc_record_zone_change( HRD_herd_t *_herd, ZON_zone_t *_zone );        
        
    /* void  sc_trace_herd(HRD_update_t); *//* Not currently used --  taken care of by attempt_trace_herd*/

/**  Not used
    void sc_record_exposure(HRD_expose_t);
**/    
       
    void clear_herd_zones_list( GPtrArray *_herdsInZones );  
  #endif
#endif
