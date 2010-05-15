/**\file
 *\section License
 * License: GPL
 * Online License Link: http://www.gnu.org/licenses/gpl.html
 *
 *\author Copyright © 2005-2009 Jaakko Keränen <jaakko.keranen@iki.fi>
 *\author Copyright © 2005-2009 Daniel Swanson <danij@dengine.net>
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

/**
 * st_lib.h: HUD, Status Widgets.
 */

#ifndef __COMMON_STLIB__
#define __COMMON_STLIB__

#include "hu_stuff.h"

/**
 * Number widget (uses font drawing).
 */
typedef struct {
    int             x, y; // Upper right-hand corner of the number (right-justified).
    int             maxDigits; // Max # of digits in number.
    float           alpha;
    int*            num; // Pointer to current value.
    gamefontid_t    font;
    boolean         percent;
} st_number_t;

void            STlib_InitNum(st_number_t* n, int x, int y, gamefontid_t font, int* num, int maxDigits, boolean percent, float alpha);
void            STlib_DrawNum(st_number_t* n);

/**
 * Icon widget.
 */
typedef struct {
    int             x, y; // Center-justified location of icon.
    float           alpha;
    patchinfo_t*    p; // Icon.
} st_icon_t;

void            STlib_InitIcon(st_icon_t* b, int x, int y, patchinfo_t* i, float alpha);
void            STlib_DrawIcon(st_icon_t* bi, float alpha);

/**
 * Multi Icon widget.
 */
typedef struct {
    int             x, y; // Center-justified location of icons.
    float           alpha;
    patchinfo_t*    p; // List of icons.
} st_multiicon_t;

void            STlib_InitMultiIcon(st_multiicon_t* mi, int x, int y, patchinfo_t* il, float alpha);
void            STlib_DrawMultiIcon(st_multiicon_t* mi, int icon, float alpha);

#endif
