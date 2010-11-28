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
 * \file		stream.c
 *
 */

/* /////////////////////////////////////////////////////////
 * includes
 */
#include "gstream.h"
#include "../math/math.h"
#include "../varg.h"

/* /////////////////////////////////////////////////////////
 * macros
 */

/* /////////////////////////////////////////////////////////
 * decls
 */

tb_gstream_t* tb_gstream_open_http(tb_char_t const* url);
tb_gstream_t* tb_gstream_open_file(tb_char_t const* url);
tb_gstream_t* tb_gstream_open_data(tb_byte_t const* data, tb_size_t size);

/* /////////////////////////////////////////////////////////
 * details
 */

static tb_int_t tb_gstream_read_no_block(tb_gstream_t* st, tb_byte_t* data, tb_size_t size)
{
	if (!st || !data) return -1;
	if (!size) return 0;

	// {read data from stream data first
	tb_int_t read_n = 0;
	if (st->size > 0)
	{
		// if enough?
		if (st->size > size)
		{
			read_n = size;
			memcpy(data, st->head, read_n);
			st->head += read_n;
			st->size -= read_n;
		}
		else
		{
			read_n = st->size;
			memcpy(data, st->head, read_n);
			st->head = st->data;
			st->size = 0;
		}
	}

	// is enough?
	if (read_n == size) return read_n;

	// read stream directly
	if (!(st->flag & TB_GSTREAM_FLAG_ZLIB))
	{
		// for large block
		tb_int_t need_n = size - read_n;
		if (need_n >= TB_GSTREAM_DATA_MAX)
		{
			tb_int_t ret = st->read(st, data + read_n, need_n);
			if (ret > 0) read_n += ret; 
		}
		// for small block
		else 
		{
			tb_int_t ret = st->read(st, st->head, TB_GSTREAM_DATA_MAX);
			if (ret > 0)
			{
				// if enough?
				st->size = ret;
				if (st->size > need_n)
				{
					memcpy(data + read_n, st->head, need_n);
					read_n += need_n;
					st->head += need_n;
					st->size -= need_n;
				}
				else
				{
					memcpy(data + read_n, st->head, st->size);
					read_n += st->size;
					st->head = st->data;
					st->size = 0;
				}
			}
		}
	}
	else 
	{
#ifdef TB_CONFIG_ZLIB
		// check
		TB_ASSERT(st->hzlib != TB_INVALID_HANDLE);
		
		// exists zdata?
		if (!tb_zlib_left(st->hzlib))
		{
			// read zdata
			tb_int_t zsize = st->read(st, st->zdata, TB_GSTREAM_ZDATA_MAX);
			if (zsize < 0) goto fail;
			else if (!zsize) return read_n;

			// attach zdata
			tb_zlib_attach(st->hzlib, st->zdata, zsize);
		}

		// { inflate data
		tb_int_t need_n = size - read_n;
		tb_int_t real_n = TB_GSTREAM_DATA_MAX;
		if (TB_FALSE == tb_zlib_inflate_partial(st->hzlib, st->data, &real_n) || real_n < 0)
			goto fail;

		// read data
		st->size = real_n;
		if (st->size > 0)
		{
			// if enough?
			if (st->size > need_n)
			{
				memcpy(data + read_n, st->data, need_n);
				st->head += need_n;
				st->size -= need_n;
				read_n += need_n;
			}
			else
			{
				memcpy(data + read_n, st->data, st->size);
				read_n += st->size;
				st->head = st->data;
				st->size = 0;
			}
		}
		// }
#endif
	}

	return read_n;

fail:
	if (read_n) return read_n;
	else return -1;
	// }
}
static tb_int_t tb_gstream_read_block(tb_gstream_t* st, tb_byte_t* data, tb_size_t size)
{
	if (!st || !data) return -1;
	if (!size) return 0;

	// {
	tb_int_t try_n = 100;
	tb_int_t read_n = 0;
	while (read_n < size)
	{
		tb_int_t ret = tb_gstream_read_no_block(st, data + read_n, size - read_n);
		if (ret < 0) goto fail;
		else if (!ret) 
		{
			if (!try_n--) break;
		}
		read_n += ret;
	}

	return read_n;

fail:
	if (read_n) return read_n;
	else return -1;
	// }
}
static tb_int_t tb_gstream_write_no_block(tb_gstream_t* st, tb_byte_t* data, tb_size_t size)
{
	if (!st || !data) return -1;
	if (!size) return 0;

	// write to stream data if enough
	if (TB_GSTREAM_DATA_MAX >= size + st->size) 
	{
		memcpy(st->head + st->size, data, size);
		st->size += size;
		return size;
	}

	// flush stream data
	if (st->size) tb_gstream_flush(st);

	// write stream directly
	tb_int_t write_n = 0;
	if (!(st->flag & TB_GSTREAM_FLAG_ZLIB))
	{
		tb_int_t ret = st->write(st, data, size);
		if (ret > 0) write_n += ret; 
	}

	return write_n;
}
static tb_int_t tb_gstream_write_block(tb_gstream_t* st, tb_byte_t* data, tb_size_t size)
{
	if (!st || !data) return -1;
	if (!size) return 0;

	// {
	tb_int_t try_n = 100;
	tb_int_t write_n = 0;
	while (write_n < size)
	{
		tb_int_t ret = tb_gstream_write_no_block(st, data + write_n, size - write_n);
		if (ret < 0) goto fail;
		else if (!ret) 
		{
			if (!try_n--) break;
		}
		write_n += ret;
	}

	return write_n;

fail:
	if (write_n) return write_n;
	else return -1;
	// }
}

