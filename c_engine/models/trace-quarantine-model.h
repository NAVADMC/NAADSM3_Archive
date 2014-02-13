/** @file trace-quarantine-model.h
 *
 * @author Aaron Reeves <Aaron.Reeves@colostate.edu><br>
 *   Animal Population Health Institute<br>
 *   College of Veterinary Medicine and Biomedical Sciences<br>
 *   Colorado State University<br>
 *   Fort Collins, CO 80523<br>
 *   USA
 * @version 0.1
 * @date August 2007
 *
 * Copyright &copy; 2007 Animal Population Health Institute, Colorado State University
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */



#ifndef TRACE_QUARANTINE_MODEL_H
#define TRACE_QUARANTINE_MODEL_H

naadsm_model_t *trace_quarantine_model_new (scew_element * params, HRD_herd_list_t *,
                                            projPJ, ZON_zone_list_t *);

#endif
