/*!The Tiny Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2009 - 2011, ruki All rights reserved.
 *
 * \author		ruki
 * \file		writer.c
 *
 */

/* /////////////////////////////////////////////////////////
 * includes
 */
#include "writer.h"
#include "../memory/memory.h"

/* /////////////////////////////////////////////////////////
 * interfaces
 */

tb_xml_writer_t* tb_xml_writer_open(tb_gstream_t* gst)
{
	TB_ASSERT(gst);
	if (!gst) return TB_NULL;

	// alloc writer
	tb_xml_writer_t* writer = (tb_xml_writer_t*)tb_malloc(sizeof(tb_xml_writer_t));
	if (!writer) return TB_NULL;

	// init it
	tb_memset(writer, 0, sizeof(tb_xml_writer_t));
	writer->gst = gst;

	// init attributes
	tb_int_t i = 0;
	writer->attributes_n = 0;
	for (i = 0; i < TB_XML_WRITER_ATTRIBUTES_MAX; i++)
	{
		tb_xml_node_t* node = (tb_xml_node_t*)(writer->attributes + i);
		tb_string_init(&node->name);
		tb_string_init(&node->value);
	}

	return writer;
}

void tb_xml_writer_close(tb_xml_writer_t* writer)
{
	if (writer)
	{
		// free attributes
		tb_int_t i = 0;
		for (i = 0; i < TB_XML_WRITER_ATTRIBUTES_MAX; i++)
		{
			tb_xml_node_t* node = (tb_xml_node_t*)(writer->attributes + i);
			tb_string_uninit(&node->name);
			tb_string_uninit(&node->value);
		}

		// detach stream
		writer->gst = TB_NULL;

		// free it
		tb_free(writer);
	}
}
void tb_xml_writer_document_beg(tb_xml_writer_t* writer, tb_char_t const* version, tb_char_t const* encoding)
{
	TB_ASSERT(writer && writer->gst);
	if (!writer || !writer->gst) return ;

	tb_gstream_printf(writer->gst, "<?xml version=\"%s\" encoding=\"%s\"?>", version? version : "", encoding? encoding : "");
}
void tb_xml_writer_document_end(tb_xml_writer_t* writer)
{

}

