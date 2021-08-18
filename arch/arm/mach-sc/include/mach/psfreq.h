/* Copyright (C) 2007-2008 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LINUX_PSFREQ_H
#define _LINUX_PSFREQ_H

#ifdef CONFIG_HAS_PSFREQ
void register_freqconv_change(int (*handler)(unsigned int, unsigned int));
void unregister_freqconv_change(int (*handler)(unsigned int, unsigned int));
#else
#define register_freqconv_change(handler) do { } while (0)
#define unregister_freqconv_change(handler) do { } while (0)
#endif

#endif