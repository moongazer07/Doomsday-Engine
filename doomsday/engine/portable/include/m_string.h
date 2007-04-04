/**\file
 *\section Copyright and License Summary
 * License: GPL
 * Online License Link: http://www.gnu.org/licenses/gpl.html
 *
 *\author Copyright © 2003-2006 Jaakko Keränen <skyjake@dengine.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

/*
 * m_string.h: Dynamic Strings
 */

#ifndef __DOOMSDAY_DYN_STRING_H__
#define __DOOMSDAY_DYN_STRING_H__

// You can init with string constants, for example:
// ddstring_t mystr = { "Hello world." };

typedef struct ddstring_s {
	char           *str;
	size_t          length;		   // String length (no terminating nulls).
	size_t          size;		   // Allocated memory (not necessarily string size).
} ddstring_t;

void            Str_Init(ddstring_t *ds);
void            Str_Free(ddstring_t *ds);
ddstring_t     *Str_New(void);
void            Str_Delete(ddstring_t *ds);
void            Str_Clear(ddstring_t *ds);
void            Str_Reserve(ddstring_t *ds, size_t length);
void            Str_Set(ddstring_t *ds, const char *text);
void            Str_Append(ddstring_t *ds, const char *append_text);
void            Str_Appendf(ddstring_t *ds, const char *format, ...);
void            Str_PartAppend(ddstring_t *dest, const char *src, int start,
							   size_t count);
void            Str_Prepend(ddstring_t *ds, const char *prepend_text);
size_t          Str_Length(ddstring_t *ds);
char           *Str_Text(ddstring_t *ds);
void            Str_Copy(ddstring_t *dest, ddstring_t *src);
void            Str_StripLeft(ddstring_t *ds);
void            Str_StripRight(ddstring_t *ds);
void            Str_Strip(ddstring_t *ds);
const char     *Str_GetLine(ddstring_t *ds, const char *src);

#endif
