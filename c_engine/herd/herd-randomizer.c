/** @file herd-randomizer.c
 * FIX ME: Add comment
 *
 * @author Aaron Reeves <Aaron.Reeves@colostate.edu><br>
 *   Animal Population Health Institute<br>
 *   College of Veterinary Medicine and Biomedical Sciences<br>
 *   Colorado State University<br>
 *   Fort Collins, CO 80523<br>
 *   USA
 * @version 0.1
 * @date March 26, 2011
 *
 * Copyright &copy; 2011 - 2012 Colorado State University
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "herd-randomizer.h"

#include <glib.h>
#include <gsl/gsl_randist.h>

#include "naadsm.h"

/**
 * Returns of a list of herds that are Latent, Infectious Subclinical,
 * Infectious Clinical, or Naturally Immune.
 */
unsigned int
get_initially_infected_herds (HRD_herd_list_t * herds, HRD_herd_t *** list)
{
  HRD_herd_t **partial_list;
  unsigned int n;
  GArray *array;
  HRD_status_t state;

  /* Concatenate the lists of herds for each diseased state. */
  array = g_array_new (FALSE, FALSE, sizeof (HRD_herd_t *));

  for (state = Latent; state <= NaturallyImmune; state++)
    {
      n = HRD_herd_list_get_by_initial_status (herds, state, &partial_list);
      g_array_append_vals (array, partial_list, n);
      g_free (partial_list);
    }

  /* Don't return the wrapper object. */
  n = array->len;
  *list = (HRD_herd_t **) (array->data);
  g_array_free (array, FALSE);
  return n;
}



/**
 * Returns of a list of herds that are Vaccine Immune.
 */
unsigned int
get_initially_immune_herds (HRD_herd_list_t * herds, HRD_herd_t *** list)
{
  HRD_herd_t **partial_list;
  unsigned int n;
  GArray *array;

  array = g_array_new (FALSE, FALSE, sizeof (HRD_herd_t *));
  n = HRD_herd_list_get_by_initial_status (herds, VaccineImmune, &partial_list);
  g_array_append_vals (array, partial_list, n);
  g_free (partial_list);

  /* Don't return the wrapper object. */
  n = array->len;
  *list = (HRD_herd_t **) (array->data);
  g_array_free (array, FALSE);
  return n;
}



/**
 * Returns of a list of herds that are Destroyed.
 */
unsigned int
get_initially_destroyed_herds (HRD_herd_list_t * herds, HRD_herd_t *** list)
{
  HRD_herd_t **partial_list;
  unsigned int n;
  GArray *array;

  array = g_array_new (FALSE, FALSE, sizeof (HRD_herd_t *));
  n = HRD_herd_list_get_by_initial_status (herds, Destroyed, &partial_list);
  g_array_append_vals (array, partial_list, n);
  g_free (partial_list);

  /* Don't return the wrapper object. */
  n = array->len;
  *list = (HRD_herd_t **) (array->data);
  g_array_free (array, FALSE);
  return n;
}


/**
 * Returns the herds with a given production type.
 *
 * @param herds a herd list.
 * @param nherds the number of herds in the herd list.
 * @param production_type the desired production_type.
 * @param array a location in which to store the address of a list of pointers
 *   to herds.
 * @return the number of herds with the given production_type.
 */
unsigned int
get_herds_by_production_type_from_array( HRD_herd_t **herds, unsigned int nherds, HRD_production_type_t production_type, GArray* array )
{
  HRD_herd_t* herd;
  unsigned int count = 0;
  unsigned int i;

  for (i = 0; i < nherds; i++)
    {
      herd = herds[i]; 
      if( herd->production_type == production_type )
        {    
          g_array_append_val( array, herd );
          count++;
        }
    }
  return count;
}


