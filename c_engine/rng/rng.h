/** @file rng.h
 * Random numbers.
 *
 * Symbols from this module begin with RAN_.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Grid Computing Research Group<br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.1
 * @date October 2005
 *
 * Copyright &copy; University of Guelph, 2005-2008
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */
#ifndef RNG_H
#define RNG_H


#if defined(DLL_EXPORTS)
#	define DLL_API __declspec( dllexport )
#elif defined(DLL_IMPORTS)
# define DLL_API __declspec( dllimport )
#else
# define DLL_API
#endif

#include <glib.h>
#include <gsl/gsl_rng.h>



/** A random number generator object. */
typedef struct
{
  gboolean fixed;
  double fixed_value;

  /* These fields included so that the random number generator can be used by
   * GNU Scientific Library functions. */
  gsl_rng_type as_gsl_rng_type;
  gsl_rng as_gsl_rng;
}
RAN_gen_t;



RAN_gen_t *RAN_new_generator (int seed);
double RAN_num (RAN_gen_t *);
gsl_rng *RAN_generator_as_gsl (RAN_gen_t *);
void RAN_fix (RAN_gen_t *, double);
void RAN_unfix (RAN_gen_t *);
void RAN_free_generator (RAN_gen_t *);


/* Function pointer types */
/*------------------------*/
typedef void (*TRngVoid_1_Int) (int);

/* FIX ME: Someone should write this comment... */
TRngVoid_1_Int rng_read_seed;

DLL_API void set_rng_read_seed (TRngVoid_1_Int fn);

/* Function pointer helpers */
/*--------------------------*/
void clear_rng_fns (void);


#endif /* !RNG_H */
