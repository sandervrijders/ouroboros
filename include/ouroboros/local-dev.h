/*
 * Ouroboros - Copyright (C) 2016 - 2018
 *
 * Optimized calls for the local IPCPs
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

#ifndef OUROBOROS_LOCAL_DEV_H
#define OUROBOROS_LOCAL_DEV_H

ssize_t local_flow_read(int fd);

int     local_flow_write(int    fd,
                         size_t idx);

#endif /* OUROBOROS_LOCAL_DEV_H */