unsigned int
get_herds_by_production_type_from_herd_list( 
  HRD_herd_list_t *herds, HRD_production_type_t production_type, GArray* array,
  gboolean set_herd_to_susceptible )
{
  HRD_herd_t* herd;
  unsigned int count = 0;
  unsigned int i, nherds;

  nherds = HRD_herd_list_length( herds );
  for (i = 0; i < nherds; i++)
    {
      herd = HRD_herd_list_get( herds, i );
               
      if( herd->production_type == production_type )
        { 
          if( set_herd_to_susceptible )
            { 
              herd->initial_status = Susceptible;
              herd->days_in_initial_status = 0;
              herd->days_left_in_initial_status = 0;  
            }
                        
          g_array_append_val( array, herd );
          count++;
        }
    }
  return count;
}


#ifdef WHEATLAND
void
randomize_initial_states( HRD_herd_list_t *herds, RAN_gen_t *rng )
{
  unsigned int ninitially_infected_herds, ninitially_immune_herds, ninitially_destroyed_herds, nselected, nall;
  HRD_herd_t **initially_infected_herds, **initially_immune_herds, **initially_destroyed_herds;
  GArray* arr;
  GArray* arr2;
  HRD_herd_t* herd;
  unsigned int i, j, k, h;
  unsigned int n_each_state[ HRD_NSTATES ];
  HRD_herd_t** selected_herds;
  HRD_herd_t** all_herds;
  char str[1024];

  if (NULL != naadsm_printf)
    naadsm_printf ("START randomize_initial_states...");

  /* Record the initially infected herds. */
  ninitially_infected_herds = get_initially_infected_herds (herds, &initially_infected_herds);
  if (ninitially_infected_herds == 0)
    g_warning ("no units initially infected");

  /* Record the initially immune herds. */
  ninitially_immune_herds = get_initially_immune_herds (herds, &initially_immune_herds);

  /* Record the initially destroyed herds. */
  ninitially_destroyed_herds = get_initially_destroyed_herds (herds, &initially_destroyed_herds);

  nselected = ninitially_infected_herds + ninitially_immune_herds + ninitially_destroyed_herds;
  nall = HRD_herd_list_length( herds );

  /* Debugging... */
  if (NULL != naadsm_printf) {
    sprintf( str, "nInfected: %d, nImmune: %d, nDestroyed: %d", ninitially_infected_herds, ninitially_immune_herds, ninitially_destroyed_herds );
    naadsm_printf( str );
  }


  /* How many are in each individual disease state? */
  for( j = 0; j < HRD_NSTATES; ++j )
    n_each_state[j] = 0;

  for (i = 0; i < ninitially_infected_herds; i++)
    {
      herd = initially_infected_herds[i];
       ++n_each_state[herd->initial_status];
    }
  n_each_state[VaccineImmune] = ninitially_immune_herds;
  n_each_state[Destroyed] = ninitially_destroyed_herds;

  /* Set all herds to be initially susceptible.  The right number will be changed back later. */
  for( i = 0; i < nall; ++i ) {
    herd = HRD_herd_list_get( herds, i );
    herd->initial_status = Susceptible;
  }



  /* Choose nselected herds at random and shuffle them. */
  /*----------------------------------------------------*/
  arr = g_array_new( FALSE, FALSE, sizeof( HRD_herd_t* ) );
  /* Fill the array with dummy data.  It will be replaced in the function gsl_ran_choose(). */
  herd = NULL;
  for( i = 0; i < nselected; ++i )
    g_array_append_val( arr, herd );
  selected_herds = (HRD_herd_t**) arr->data;

  arr2 = g_array_new( FALSE, FALSE, sizeof( HRD_herd_t* ) );
  for( i = 0; i < nall; ++i ) {
    herd =  HRD_herd_list_get( herds, i );
    g_array_append_val( arr2, herd );
  }

  all_herds = (HRD_herd_t**) arr2->data;


  /* Debugging... */
  /*
  if (NULL != naadsm_printf) {
    if( NULL == herds )
      naadsm_printf( "herds is NULL");
    if( NULL == herds->list )
      naadsm_printf( "list is NULL" );
    if( NULL == herds->list->data )
      naadsm_printf( "data is NULL" );
  }
  */

  /* Debugging... */
  /*
  if( NULL != naadsm_printf ) {
    naadsm_printf( "Top 5 herds..." );
    for( j = 0; j < 5; ++j ) {
      herd = HRD_herd_list_get( herds, j );
      naadsm_printf( HRD_herd_to_string( herd ) );
    }
    naadsm_printf( "Done with top 5 herds" );


    naadsm_printf( "Top 5 herds again..." );
    for( j = 0; j < 5; ++j ) {
      herd = all_herds[j];
      if( NULL == herd )
        naadsm_printf( "herd is NULL" );
      //naadsm_printf( HRD_herd_to_string( herd ) );
    }
    naadsm_printf( "Done with top 5 herds again" );
  }
  */

  gsl_ran_choose( RAN_generator_as_gsl( rng ), selected_herds, nselected, all_herds, nall, sizeof( HRD_herd_t* ) );
  gsl_ran_shuffle( RAN_generator_as_gsl( rng ), selected_herds, nselected, sizeof( HRD_herd_t* ) );

  /* Debugging... */
  /*
  if( NULL != naadsm_printf )
    {
      naadsm_printf( "Selected, shuffled herds..." );
      for( j = 0; j < nselected; ++j )
        naadsm_printf( HRD_herd_to_string( selected_herds[j] ) );
    }
  */

  /* Set the randomly selected units to their new disease states, based
     on the numbers that should be in each state as determined above. */
  /*------------------------------------------------------------------*/
  h = 0;
  for( j = 0; j < HRD_NSTATES; ++j )
    {
      for( k = 0; k < n_each_state[j]; ++k )
        {
          herd = selected_herds[h];

          /* There is currently no way in Wheatland to preserve the number
           * of days in state or days left in state. */
          herd->days_in_initial_status = 0;
          herd->days_left_in_initial_status = 0;

          /* Set the initial state for the herd. */
          herd->initial_status = j;

          ++h;
        }
    }


  /* Clean-up */
  /*----------*/
  g_array_free( arr2, FALSE );
  g_array_free( arr, FALSE );
  g_free( selected_herds );
  g_free( all_herds );

  g_free( initially_infected_herds );
  g_free( initially_immune_herds );
  g_free( initially_destroyed_herds );

  /* Debugging */
  /*
  if (NULL != naadsm_printf)
    naadsm_printf ("DONE randomize_initial_states.");
  */
}
#endif


