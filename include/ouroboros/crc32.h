/*
 * Ouroboros - Copyright (C) 2016 - 2017
 *
 * 32-bit Cyclic Redundancy Check
 *
 *    Sander Vrijders   <sander.vrijders@intec.ugent.be>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef OUROBOROS_CRC32_H
#define OUROBOROS_CRC32_H

#include <stdint.h>
#include <stddef.h>

void crc32(uint32_t *   crc,
           const void * buf,
           size_t       len);

#endif /* OUROBOROS_CRC32_H */
