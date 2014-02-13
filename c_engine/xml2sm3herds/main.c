/** @file xml2sm3herds/main.c
 * A utility that converts XML herd status files to SpreadModel 3 format.  Send
 * the XML file to standard input; the SpreadModel file is written to standard
 * output.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Grid Computing Research Group<br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.1
 * @date March 2004
 *
 * Copyright &copy; University of Guelph, 2004-2008
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "herd.h"



int
main (int argc, char *argv[])
{
  HRD_herd_list_t *herds;
  unsigned int nherds, i;
  HRD_herd_t *herd;
  int days_left;

  g_print ("ID,ProductionType,HerdSize,Lat,Lon,Status,DaysLeftInStatus\n");
#ifdef USE_SC_GUILIB
  herds = HRD_load_herd_list_from_stream (NULL, NULL, NULL);
#else
  herds = HRD_load_herd_list_from_stream (NULL, NULL);
#endif
  nherds = HRD_herd_list_length (herds);
  for (i = 0; i < nherds; i++)
    {
      herd = HRD_herd_list_get (herds, i);
      days_left = herd->days_left_in_initial_status;
      if (days_left < 1)
        days_left = -1;
      g_print ("%s,%s,%u,%g,%g,%i,%i\n",
               herd->official_id, herd->production_type_name, herd->size,
               herd->latitude, herd->longitude, herd->status, days_left);
    }

  HRD_free_herd_list (herds);

  return EXIT_SUCCESS;
}

/* end of file main.c */