/* /////////////////////////////////////////////////////////
 * interface
 */

void tb_gstream_close(tb_gstream_t* st)
{
	if (st)
	{
		// flush data
		if (st->flag & TB_GSTREAM_FLAG_WO)
			tb_gstream_flush(st);

		// close stream
		if (st->close) st->close(st); 

		// free url
		tb_string_uninit(&st->url);

#ifdef TB_CONFIG_ZLIB
		// free hzlib
		if (st->hzlib != TB_INVALID_HANDLE) tb_zlib_destroy(st->hzlib);
#endif
	}
}

tb_size_t tb_gstream_size(tb_gstream_t const* st)
{
	if (st && st->ssize) return st->ssize(st);
	else return 0;
}
tb_size_t tb_gstream_offset(tb_gstream_t const* st)
{
	if (st) return st->offset;
	else return 0;
}
tb_size_t tb_gstream_left(tb_gstream_t const* st)
{
	tb_size_t size = tb_gstream_size(st);
	tb_size_t offset = tb_gstream_offset(st);
	return (size > offset? (size - offset) : 0);
}
tb_int_t tb_gstream_read(tb_gstream_t* st, tb_byte_t* data, tb_size_t size)
{
	// check
	TB_ASSERT(st && st->read && st->flag & TB_GSTREAM_FLAG_RO);
	if (!st || !st->read || !(st->flag & TB_GSTREAM_FLAG_RO)) return -1;

	// read data
	tb_int_t ret = 0;
	if (st->flag & TB_GSTREAM_FLAG_BLOCK) 
		ret = tb_gstream_read_block(st, data, size);
	else ret = tb_gstream_read_no_block(st, data, size);

	// update offset
	if (ret > 0) st->offset += ret;
	//TB_DBG("read: %d", ret);
	return ret;
}
tb_int_t tb_gstream_write(tb_gstream_t* st, tb_byte_t* data, tb_size_t size)
{
	// check
	TB_ASSERT(st && st->write && st->flag & TB_GSTREAM_FLAG_WO);
	if (!st || !st->write || !(st->flag & TB_GSTREAM_FLAG_WO)) return -1;

	// write data
	tb_int_t ret = 0;
	if (st->flag & TB_GSTREAM_FLAG_BLOCK) 
		ret = tb_gstream_write_block(st, data, size);
	else ret = tb_gstream_write_no_block(st, data, size);

	// update offset
	if (ret > 0) st->offset += ret;
	//TB_DBG("write: %d", ret);
	return ret;
}
void tb_gstream_flush(tb_gstream_t* st)
{
	// check
	TB_ASSERT(st && st->write && st->flag & TB_GSTREAM_FLAG_WO);
	if (!st || !st->write || !(st->flag & TB_GSTREAM_FLAG_WO)) return ;

	if (st->size > 0)
	{
		tb_int_t try_n = 100;
		tb_int_t write_n = 0;
		tb_int_t total_n = st->size;
		while (write_n < total_n)
		{
			tb_int_t ret = st->write(st, st->head + write_n, total_n - write_n);
			if (ret < 0) break;
			else if (!ret) 
			{
				if (!try_n--) break;
			}
			if (ret > 0) write_n += ret; 
		}
		st->head = st->data;
		st->size = 0;
	}
}
tb_int_t tb_gstream_printf(tb_gstream_t* st, tb_char_t const* fmt, ...)
{
	// format data
	tb_char_t data[TB_GSTREAM_DATA_MAX];
	tb_size_t size = 0;
    TB_VARG_FORMAT(data, TB_GSTREAM_DATA_MAX, fmt, &size);

	// write data
	if (size) return tb_gstream_write_block(st, data, size);
	else return 0;
}
tb_byte_t* tb_gstream_need(tb_gstream_t* st, tb_size_t size)
{
	if (!st) return TB_NULL;

	//TB_DBG("need: %d %d", size, st->size);

	// hook
	if (st->need)
	{
		tb_byte_t* p = st->need(st, size);
		if (p) return p;
	}

	// check size
	TB_ASSERT(size <= TB_GSTREAM_DATA_MAX);
	if (size > TB_GSTREAM_DATA_MAX) return TB_NULL;
	if (!size) return st->head;

	// read data from stream data first
	if (st->size > 0)
	{
		// if enough?
		if (st->size > size) return st->head;
		else
		{
			// move data
			memmove(st->data, st->head, st->size);
			st->head = st->data;
		}
	}

	// { read stream directly
	tb_int_t try_n = 100;
	if (!(st->flag & TB_GSTREAM_FLAG_ZLIB))
	{
		while (st->size < size)
		{
			tb_int_t ret = st->read(st, st->data + st->size, size - st->size);
			if (ret < 0) break;
			else if (!ret)
			{
				if (!try_n--) break;
			}
			st->size += ret;
		}
	}
	else 
	{
#ifdef TB_CONFIG_ZLIB
		// check
		TB_ASSERT(st->hzlib != TB_INVALID_HANDLE);
		while (st->size < size)
		{
			// exists zdata?
			if (!tb_zlib_left(st->hzlib))
			{
				// read zdata
				tb_int_t zsize = st->read(st, st->zdata, TB_GSTREAM_ZDATA_MAX);
				if (zsize < 0) return -1;
				else if (!zsize) 
				{
					if (!try_n--) break;
				}

				// attach zdata
				tb_zlib_attach(st->hzlib, st->zdata, zsize);
			}

			// { inflate data
			tb_int_t real_n = size - st->size;

			if (TB_FALSE == tb_zlib_inflate_partial(st->hzlib, st->data + st->size, &real_n) || real_n < 0)
				return -1;
			st->size += real_n;
			// }
		}
#endif
	}
	// }

	if (st->size < size) return TB_NULL;
	else return st->head;
}

