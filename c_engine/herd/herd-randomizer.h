/** @file herd-randomizer.h
 *
 * @author Aaron Reeves <Aaron.Reeves@colostate.edu><br>
 *   Animal Population Health Institute<br>
 *   College of Veterinary Medicine and Biomedical Sciences<br>
 *   Colorado State University<br>
 *   Fort Collins, CO 80523<br>
 *   USA
 * @version 0.1
 * @date March 26, 2011
 *
 * Copyright &copy; 2011 Animal Population Health Institute, Colorado State University
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef HERD_RANDOMIZER_H
#define HERD_RANDOMIZER_H

#include "herd.h"
#include "rng.h"

void randomize_initial_states( HRD_herd_list_t *herds, RAN_gen_t *rng );

#endif /* HERD_RANDOMIZER_H */
