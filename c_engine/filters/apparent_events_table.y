%{
#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "herd.h"
#include "event.h"
#include "reporting.h"
#include <stdio.h>

#if STDC_HEADERS
#  include <stdlib.h>
#endif

#if HAVE_STRING_H
#  include <string.h>
#endif

/* #define DEBUG 1 */

/** @file filters/apparent_events_table.c
 * A filter that turns SHARCSpread output into a table of detections,
 * vaccinations and destructions.
 *
 * Call it as
 *
 * <code>apparent_events_table_filter HERD-FILE < LOG-FILE</code>
 *
 * The apparent events table is written to standard output in comma-separated
 * values format.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 *
 * Copyright &copy; University of Guelph, 2005-2011
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#define YYERROR_VERBOSE
#define BUFFERSIZE 2048

/* int yydebug = 1; must also compile with --debug to use this */
int yylex(void);
int yyerror (char const *s);
char errmsg[BUFFERSIZE];

HRD_herd_list_t *herds;
int current_node; /**< The most recent node number we have seen in the
  simulator output. */
int current_run; /**< The most recent run number we have seen in the output. */
int current_day; /**< The most recent day we have seen in the output. */
int run; /**< A run identifier that increases each time we print one complete
  Monte Carlo run.  This is distinct from the per-node run numbers in the
  simulator output. */



/**
 * Prints the values stored in an RPT_reporting_t structure.  This function is
 * typed as a GDataForeachFunc so that it can easily be called recursively on
 * the sub-variables.
 *
 * @param key_id use 0.
 * @param data an output variable, cast to a gpointer.
 * @param user_data the output variable's name as seen so far.  Used when
 *   recursively drilling down into sub-variables.  Use NULL for the top-level
 *   call.
 */
void
print_value (GQuark key_id, gpointer data, gpointer user_data)
{
  RPT_reporting_t *reporting;
  EVT_event_type_t event_type;
  char *reason;
  char *s; /* For building temporary strings. */
  gchar **tokens, **iter;
  unsigned int herd_index;
  HRD_herd_t *herd;

  reporting = (RPT_reporting_t *) data;

  /* The reporting variable is one level deep; the categories are the reasons
   * for detection/vaccination/destruction. */

  /* If this is a "detections", "exams", "vaccinations", or "destructions"
   * variable, recurse into its sub-categories. */
  if (user_data == NULL) /* will be false when in a sub-category */
    {
      if (strcmp (reporting->name, "detections") == 0
	  || strcmp (reporting->name, "exams") == 0
	  || strcmp (reporting->name, "vaccinations") == 0
	  || strcmp (reporting->name, "destructions") == 0)
	{
#if DEBUG
	  s = RPT_reporting_value_to_string (reporting, NULL);
          g_debug ("found %s variable, value = %s", reporting->name, s);
	  free (s);
#endif
          if (reporting->type == RPT_group)
	    {
	      g_datalist_foreach ((GData **) (&reporting->data), print_value, reporting->name);
	      goto end;
	    }
	}
      else
        {
	  goto end;
        }
    }

  /* If this is a sub-category of a "detections", "exams", "vaccinations", or
   * "destructions" variable, we want to print the individual events.  We get
   * the individual units from the variable's value, which is a comma-separated
   * list of unit indices. */
  s = RPT_reporting_get_text (reporting, NULL);
#if DEBUG
  g_debug ("s = \"%s\"", s);
#endif
  if (strlen (s) == 0)
    goto end;

  if (strcmp (user_data ? (char *)user_data : reporting->name, "detections") == 0)
    {
      event_type = EVT_Detection;
      reason = reporting->name;
    }
  else if (strcmp (user_data ? (char *)user_data : reporting->name, "exams") == 0)
    {
      event_type = EVT_Exam;
      reason = NULL;
    }
  else if (strcmp (user_data ? (char *)user_data : reporting->name, "vaccinations") == 0)
    {
      event_type = EVT_Vaccination;
      reason = reporting->name;
    }
  else if (strcmp (user_data ? (char *)user_data : reporting->name, "destructions") == 0)
    {
      event_type = EVT_Destruction;
      reason = reporting->name;
    }

  /* Split the text on commas. */
  tokens = g_strsplit (s, ",", 0);
  for (iter = tokens; *iter != NULL; iter++)
    {
      g_print ("%i,%i,%s,%s,", run, current_day,
               event_type == EVT_Detection ? "Detection"
                 : (event_type == EVT_Exam ? "Exam"
                     : (event_type == EVT_Vaccination ? "Vaccination" : "Destruction")),
               reason ? reason : "");
      /* This chunk of text will be a herd index. */
      herd_index = strtol (*iter, NULL, 10);
      herd = HRD_herd_list_get (herds, herd_index);
      g_print ("%s,%s,%u,%g,%g\n",
               herd->official_id,
               herd->production_type_name,
               herd->size,
               herd->latitude,
               herd->longitude);
    }
  g_strfreev (tokens);

end:
  return;
}

%}