tb_bool_t tb_gstream_seek(tb_gstream_t* st, tb_int_t offset, tb_gstream_seek_t flag)
{
	TB_ASSERT(st);
	if (!st) return TB_FALSE;

	// flush data
	if (st->flag & TB_GSTREAM_FLAG_WO)
		tb_gstream_flush(st);

	// hook
	if (st->seek && TB_TRUE == st->seek(st, offset, flag))
		return TB_TRUE;

	// adjust offset
	tb_size_t streamsize = tb_gstream_size(st);
	if (streamsize)
	{
		if (flag == TB_GSTREAM_SEEK_CUR) offset += st->offset;
		else if (flag == TB_GSTREAM_SEEK_END) 
		{
			if (!streamsize) return TB_FALSE;
			offset += streamsize;
		}
	}
	else
	{
		if (flag == TB_GSTREAM_SEEK_BEG && st->offset > offset) return TB_FALSE;
		else if (flag == TB_GSTREAM_SEEK_END && !offset) return TB_FALSE;
		else if (flag == TB_GSTREAM_SEEK_CUR) 
		{
			if (offset < 0) return TB_FALSE;
			offset += st->offset;
		}
	}

	// only for backward
	if (st->offset < offset)
	{
		tb_int_t try_n = 100;
		while (st->offset < offset)
		{
			tb_byte_t data[TB_GSTREAM_DATA_MAX];
			tb_size_t size = TB_MATH_MIN(offset - st->offset, TB_GSTREAM_DATA_MAX);
			tb_int_t ret = tb_gstream_read(st, data, size);
			if (ret != size && st->flag & TB_GSTREAM_FLAG_BLOCK) break;
			else if (ret < 0) break;
			else if (!ret)
			{
				if (!try_n--) break;
			}
		}
	}

	if (st->offset == offset) return TB_TRUE;
	else return TB_FALSE;
}
tb_gstream_flag_t tb_gstream_flag(tb_gstream_t const* st)
{
	if (st) return st->flag;
}
tb_bool_t tb_gstream_switch(tb_gstream_t* st, tb_gstream_flag_t flag)
{
	// the old flag
	tb_uint32_t oflag = st->flag;

#ifdef TB_CONFIG_ZLIB
	// is zlib? 0 => 1
	if (!(oflag & TB_GSTREAM_FLAG_ZLIB) && (flag & TB_GSTREAM_FLAG_ZLIB))
	{
		// create zlib
		st->hzlib = tb_zlib_create();
		if (st->hzlib == TB_INVALID_HANDLE) return TB_FALSE;

		// move zlib data from the stream data
		if (st->size)
		{
			memcpy(st->zdata, st->head, st->size);
			tb_zlib_attach(st->hzlib, st->zdata, st->size);

			st->size = 0;
			st->head = st->data;
		}
	}
	// is zlib? 1 => 0
	else if (!(flag & TB_GSTREAM_FLAG_ZLIB) && (oflag & TB_GSTREAM_FLAG_ZLIB))
	{
		// resume non-zlib data from the zlib data
		// ...
		TB_DBG("[warning]: need resume non-zlib data from the zlib data.");

		// destroy zlib
		if (st->hzlib != TB_INVALID_HANDLE)
			tb_zlib_destroy(st->hzlib);
		st->hzlib = TB_INVALID_HANDLE;
	}

#endif
	// update flag
	st->flag = flag;
	return TB_TRUE;
}

