/*!The Tiny Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2009 - 2010, ruki All rights reserved.
 *
 * \author		ruki
 * \file		malloc.h
 *
 */
#ifndef TB_MALLOC_H
#define TB_MALLOC_H

// c plus plus
#ifdef __cplusplus
extern "C" {
#endif

/* /////////////////////////////////////////////////////////
 * includes
 */
#include "type.h"

/* /////////////////////////////////////////////////////////
 * interfaces
 */

#if 1

# 	define tb_malloc(size) 					tplat_malloc(TPLAT_POOL_PUBLIC_INDEX, size)
# 	define tb_calloc(item, size) 			tplat_calloc(TPLAT_POOL_PUBLIC_INDEX, item, size)
# 	define tb_realloc(data, size) 			tplat_realloc(TPLAT_POOL_PUBLIC_INDEX, data, size)
# 	define tb_free(data) 					tplat_free(TPLAT_POOL_PUBLIC_INDEX, data)

#else

void* 	tb_malloc(tb_size_t size);
void* 	tb_realloc(void* data, tb_size_t size);
void* 	tb_calloc(tb_size_t item, tb_size_t size);
void 	tb_free(void* data);

#endif

// c plus plus
#ifdef __cplusplus
}
#endif

#endif