%union {
  int ival;
  float fval;
  char *sval;
  RPT_reporting_t *rval;
  GSList *lval;
}

%token NODE RUN DAY POLYGON
%token COMMA COLON EQ LBRACE RBRACE LPAREN RPAREN DQUOTE NEWLINE
%token <ival> INT
%token <fval> FLOAT
%token <sval> VARNAME STRING
%type <rval> value subvar
%type <lval> subvars
%%
output_lines :
    output_lines output_line
    { }
  | output_line
    { }
  ;

output_line:
    tracking_line NEWLINE data_line NEWLINE
    { }
  ;

tracking_line:
    NODE INT RUN INT
    {
      int node = $2;
      int node_run = $4;

      /* When we see that the node number or run number has changed, we know
       * the output from one Monte Carlo trial is over, so we reset the day. */
      if (node != current_node || node_run != current_run)
        {
          current_day = 1;
          run++;
#if DEBUG
          g_debug ("now on day 1 (node %i, run %i)", node, node_run);
#endif
        }
      else
        {
          current_day++;
        }
      current_node = node;
      current_run = node_run;
    }
  ;

data_line:
    state_codes vars
    { }
  | state_codes
    { }
  | vars
    { }
  |
    { }
  ;

state_codes:
    state_codes INT
    { }
  | INT
    { }
  ;

vars:
    vars var
    { }
  | var
    { }
  ;       

var:
    VARNAME EQ value
    {
      if ($3 != NULL)
	{
	  $3->name = $1;

	  /* At this point we have built, in a bottom-up fashion, one complete
	   * output variable, stored in an RPT_reporting_t structure.  So we have a
	   * tree-style represention of
	   * 
	   * destructions={'reported diseased':'1','trace out':'2'}
	   *
	   * What we want for the summary table is something like:
	   *
	   * run 1,day 1,reported diseased,1
	   * run 1,day 1,trace out,2
	   *
	   * So we call a function that drills down into the RPT_reporting_t
	   * structure and prints whatever sub-variables it finds.
	   */
	  print_value (0, $3, NULL);
	  RPT_free_reporting ($3);
	}
      else
        g_free ($1);
    }
  ;

value:
    INT
    {
      $$ = NULL;
    }
  | FLOAT
    {
      $$ = NULL;
    }
  | STRING
    {
      $$ = RPT_new_reporting (NULL, RPT_text, RPT_never);
      RPT_reporting_set_text ($$, $1, NULL);
      /* The string token's value is set with a g_strndup, so we need to free
       * it after copying it into the RPT_reporting structure. */
      g_free ($1);
    }
  | POLYGON LPAREN RPAREN
    {
      $$ = NULL;
    }
  | POLYGON LPAREN contours RPAREN
    {
      $$ = NULL;
    }
  | LBRACE RBRACE
    {
      $$ = NULL;
    }
  | LBRACE subvars RBRACE
    {
      GSList *iter;

      $$ = RPT_new_reporting (NULL, RPT_group, RPT_never);
      for (iter = $2; iter != NULL; iter = g_slist_next (iter))
	RPT_reporting_splice ($$, (RPT_reporting_t *) (iter->data));

      /* Now that the sub-variables have been attached to a newly-created group
       * reporting variable, free the linked list structure that contained
       * them. */
      g_slist_free ($2);
    }
  ;