#ifdef TORRINGTON
void 
randomize_initial_states( HRD_herd_list_t *herds, RAN_gen_t *rng )
{ 
  unsigned int ninitially_infected_herds, ninitially_immune_herds, ninitially_destroyed_herds;
  HRD_herd_t **initially_infected_herds, **initially_immune_herds, **initially_destroyed_herds;

  unsigned int i, j, k, h;
  unsigned int n, m;
  char* prod_type_name;
  GArray* array0;
  HRD_herd_t* herd;
  unsigned int n_each_state[ HRD_NSTATES ];
  
  GArray* array1;
  GArray* array2;
  HRD_herd_t** all_herds_by_pt;
  HRD_herd_t** selected_herds;
  
  char str[1024];
  
  /*
  if (NULL != naadsm_printf)
    naadsm_printf ("START randomize_initial_states...");  
  */
  
  /* Record the initially infected herds. */
  ninitially_infected_herds = get_initially_infected_herds (herds, &initially_infected_herds);
  if( 0 == ninitially_infected_herds )
    g_warning ("no units initially infected");

  /* Record the initially immune herds. */
  ninitially_immune_herds = get_initially_immune_herds (herds, &initially_immune_herds);

  /* Record the initially destroyed herds. */
  ninitially_destroyed_herds = get_initially_destroyed_herds (herds, &initially_destroyed_herds);  

  /*
  sprintf( str, "nTypes: %d", herds->production_type_names->len );
  if (NULL != naadsm_printf)
    naadsm_printf( str );
  */

  if (NULL != naadsm_printf) {
    sprintf( str, "TORRINGTON: nInfected: %d, nImmune: %d, nDestroyed: %d", ninitially_infected_herds, ninitially_immune_herds, ninitially_destroyed_herds );
    naadsm_printf( str );  
  }

    
 /* For each production type... */
 /*-----------------------------*/      
  for( i = 0; i < herds->production_type_names->len; ++i )
    {   
      prod_type_name = g_ptr_array_index( herds->production_type_names, i );
      
      /* Determine how many units of the selected type are in each disease state. */
      /*--------------------------------------------------------------------------*/
      array0 = g_array_new( FALSE, FALSE, sizeof( HRD_herd_t* ) );
      n = get_herds_by_production_type_from_array( initially_infected_herds, ninitially_infected_herds, i, array0 );
      n = n + get_herds_by_production_type_from_array( initially_immune_herds, ninitially_immune_herds, i, array0 );
      n = n + get_herds_by_production_type_from_array( initially_destroyed_herds, ninitially_destroyed_herds, i, array0 );
        
      for( j = 0; j < HRD_NSTATES; ++j )
        n_each_state[j] = 0; 
    
      for( j = 0; j < n; ++j )
        {
          herd = g_array_index( array0, HRD_herd_t*, j ); 
          ++n_each_state[herd->initial_status];
        }
      
      /* Debugging... */
      /*
      if( NULL != naadsm_printf )
        {
          for( j = 0; j < HRD_NSTATES; ++j )
            {
              sprintf( str, "%s: %d %d", prod_type_name, n_each_state[j], j );
              naadsm_printf( str );
            }
        }
      */
         
      
      if( 0 < n )
        {
          /* Make an array with only units of the selected type. */
          /*-----------------------------------------------------*/
          array1 = g_array_new( FALSE, FALSE, sizeof( HRD_herd_t* ) );     
          m = get_herds_by_production_type_from_herd_list( herds, i, array1, TRUE );
          all_herds_by_pt = (HRD_herd_t**) array1->data;
      
          /* Choose n herds at random for the production type and shuffle them. */
          /*--------------------------------------------------------------------*/
          array2 = g_array_new( FALSE, FALSE, sizeof( HRD_herd_t* ) );
          /* Fill the array with dummy data.  It will be replaced in the function gsl_ran_choose(). */
          herd = NULL;
          for( j = 0; j < n; ++j )
            g_array_append_val( array2, herd );  
          selected_herds = (HRD_herd_t**) array2->data;
          
          gsl_ran_choose( RAN_generator_as_gsl( rng ), selected_herds, n, all_herds_by_pt, m, sizeof( HRD_herd_t* ) );   
          gsl_ran_shuffle( RAN_generator_as_gsl( rng ), selected_herds, n, sizeof( HRD_herd_t* ) ); 
          
          /* Debugging... */
          /*
          if( NULL != naadsm_printf )
            {
              naadsm_printf( "Selected, shuffled herds..." );
              for( j = 0; j < n; ++j )
                naadsm_printf( HRD_herd_to_string( selected_herds[j] ) );
            }
          */
                
          /* Set the randomly selected units to their new disease states, based 
             on the numbers that should be in each state as determined above. */ 
          /*------------------------------------------------------------------*/
          h = 0;
          for( j = 0; j < HRD_NSTATES; ++j )
            {
              for( k = 0; k < n_each_state[j]; ++k )
                {
                  herd = selected_herds[h]; 
                  
                  /* There is currently no way in Torrington to preserve the number 
                   * of days in state or days left in state. */
                  herd->days_in_initial_status = 0;
                  herd->days_left_in_initial_status = 0;                
                  
                  /* Set the initial state for the herd. */
                  herd->initial_status = j;
    
                  ++h;                                  
                }   
            } 
          
          /* Clean up. */
          g_array_free( array1, FALSE );
          g_array_free( array2, FALSE );       
          g_free( all_herds_by_pt );
          g_free( selected_herds );
        }
        
      g_array_free( array0, TRUE );
    }
   
  g_free( initially_infected_herds );
  g_free( initially_immune_herds );
  g_free( initially_destroyed_herds );
  
  /*
  if (NULL != naadsm_printf)
    naadsm_printf ("DONE randomize_initial_states...");
  */ 
}
#endif
