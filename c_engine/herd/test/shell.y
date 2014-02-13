%{
#if HAVE_CONFIG_H
#  include <config.h>
#endif

#if HAVE_STRINGS_H
#  include <strings.h>
#endif

#include <herd.h>
#include <glib.h>
#include <math.h>

#define PROMPT "> "

/** @file herd/test/shell.c
 * A simple shell to exercise libherd.  It provides a way to create herds and
 * call the functions offered by the <a href="herd_8h.html">library</a>, so
 * that a suite of tests can be scripted.
 *
 * The commands are:
 * <ul>
 *   <li>
 *     <code>herd (type,size,lat,long)</code>
 *
 *     Adds a herd to the test set.  The first argument may be any string,
 *     e.g., "Beef Cattle", "Swine".  The herds are numbered starting from 0.
 *     They start as Susceptible.
 *   <li>
 *     <code>infect (herd,latent,infectious_subclinical,infectious_clinical,immunity)</code>
 *
 *     Starts the progression of the disease in the given herd.  The next four
 *     arguments (all integers) give the number of days the herd spends in each
 *     state.
 *   <li>
 *     <code>vaccinate (herd,delay,immunity)</code>
 *
 *     Vaccinates the given herd.  The argument <i>delay</i> gives the number
 *     of days the vaccine requires to take effect.  The animals are immune to
 *     the disease for the number of days given by the argument
 *     <i>immunity</i>.
 *   <li>
 *     <code>destroy (herd)</code>
 *
 *     Destroys the given herd.
 *   <li>
 *     <code>step</code>
 *
 *     Step forward one day.  The output of this command is a space-separated
 *     list of the herd statuses.
 *   <li>
 *     <code>reset</code>
 *
 *     Erase all currently entered herds.
 * </ul>
 *
 * The shell exits on EOF (Ctrl+D if you're typing commands into it
 * interactively in Unix).
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Grid Computing Research Group<br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.1
 * @date July 2003
 *
 * Copyright &copy; University of Guelph, 2003-2006
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

HRD_herd_list_t *current_herds = NULL;
GPtrArray *production_type_names = NULL;
GHashTable *dummy; /* The HRD_step function, which advances a herd's state, has
  as an argument a hash table of infectious herds, which is updated at the same
  time as the state change.  In this small test program, we don't need to do
  that, but we still need a hash table to pass to HRD_step. */


void g_free_as_GFunc (gpointer data, gpointer user_data);
%}

%union {
  int ival;
  double fval;
  char *sval;
  GSList *lval;
}

%token HERD INFECT VACCINATE DESTROY STEP RESET
%token INT FLOAT STRING
%token LPAREN RPAREN COMMA
%token <ival> INT
%token <fval> REAL
%token <sval> STRING
%type <fval> real
%%

commands :
    command commands
  | command
  ;

command :
    new_command
  | infect_command
  | vaccinate_command
  | destroy_command
  | step_command
  | reset_command
  ;

new_command :
    HERD LPAREN STRING COMMA INT COMMA real COMMA real RPAREN
    {
      HRD_herd_t *herd;
      int i;
      
      /* Find the production type name in the list of names encountered so far.
       * If it's not there, add it. */
      for (i = 0; i < production_type_names->len; i++)
	if (strcasecmp ($3, g_ptr_array_index (production_type_names, i)) == 0)
	  break;
      if (i == production_type_names->len)
	g_ptr_array_add (production_type_names, $3);
      else
	free ($3);

      herd = HRD_new_herd (i, g_ptr_array_index (production_type_names, i), $5, $7, $9);
      HRD_herd_list_append (current_herds, herd);
      
      HRD_printf_herd (herd);
      free (herd);
      printf ("\n%s", PROMPT);
      fflush (stdout);
    }
  ;

infect_command:
    INFECT LPAREN INT COMMA INT COMMA INT COMMA INT COMMA INT RPAREN
    {
      g_assert (0 <= $3 && $3 < HRD_herd_list_length (current_herds));
      HRD_infect (HRD_herd_list_get (current_herds, $3), $5, $7, $9, $11, 0);
      printf ("%s", PROMPT);
      fflush (stdout);
    }
  ;

vaccinate_command:
    VACCINATE LPAREN INT COMMA INT COMMA INT RPAREN
    {
      g_assert (0 <= $3 && $3 < HRD_herd_list_length (current_herds));
      HRD_vaccinate (HRD_herd_list_get (current_herds, $3), $5, $7);
      printf ("%s", PROMPT);
      fflush (stdout);
    }
  ;

destroy_command:
    DESTROY LPAREN INT RPAREN
    {
      g_assert (0 <= $3 && $3 < HRD_herd_list_length (current_herds));
      HRD_destroy (HRD_herd_list_get (current_herds, $3));
      printf ("%s", PROMPT);
      fflush (stdout);
    }
  ;

step_command :
    STEP
    {
      unsigned int nherds;
      int i;
      
      nherds = HRD_herd_list_length (current_herds);
      g_assert (nherds > 0);
      
      for (i = 0; i < nherds; i++)
	HRD_step (HRD_herd_list_get (current_herds, i), dummy);

      printf ("%s", HRD_status_name[HRD_herd_list_get (current_herds, 0)->status]);
      for (i = 1; i < nherds; i++)
        printf (" %s",  HRD_status_name[HRD_herd_list_get (current_herds, i)->status]);
      printf ("\n%s", PROMPT);
      fflush (stdout);
    }
  ;

reset_command :
    RESET
    {
      HRD_free_herd_list (current_herds);
      current_herds = HRD_new_herd_list ();
      printf ("%s", PROMPT);
      fflush (stdout);
    }
  ;

real:
    INT
    {
      $$ = (double) $1; 
    }
  | REAL
    {
      $$ = $1;
    }
  ;

%%

extern FILE *yyin;
extern int tokenpos;
extern char linebuf[];

/* Simple yyerror from _lex & yacc_ by Levine, Mason & Brown. */
int
yyerror (char const *s)
{
  g_error ("%s\n%s\n%*s", s, linebuf, 1+tokenpos, "^");
  return 0;
}



/**
 * Wraps free so that it can be used in GLib calls.
 *
 * @param data a pointer cast to a gpointer.
 * @param user_data not used, pass NULL.
 */
void
g_free_as_GFunc (gpointer data, gpointer user_data)
{
  g_free (data);
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
  g_log_set_handler ("herd", G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG, silent_log_handler, NULL);

  current_herds = HRD_new_herd_list ();
  production_type_names = g_ptr_array_new ();
  dummy = g_hash_table_new (g_direct_hash, g_direct_equal);

  printf (PROMPT);
  if (yyin == NULL)
    yyin = stdin;
  while (!feof(yyin))
    yyparse();

  return EXIT_SUCCESS;
}

/* end of file shell.y */
