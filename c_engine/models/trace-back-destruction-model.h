/** @file trace-back-destruction-model.h
 *
 * @author Aaron Reeves <Aaron.Reeves@colostate.edu><br>
 *   Animal Population Health Institute<br>
 *   College of Veterinary Medicine and Biomedical Sciences<br>
 *   Colorado State University<br>
 *   Fort Collins, CO 80523<br>
 *   USA
 * @version 0.1
 * @date June 2005
 *
 * Copyright &copy; 2005 - 2009 Animal Population Health Institute, Colorado State University
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */
 

/*
 * NOTE: This module is DEPRECATED, and is included only for purposes of backward
 * compatibility with parameter files from NAADSM 3.0 - 3.1.x.  Any new 
 * development should be done elsewhere: see trace-model and contact-recorder.
*/
 
#ifndef TRACE_BACK_DESTRUCTION_MODEL_H
#define TRACE_BACK_DESTRUCTION_MODEL_H

naadsm_model_t *trace_back_destruction_model_new (scew_element * params, HRD_herd_list_t *,
                                                  projPJ, ZON_zone_list_t *);

#endif
