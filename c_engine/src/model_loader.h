/** @file model_loader.h
 * Interface for model_loader.c.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Grid Computing Research Group<br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.1
 * @date March 2003
 *
 * Copyright &copy; University of Guelph, 2003-2006
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include "model.h"



/* Prototypes. */
int naadsm_load_models (const char *parameter_file,
                        HRD_herd_list_t *, projPJ, ZON_zone_list_t *,
                        unsigned int *ndays, unsigned int *nruns,
                        naadsm_model_t *** models, GPtrArray * outputs, guint *_exit_conditions);
void naadsm_unload_models (int nmodels, naadsm_model_t ** models);

#endif /* !MODEL_LOADER_H */
