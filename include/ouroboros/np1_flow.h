/*
 * Ouroboros - Copyright (C) 2016 - 2018
 *
 * Adapter functions for N + 1 flow descriptors
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

#ifndef OUROBOROS_NP1_FLOW_H
#define OUROBOROS_NP1_FLOW_H

#include <ouroboros/qoscube.h>

#include <unistd.h>

int  np1_flow_alloc(pid_t     n_pid,
                    int       port_id,
                    qosspec_t qs);

int  np1_flow_resp(int port_id);

int  np1_flow_dealloc(int port_id);

#endif /* OUROBOROS_NP1_FLOW_H */