subvars:
    subvars COMMA subvar
    {
      $$ = g_slist_append ($1, $3);
    }
  | subvar
    {
      /* Initialize a linked list of subvars. */
      $$ = g_slist_append (NULL, $1);
    }
  ;

subvar:
    STRING COLON value
    {
      $$ = $3;
      if ($$ != NULL)
        $$->name = $1;
      else
        g_free ($1);
    }
  ;

/* The parser contains rules for parsing polygons, because they may appear in
 * the output, but we do nothing with them. */

contours:
    contours contour
    { }
  | contour
    { }
  ;

contour:
    LPAREN coords RPAREN
    { }
  ;

coords:
    coords COMMA coord
    { }
  | coord
    { }
  ;

coord:
    FLOAT FLOAT
    { }
  | FLOAT INT
    { }
  | INT FLOAT
    { }
  | INT INT
    { }
  ;

%%
extern FILE *yyin;
extern int yylineno, tokenpos;
extern char linebuf[];

/* Simple yyerror from _lex & yacc_ by Levine, Mason & Brown. */
int
yyerror (char const *s)
{
  fprintf (stderr, "Error in output (line %d): %s:\n%s\n", yylineno, s, linebuf);
  fprintf (stderr, "%*s\n", 1+tokenpos, "^");
  return 0;
}



/**
 * A log handler that simply discards messages.
 */
void
silent_log_handler (const gchar * log_domain, GLogLevelFlags log_level,
		    const gchar * message, gpointer user_data)
{
  ;
}



int
main (int argc, char *argv[])
{
  const char *herd_file = NULL;
  int verbosity = 0;
  unsigned int nherds;
  GError *option_error = NULL;
  GOptionContext *context;
  GOptionEntry options[] = {
    { "verbosity", 'V', 0, G_OPTION_ARG_INT, &verbosity, "Message verbosity level (0 = simulation output only, 1 = all debugging output)", NULL },
    { NULL }
  };

  /* Get the command-line options and arguments.  There should be one command-
   * line argument, the name of the herd file. */
  context = g_option_context_new ("");
  g_option_context_add_main_entries (context, options, /* translation = */ NULL);
  if (!g_option_context_parse (context, &argc, &argv, &option_error))
    {
      g_error ("option parsing failed: %s\n", option_error->message);
    }
  g_option_context_free (context);

  if (argc >= 2)
    herd_file = argv[1];
  else
    {
      g_error ("Need the name of a herd file.");
    }

  /* Set the verbosity level. */
  if (verbosity < 1)
    {
      g_log_set_handler (NULL, G_LOG_LEVEL_DEBUG, silent_log_handler, NULL);
      g_log_set_handler ("herd", G_LOG_LEVEL_DEBUG, silent_log_handler, NULL);
      g_log_set_handler ("reporting", G_LOG_LEVEL_DEBUG, silent_log_handler, NULL);
    }
#if DEBUG
  g_debug ("verbosity = %i", verbosity);
#endif

#ifdef USE_SC_GUILIB
  herds = HRD_load_herd_list (herd_file, NULL);
#else
  herds = HRD_load_herd_list (herd_file);
#endif
  nherds = HRD_herd_list_length (herds);

#if DEBUG
  g_debug ("%i units read", nherds);
#endif
  if (nherds == 0)
    {
      g_error ("no units in file %s", herd_file);
    }

  /* Print the table header. */
  printf ("Run,Day,Type,Reason,ID,Production type,Size,Lat,Lon\n");

  run = 0;
  /* Initialize the current node to "-1" so that whatever node appears first in
   * the simulator output will signal the parser that output from a new node is
   * beginning, that the current day needs to be reset, etc. */
  current_node = -1;

  yyin = stdin;
  while (!feof(yyin))
    yyparse();

  /* Clean up. */
  HRD_free_herd_list (herds);

  return EXIT_SUCCESS;
}

/* end of file apparent_events_table.y */
