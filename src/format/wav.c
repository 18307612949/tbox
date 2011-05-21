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
 * \file		wav.c
 *
 */

/* /////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"

/* /////////////////////////////////////////////////////////
 * types
 */

/* /////////////////////////////////////////////////////////
 * interfaces
 */

tb_size_t tb_format_wav_probe(tb_gstream_t* gst)
{
	tb_byte_t const* p = tb_gstream_need(gst, 16);
	if (p && p[0] == 'R' && p[1] == 'I' && p[2] == 'F' && p[3] == 'F'
		&& p[8] == 'W' && p[9] == 'A' && p[10] == 'V' && p[11] == 'E'
		&& p[12] == 'f' && p[13] == 'm' && p[14] == 't')
		return TB_FORMAT_SCORE_MAX;
	else return 0;
}