tb_tstream_t* tb_gstream_tstream(tb_gstream_t* st, tb_tstream_t* tstream)
{
	return TB_NULL;
}
tb_gstream_t* tb_gstream_open(tb_ustream_t* st, tb_char_t const* url, tb_byte_t const* data, tb_size_t size, tb_gstream_flag_t flag)
{
	tb_gstream_t* gst = TB_NULL;
	if (url)
	{
		tb_size_t n = strlen(url);
		if (n > 7)
		{
			if (url[0] == 'h'
				&& url[1] == 't'
				&& url[2] == 't'
				&& url[3] == 'p'
				&& url[4] == ':'
				&& url[5] == '/'
				&& url[6] == '/')
			{
				gst = tb_gstream_open_from_http(&st->http, url, flag);
				if (gst) return gst;
			}
			else if (url[0] == 'f'
				&& url[1] == 'i'
				&& url[2] == 'l'
				&& url[3] == 'e'
				&& url[4] == ':'
				&& url[5] == '/'
				&& url[6] == '/')
			{
				gst = tb_gstream_open_from_file(&st->file, &url[6], flag);
				if (gst) return gst;
			}
		}
		gst = tb_gstream_open_from_file(&st->file, url, flag);
		if (gst) return gst;
		gst = tb_gstream_open_from_http(&st->http, url, flag);
		if (gst) return gst;
	}
	if (data && size)
	{
		gst = tb_gstream_open_from_data(&st->data, data, size, flag);
		if (gst) return gst;
	}

	return TB_NULL;
}
tb_char_t const* tb_gstream_url(tb_gstream_t const* st)
{
	return tb_string_c_string(&st->url);
}
