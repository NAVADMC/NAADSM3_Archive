%{
#if HAVE_CONFIG_H
#  include <config.h>
#endif

#if HAVE_STRINGS_H
#  include <strings.h>
#endif

#include "herd.h"
#include <assert.h>

/** @file sm3herds2xml/parser.c
 * A parser for SpreadModel 3.0 herd status snapshot files.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Grid Computing Research Group<br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.1
 * @date August 2004
 *
 * Copyright &copy; University of Guelph, 2004-2006
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/** @page fileformat SpreadModel 3.0 herd status snapshot file format
 * The first line is a header line.
 * It contains the following field names, separated by commas, in this order:
 * <ul>
 *   <li>\em ID
 *   <li>\em Production Type
 *   <li>\em HerdSize
 *   <li>\em Lat
 *   <li>\em Lon
 *   <li>\em Status
 *   <li>\em DinStatus or \em DaysinStatus
 *   <li>\em DLinStatus or \em DaysLeftinStatus
 *   <li>\em ExposedLHN1 or \em ExposedHN1
 *   <li>\em ExposedLHE1 or \em HowExposed1
 *   <li>\em ExposedLDE1 or \em ExposedDays1
 *     <br>
 *     The above 3 fields may be repeated with increasing numbers.
 *   <li>\em HowInfected
 *   <li>\em DSinceDet or \em DaysSinceDetected
 *   <li>\em DSinceVac or \em DaysSinceVaccinated
 *   <li>\em HoldFor or \em HoldingFor
 *   <li>\em HoldDays or \em HoldingDays
 *   <li>\em HoldPriority or \em HoldingPriority
 * </ul>
 * (The field names are not case sensitive.)
 *
 * The following lines are data.  They contain numbers or numeric codes,
 * comma-separated.
 *
 * \em Lat and \em Lon are decimal numbers.  That is, 100&deg;30'00"W would be
 * given as -100.5.
 */
#define YYERROR_VERBOSE
#define BUFFERSIZE 2048

/* int yydebug = 1; must also compile with --debug to use this */
int yylex(void);
int yyerror (char const *s);
HRD_herd_list_t * herds;
gboolean first_exposure_fields = TRUE;
int ntracked_exposures;
int nfields;
char errmsg[BUFFERSIZE];
%}

%union {
  int ival;
  float fval;
  HRD_herd_t *hval;
  GSList *lval;
}

%token ID PRODTYPE HERDSIZE LAT LON STATUS DINSTATUS DLINSTATUS
%token HOWINFECTED DSINCEDET DSINCEVAC HOLDFOR HOLDDAYS HOLDPRIORITY
%token COMMA NEWLINE
%token <ival> INT EXPOSEDHN HOWEXPOSED EXPOSEDDAYS
%token <fval> FLOAT
%type <ival> exposure_fields exposure_fields_list
%type <hval> herd
%type <lval> int_list
%%
herd_snapshot :
    header NEWLINE herd_list NEWLINE
    { }
  ;

header :
    ID COMMA PRODTYPE COMMA HERDSIZE COMMA LAT COMMA LON
    {
      ntracked_exposures = 0;
      nfields = 5;
    }
  | ID COMMA PRODTYPE COMMA HERDSIZE COMMA LAT COMMA LON COMMA STATUS
    {
      ntracked_exposures = 0;
      nfields = 6;
    }
  | ID COMMA PRODTYPE COMMA HERDSIZE COMMA LAT COMMA LON COMMA STATUS COMMA
    DINSTATUS COMMA DLINSTATUS COMMA exposure_fields_list COMMA HOWINFECTED COMMA
    DSINCEDET COMMA DSINCEVAC COMMA HOLDFOR COMMA HOLDDAYS COMMA HOLDPRIORITY
    {
      ntracked_exposures = $17;
      nfields = 8 + $17*3 + 6;
    }
  ;

exposure_fields_list:
    exposure_fields_list COMMA exposure_fields
    {
      /* How many exposures we track (just the last one, the last two, ...) is
       * arbitrary and we should be able to change it with a single #define.
       * So here in the parser we take a little care to see that the fields are
       * numbered in order starting at 1. */
      if ($1+1 != $3)
        yyerror ("Field names are out of order");
      $$ = $3;
    }
  | exposure_fields
    {
      if (first_exposure_fields && ($1 != 1))
        yyerror ("Field names are out of order");
      first_exposure_fields = FALSE;
      $$ = $1;
    }
  ;

exposure_fields :
    EXPOSEDHN COMMA HOWEXPOSED COMMA EXPOSEDDAYS
    {
      /* Exposure event fields come in threes (e.g., ExposedHN1, HowExposed1,
       * ExposedDays1). */
      if (!($1 == $3 && $3 == $5))
        yyerror ("Field names are out of order");
      $$ = $1;
    }
  ;

herd_list :
    herd_list NEWLINE herd
    {
      HRD_herd_list_append (herds, $3);
      /* Destroy the herd structure now that its contents have been copied. */
      HRD_free_herd ($3, FALSE);
    }
  | herd
    {
      /* Initialize an array of herd structures. */
      herds = HRD_new_herd_list ();
      assert (herds != NULL);
      HRD_herd_list_append (herds, $1);
      /* Destroy the herd structure now that its contents have been copied. */
      HRD_free_herd ($1, FALSE);
    }
  ;
    
