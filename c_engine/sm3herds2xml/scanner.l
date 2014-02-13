%{
#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "herd.h"
#include "parser.h"

#if STDC_HEADERS
#  include <string.h>
#elif HAVE_STRINGS_H
#  include <strings.h>
#endif

/** @file sm3herds2xml/scanner.c
 * A scanner for SpreadModel 3.0 herd status snapshot files.
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
 *
 * @todo Check for overflow in string-to-int and string-to-float conversions.
 *
 */
#define TABWIDTH 8
#define BUFFERSIZE 2048
#define ADJUST (tokenpos = charpos, charpos += yyleng)

int yyerror (char *s, int fatal);

char linebuf[BUFFERSIZE];
int charpos = 0, tokenpos = 0;
%}
%option yylineno
%option case-insensitive
%x PASTFIRSTLINE
%%
.*	{
  /* Copy a line of input to a buffer, then "put it back" for processing by
   * lex.  This allows us to print pretty error messages showing the entire
   * current line of input. */
  strncpy (linebuf, yytext, BUFFERSIZE);
  /* If the input line didn't fit in the buffer, null-terminate it. */
  linebuf[BUFFERSIZE-1] = '\0';
  yyless(0);
  BEGIN(PASTFIRSTLINE);
}
<PASTFIRSTLINE>{
  \n.* {
    /* Copy a line of input to a buffer (see comment above). */
    strncpy (linebuf, yytext+1, BUFFERSIZE);
    /* If the input line didn't fit in the buffer, null-terminate it. */
    linebuf[BUFFERSIZE-1] = '\0';
    yyless(1);
    charpos = 0;
    tokenpos = 0;
    return NEWLINE;
  }
  \t {
    tokenpos = charpos;
    charpos += TABWIDTH;
  }
  ,                         { ADJUST; return COMMA; }
  "ID"                      { ADJUST; return ID; }
  Prod(uction)?" "?Type     { ADJUST; return PRODTYPE; }
  Herd" "?Size              { ADJUST; return HERDSIZE; }
  Lat(itude)?               { ADJUST; return LAT; }
  Lon(gitude)?              { ADJUST; return LON; }
  "Status"                  { ADJUST; return STATUS; }
  D(ays)?inStatus           { ADJUST; return DINSTATUS; }
  D(ays)?L(eft)?inStatus    { ADJUST; return DLINSTATUS; }
  ExposedHN([[:digit:]]+) {
    ADJUST;
    yylval.ival = atoi(&yytext[9]); /* just convert the numeric part */
    return EXPOSEDHN;
  }
  ExposedLHN([[:digit:]]+) { /* Another version of ExposedHN. */
    ADJUST;
    yylval.ival = atoi(&yytext[10]);
    return EXPOSEDHN;
  }
  HowExposed([[:digit:]]+) {
    ADJUST;
    yylval.ival = atoi(&yytext[10]);
    return HOWEXPOSED;
  }  
  ExposedLHE([[:digit:]]+) { /* Another version of HowExposed. */
    ADJUST;
    yylval.ival = atoi(&yytext[10]);
    return HOWEXPOSED;
  }
  ExposedDays([[:digit:]]+) {
    ADJUST;
    yylval.ival = atoi(&yytext[11]);
    return EXPOSEDDAYS;
  }
  ExposedLDE([[:digit:]]+) { /* Another version of ExposedDays. */
    ADJUST;
    yylval.ival = atoi(&yytext[10]);
    return EXPOSEDDAYS;
  }
  "HowInfected"             { ADJUST; return HOWINFECTED; }
  D(ays)?SinceDet(ected)?   { ADJUST; return DSINCEDET; }
  D(ays)?SinceVac(cinated)? { ADJUST; return DSINCEVAC; }
  Hold(ing)?For             { ADJUST; return HOLDFOR; }
  Hold(ing)?Days            { ADJUST; return HOLDDAYS; }
  Hold(ing)?Priority        { ADJUST; return HOLDPRIORITY; }
  [[:digit:]]+ {
    ADJUST;
    yylval.ival = atoi(yytext);
    return INT;
  }
  [+-]?[[:digit:]]+\.[[:digit:]]+ {
    ADJUST;
    yylval.fval = (float)atof(yytext);
    return FLOAT;
  }
  \n {
    charpos = 0;
    tokenpos = 0;
    return NEWLINE;
  }
  [:space:] { /* Ignore whitespace */ }
  . {
    /* Report an unrecognized token. */
    yyerror ("Unrecognized token (could the field names be different than what we're expecting?)", 1);
  }
}
%%