void tb_xml_writer_element_beg(tb_xml_writer_t* writer, tb_char_t const* name)
{
	TB_ASSERT(writer && writer->gst);
	if (!writer || !writer->gst) return ;

	if (writer->attributes_n) 
	{
		tb_gstream_printf(writer->gst, "<%s", name? name : "");
		tb_int_t i = 0;
		tb_int_t n = writer->attributes_n;
		for (i = 0; i < n; i++)
		{
			tb_char_t const* attr_name = tb_string_c_string(&writer->attributes[i].base.name);
			tb_char_t const* attr_value = tb_string_c_string(&writer->attributes[i].base.value);
			if (attr_name && attr_value) tb_gstream_printf(writer->gst, " %s=\"%s\"", attr_name? attr_name : "", attr_value? attr_value : "");
		}
		tb_gstream_printf(writer->gst, ">");
		tb_xml_writer_attributes_clear(writer);
	}
	else tb_gstream_printf(writer->gst, "<%s>", name? name : "");
}
void tb_xml_writer_element_empty(tb_xml_writer_t* writer, tb_char_t const* name)
{
	TB_ASSERT(writer && writer->gst);
	if (!writer || !writer->gst) return ;

	if (writer->attributes_n) 
	{
		tb_gstream_printf(writer->gst, "<%s", name? name : "");
		tb_int_t i = 0;
		tb_int_t n = writer->attributes_n;
		for (i = 0; i < n; i++)
		{
			tb_char_t const* attr_name = tb_string_c_string(&writer->attributes[i].base.name);
			tb_char_t const* attr_value = tb_string_c_string(&writer->attributes[i].base.value);
			if (attr_name && attr_value) tb_gstream_printf(writer->gst, " %s=\"%s\"", attr_name? attr_name : "", attr_value? attr_value : "");
		}
		tb_gstream_printf(writer->gst, "/>");
		tb_xml_writer_attributes_clear(writer);
	}
	else tb_gstream_printf(writer->gst, "<%s/>", name? name : "");
}
void tb_xml_writer_element_end(tb_xml_writer_t* writer, tb_char_t const* name)
{
	TB_ASSERT(writer && writer->gst);
	if (!writer || !writer->gst) return ;

	tb_gstream_printf(writer->gst, "</%s>", name? name : "");
}
void tb_xml_writer_cdata(tb_xml_writer_t* writer, tb_char_t const* data)
{
	TB_ASSERT(writer && writer->gst);
	if (!writer || !writer->gst) return ;

	tb_gstream_printf(writer->gst, "<![CDATA[%s]]>", data? data : "");
}
void tb_xml_writer_text(tb_xml_writer_t* writer, tb_char_t const* text)
{
	TB_ASSERT(writer && writer->gst);
	if (!writer || !writer->gst) return ;

	tb_gstream_printf(writer->gst, "%s", text? text : "");
}
void tb_xml_writer_comment(tb_xml_writer_t* writer, tb_char_t const* comment)
{
	TB_ASSERT(writer && writer->gst);
	if (!writer || !writer->gst) return ;

	tb_gstream_printf(writer->gst, "<!--%s-->", comment? comment : "");
}
void tb_xml_writer_attributes_clear(tb_xml_writer_t* writer)
{
	TB_ASSERT(writer);
	if (!writer) return ;

	writer->attributes_n = 0;
}
void tb_xml_writer_attributes_add_string(tb_xml_writer_t* writer, tb_char_t const* name, tb_string_t const* value)
{
	TB_ASSERT(writer && name && value);
	if (!writer || !name || !value) return ;

	if (writer->attributes_n < TB_XML_WRITER_ATTRIBUTES_MAX)
	{
		tb_xml_node_t* node = (tb_xml_node_t*)&writer->attributes[writer->attributes_n++];
		tb_string_assign_c_string(&node->name, name);
		tb_string_assign(&node->value, value);
	}
}
void tb_xml_writer_attributes_add_c_string(tb_xml_writer_t* writer, tb_char_t const* name, tb_char_t const* value)
{
	TB_ASSERT(writer && name && value);
	if (!writer || !name || !value) return ;

	if (writer->attributes_n < TB_XML_WRITER_ATTRIBUTES_MAX)
	{
		tb_xml_node_t* node = (tb_xml_node_t*)&writer->attributes[writer->attributes_n++];
		tb_string_assign_c_string(&node->name, name);
		tb_string_assign_c_string(&node->value, value);
	}
}
void tb_xml_writer_attributes_add_int(tb_xml_writer_t* writer, tb_char_t const* name, tb_int_t value)
{
	TB_ASSERT(writer && name);
	if (!writer || !name) return ;

	if (writer->attributes_n < TB_XML_WRITER_ATTRIBUTES_MAX)
	{
		tb_xml_node_t* node = (tb_xml_node_t*)&writer->attributes[writer->attributes_n++];
		tb_string_assign_c_string(&node->name, name);
		tb_string_assign_format(&node->value, "%d", value);
	}
}

#ifdef TB_CONFIG_TYPE_FLOAT
void tb_xml_writer_attributes_add_float(tb_xml_writer_t* writer, tb_char_t const* name, tb_float_t value)
{
	TB_ASSERT(writer && name);
	if (!writer || !name) return ;

	if (writer->attributes_n < TB_XML_WRITER_ATTRIBUTES_MAX)
	{
		tb_xml_node_t* node = (tb_xml_node_t*)&writer->attributes[writer->attributes_n++];
		tb_string_assign_c_string(&node->name, name);
		tb_string_assign_format(&node->value, "%g", value);
	}
}
#endif
void tb_xml_writer_attributes_add_bool(tb_xml_writer_t* writer, tb_char_t const* name, tb_bool_t value)
{
	TB_ASSERT(writer && name);
	if (!writer || !name) return ;

	if (writer->attributes_n < TB_XML_WRITER_ATTRIBUTES_MAX)
	{
		tb_xml_node_t* node = (tb_xml_node_t*)&writer->attributes[writer->attributes_n++];
		tb_string_assign_c_string(&node->name, name);
		tb_string_assign_c_string(&node->value, value == TB_TRUE? "true" : "false");
	}
}
void tb_xml_writer_attributes_add_format(tb_xml_writer_t* writer, tb_char_t const* name, tb_char_t const* fmt, ...)
{
	TB_ASSERT(writer && name && fmt);
	if (!writer || !name || !fmt) return ;

	if (writer->attributes_n < TB_XML_WRITER_ATTRIBUTES_MAX)
	{
		// format text
		tb_char_t text[4096];
		tb_size_t size = 0;
		TB_VARG_FORMAT(text, 4096, fmt, &size);
		if (size) 
		{
			tb_xml_node_t* node = (tb_xml_node_t*)&writer->attributes[writer->attributes_n++];
			tb_string_assign_c_string(&node->name, name);
			tb_string_assign_c_string(&node->value, text);
		}
	}
}