herd:
    INT COMMA INT COMMA INT COMMA FLOAT COMMA FLOAT
    {
      GPtrArray *production_type_names;
      GString *tmp;
      int i;

      /* Use the first few values (the fixed-length part of the line) to
       * initialize the herd structure. */
      tmp = g_string_new (NULL);
      g_string_printf (tmp, "%i", $3);
      printf ("production type is \"%s\"\n", tmp->str);
      production_type_names = herds->production_type_names;
      for (i = 0; i < production_type_names->len; i++)
	{
	  if (strcasecmp (tmp->str, g_ptr_array_index (production_type_names, i)) == 0)
            break;
	}
      if (i == production_type_names->len)
	{
	  /* We haven't encountered this production type before; add its name to
	   * the list. */
	  g_ptr_array_add (production_type_names, tmp->str);
	  g_string_free (tmp, FALSE);
	}
      else
        g_string_free (tmp, TRUE);

      $$ = HRD_new_herd (i, g_ptr_array_index (production_type_names, i), $5, $7, $9);
      assert ($$ != NULL);
      $$->status = Susceptible;
    }
  | INT COMMA INT COMMA INT COMMA FLOAT COMMA FLOAT COMMA INT
    {
      GPtrArray *production_type_names;
      GString *tmp;
      int i;

      /* Use the first few values (the fixed-length part of the line) to
       * initialize the herd structure. */
      tmp = g_string_new (NULL);
      g_string_printf (tmp, "%i", $3);
      production_type_names = herds->production_type_names;
      for (i = 0; i < production_type_names->len; i++)
	{
	  if (strcasecmp (tmp->str, g_ptr_array_index (production_type_names, i)) == 0)
            break;
	}
      if (i == production_type_names->len)
	{
	  /* We haven't encountered this production type before; add its name to
	   * the list. */
	  g_ptr_array_add (production_type_names, tmp->str);
	  g_string_free (tmp, FALSE);
	}
      else
        g_string_free (tmp, TRUE);

      $$ = HRD_new_herd (i, g_ptr_array_index (production_type_names, i), $5, $7, $9);
      assert ($$ != NULL);
      $$->status = $11;
    }
  | INT COMMA INT COMMA INT COMMA FLOAT COMMA FLOAT COMMA INT COMMA INT COMMA INT COMMA int_list
    {
      GPtrArray *production_type_names;
      GString *tmp;
      int length;
      GSList *iter; /* iterator */
      int i; /* loop counter */
      
      /* Use the first few values (the fixed-length part of the line) to
       * initialize the herd structure. */
      tmp = g_string_new (NULL);
      g_string_printf (tmp, "%i", $3);
      production_type_names = herds->production_type_names;
      for (i = 0; i < production_type_names->len; i++)
	{
	  if (strcasecmp (tmp->str, g_ptr_array_index (production_type_names, i)) == 0)
            break;
	}
      if (i == production_type_names->len)
	{
	  /* We haven't encountered this production type before; add its name to
	   * the list. */
	  g_ptr_array_add (production_type_names, tmp->str);
	  g_string_free (tmp, FALSE);
	}
      else
        g_string_free (tmp, TRUE);

      $$ = HRD_new_herd (i, g_ptr_array_index (production_type_names, i), $5, $7, $9);
      assert ($$ != NULL);
      $$->status = $7;

      /* Copy the values from the variable-length part of the line into the
       * herd structure. */
      length = 6 + g_slist_length ($17);
      if (length < nfields)
        {
          g_snprintf (errmsg, BUFFERSIZE, "Too few fields (found %i, require %i)",
	    length, nfields);
          yyerror (errmsg);
	}
      else if (length > nfields)
        {
          g_snprintf (errmsg, BUFFERSIZE, "Too many fields (found %i, require %i)",
	    length, nfields);
          yyerror (errmsg);
	}
      iter = $17;
      for (i = 0; i < ntracked_exposures; i++)
	{
	  int exposing_herd;
	  unsigned short int day;

	  exposing_herd = GPOINTER_TO_INT (iter->data);
	  iter = g_slist_next (iter);
	  /* skip how_exposed field */
	  iter = g_slist_next (iter);
	  day = GPOINTER_TO_INT (iter->data);
	  iter = g_slist_next (iter);
	}
      /* skip HowInfected field */
      iter = g_slist_next (iter);
      /* skip DaysSinceDetected field */
      iter = g_slist_next (iter);
      /* skip DaysSinceVaccinated field */
      iter = g_slist_next (iter);
      /* skip HoldingFor field */
      iter = g_slist_next (iter);
      /* skip HoldingDays field */
      iter = g_slist_next (iter);
      /* skip HoldingPriority field */

      /* Destroy the linked list of integers now that its contents have been
       * copied. */
      g_slist_free ($17);
    }
  ;

int_list:
    int_list COMMA INT
    {
      $$ = g_slist_append ($1, GINT_TO_POINTER ($3));
    }
  | INT
    {
      /* Initialize a linked list of integers. */
      $$ = g_slist_append (NULL, GINT_TO_POINTER ($1));
    }
  ;

%%
extern int yylineno, tokenpos;
extern char linebuf[];

/* Simple yyerror from _lex & yacc_ by Levine, Mason & Brown. */
int
yyerror (char const *s)
{
  fprintf (stderr, "Error in herd snapshot file (line %d): %s:\n%s\n", yylineno, s, linebuf);
  fprintf (stderr, "%*s\n", 1+tokenpos, "^");
  return 0;
}
