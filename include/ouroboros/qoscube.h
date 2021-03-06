/*
 * Ouroboros - Copyright (C) 2016 - 2018
 *
 * Quality of Service cubes
 *
 *    Dimitri Staessens <dimitri.staessens@ugent.be>
 *    Sander Vrijders   <sander.vrijders@ugent.be>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., http://www.fsf.org/about/contact/.
 */

#ifndef OUROBOROS_QOSCUBE_H
#define OUROBOROS_QOSCUBE_H

#include <ouroboros/qos.h>

typedef enum qos_cube {
        QOS_CUBE_BE = 0,
        QOS_CUBE_VIDEO,
        QOS_CUBE_VOICE,
        QOS_CUBE_MAX
} qoscube_t;

qoscube_t qos_spec_to_cube(qosspec_t qs);
qosspec_t qos_cube_to_spec(qoscube_t qc);

#endif
