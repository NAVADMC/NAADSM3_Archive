/** @file reporting.h
 * Output variables, and how they are to be reported.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.2
 * @date February 2004
 *
 * Copyright &copy; University of Guelph, 2004-2009
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef REPORTING_H
#define REPORTING_H

#include <stdio.h>

#include <glib.h>



/**
 * Number of types of output variables.
 *
 * @sa RPT_type_t
 */
#define RPT_NTYPES 5

/**
 * Types of output variables.
 */
typedef enum
{
  RPT_integer, RPT_real, RPT_text, RPT_group, RPT_unknown_type
}
RPT_type_t;
extern const char *RPT_type_name[];



/**
 * Number of reporting frequencies.
 *
 * @sa RPT_frequency_t
 */
#define RPT_NFREQUENCIES 3

/**
 * Reporting frequencies.
 */
typedef enum
{
  RPT_never, RPT_once, RPT_daily
}
RPT_frequency_t;
extern const char *RPT_frequency_name[];



/**
 * An output variable.
 */
typedef struct
{
  char *name; /**< The variable's name.  Should not contain commas, single
    quotes ('), double quotes ("), newlines, or carriage returns. */
  RPT_type_t type; /**< The type of variable.  For variables with categories,
    this will be RPT_group; use RPT_reporting_get_type() to find the base type
    (RPT_integer, RPT_real, or RPT_text) of the variable. */
  RPT_frequency_t frequency; /**< How frequently the variable is reported. */
  int days;
  gboolean is_null; /**< If TRUE, this variable has no meaningful value.  Used
    in cases like the day of first detection, where there is no meaningful
    numeric value until detection occurs, or R0, where there is no meaningful
    value until 2 incubation periods have passed. */
  void *data;
}
RPT_reporting_t;



/* Prototypes. */

RPT_reporting_t *RPT_new_reporting (const char *name, RPT_type_t, RPT_frequency_t);
void RPT_free_reporting (RPT_reporting_t *);
char *RPT_reporting_to_string (RPT_reporting_t *);
char *RPT_reporting_value_to_string (RPT_reporting_t *, char *format);
unsigned int RPT_reporting_var_count (RPT_reporting_t *);
GPtrArray *RPT_reporting_names (RPT_reporting_t *);
GPtrArray *RPT_reporting_values_as_strings (RPT_reporting_t *);
int RPT_fprintf_reporting (FILE *, RPT_reporting_t *);

#define RPT_printf_reporting(R) RPT_fprintf_reporting(stdout,R)

void RPT_reporting_set_frequency (RPT_reporting_t *, RPT_frequency_t);

void RPT_reporting_set_integer (RPT_reporting_t *, long, const char **);
void RPT_reporting_set_integer1 (RPT_reporting_t *, long, const char *);
void RPT_reporting_set_real (RPT_reporting_t *, double, const char **);
void RPT_reporting_set_real1 (RPT_reporting_t *, double, const char *);
void RPT_reporting_set_text (RPT_reporting_t *, char *text, const char **);
void RPT_reporting_set_text1 (RPT_reporting_t *, char *text, const char *);
void RPT_reporting_set_null (RPT_reporting_t *, const char **);
void RPT_reporting_set_null1 (RPT_reporting_t *, const char *);
void RPT_reporting_add_integer (RPT_reporting_t *, long, const char **);
void RPT_reporting_add_integer1 (RPT_reporting_t *, long, const char *);
void RPT_reporting_add_real (RPT_reporting_t *, double, const char **);
void RPT_reporting_add_real1 (RPT_reporting_t *, double, const char *);
#define RPT_reporting_sub_integer(R,I,C) RPT_reporting_add_integer(R,-(I),C)
#define RPT_reporting_sub_integer1(R,I,C) RPT_reporting_add_integer1(R,-(I),C)
#define RPT_reporting_sub_real(R,I,C) RPT_reporting_add_real(R,-(I),C)
#define RPT_reporting_sub_real1(R,I,C) RPT_reporting_add_real1(R,-(I),C)
void RPT_reporting_append_text (RPT_reporting_t *, char *text, const char **);
void RPT_reporting_append_text1 (RPT_reporting_t *, char *text, const char *);
void RPT_reporting_splice (RPT_reporting_t *, RPT_reporting_t *);
void RPT_reporting_reset (RPT_reporting_t *);
void RPT_reporting_zero (RPT_reporting_t *);
gboolean RPT_reporting_is_null (RPT_reporting_t *, const char**);
gboolean RPT_reporting_is_null1 (RPT_reporting_t *, const char*);
long RPT_reporting_get_integer (RPT_reporting_t *, const char **);
long RPT_reporting_get_integer1 (RPT_reporting_t *, const char *);
double RPT_reporting_get_real (RPT_reporting_t *, const char **);
double RPT_reporting_get_real1 (RPT_reporting_t *, const char *);
char *RPT_reporting_get_text (RPT_reporting_t *, const char **);
char *RPT_reporting_get_text1 (RPT_reporting_t *, const char *);

RPT_frequency_t RPT_string_to_frequency (const char *);
gboolean RPT_reporting_due (RPT_reporting_t *, unsigned int day);
RPT_type_t RPT_reporting_get_type (RPT_reporting_t *);
RPT_reporting_t *RPT_clone_reporting (RPT_reporting_t *);

#endif /* !REPORTING_H */
