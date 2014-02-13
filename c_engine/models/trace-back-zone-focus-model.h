/** @file trace-back-zone-focus-model.h
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Grid Computing Research Group<br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.1
 * @date June 2006
 *
 * Copyright &copy; University of Guelph, 2006
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/*
 * NOTE: This module is DEPRECATED, and is included only for purposes of backward
 * compatibility with parameter files from NAADSM 3.0 - 3.1.x.  Any new 
 * development should be done elsewhere: see trace-zone-focus-model.
*/

#ifndef TRACE_BACK_ZONE_FOCUS_MODEL_H
#define TRACE_BACK_ZONE_FOCUS_MODEL_H

naadsm_model_t *trace_back_zone_focus_model_new (scew_element * params, HRD_herd_list_t *,
                                                 projPJ, ZON_zone_list_t *);

#endif
