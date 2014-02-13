/** @file test-monitor.h
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Grid Computing Research Group<br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @date July 2009
 *
 * Copyright &copy; University of Guelph, 2009
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef TEST_MONITOR_H
#define TEST_MONITOR_H

naadsm_model_t *test_monitor_new (scew_element * params, HRD_herd_list_t *,
                                  projPJ, ZON_zone_list_t *);

#endif
