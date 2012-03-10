/**
 * @file sidedef.c
 * SideDef implementation. @ingroup map
 *
 * @authors Copyright &copy; 2003-2012 Jaakko Keränen <jaakko.keranen@iki.fi>
 * @authors Copyright &copy; 2006-2012 Daniel Swanson <danij@dengine.net>
 *
 * @par License
 * GPL: http://www.gnu.org/licenses/gpl.html
 *
 * <small>This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version. This program is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details. You should have received a copy of the GNU
 * General Public License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA</small>
 */

#include "de_base.h"
#include "de_console.h"
#include "de_refresh.h"
#include "de_play.h"

void SideDef_UpdateOrigin(SideDef* side)
{
    assert(side);

    // The side must be owned by a line.
    if(!side->line) return;

    side->origin.pos[VX] = side->line->L_v1pos[VX] + side->line->dX / 2;
    side->origin.pos[VY] = side->line->L_v1pos[VY] + side->line->dY / 2;
}

void SideDef_UpdateSurfaceTangents(SideDef* side)
{
    Surface* surface = &side->SW_topsurface;
    LineDef* line = side->line;
    byte sid;
    assert(side);

    if(!line) return;

    sid = line->L_frontside == side? FRONT : BACK;
    surface->normal[VY] = (line->L_vpos(sid  )[VX] - line->L_vpos(sid^1)[VX]) / line->length;
    surface->normal[VX] = (line->L_vpos(sid^1)[VY] - line->L_vpos(sid  )[VY]) / line->length;
    surface->normal[VZ] = 0;
    V3_BuildTangents(surface->tangent, surface->bitangent, surface->normal);

    // All surfaces of a sidedef have the same vectors.
    memcpy(side->SW_middletangent, surface->tangent, sizeof(surface->tangent));
    memcpy(side->SW_middlebitangent, surface->bitangent, sizeof(surface->bitangent));
    memcpy(side->SW_middlenormal, surface->normal, sizeof(surface->normal));

    memcpy(side->SW_bottomtangent, surface->tangent, sizeof(surface->tangent));
    memcpy(side->SW_bottombitangent, surface->bitangent, sizeof(surface->bitangent));
    memcpy(side->SW_bottomnormal, surface->normal, sizeof(surface->normal));
}

int SideDef_SetProperty(SideDef* sid, const setargs_t* args)
{
    switch(args->prop)
    {
    case DMU_FLAGS:
        DMU_SetValue(DMT_SIDEDEF_FLAGS, &sid->flags, args, 0);
        break;

    case DMU_LINEDEF:
        DMU_SetValue(DMT_SIDEDEF_LINE, &sid->line, args, 0);
        break;

    default:
        Con_Error("SideDef_SetProperty: Property %s is not writable.\n", DMU_Str(args->prop));
    }

    return false; // Continue iteration.
}

int SideDef_GetProperty(const SideDef* sid, setargs_t* args)
{
    switch(args->prop)
    {
    case DMU_SECTOR:
        DMU_GetValue(DMT_SIDEDEF_SECTOR, &sid->sector, args, 0);
        break;
    case DMU_LINEDEF:
        DMU_GetValue(DMT_SIDEDEF_LINE, &sid->line, args, 0);
        break;
    case DMU_FLAGS:
        DMU_GetValue(DMT_SIDEDEF_FLAGS, &sid->flags, args, 0);
        break;
    default:
        Con_Error("SideDef_GetProperty: Has no property %s.\n", DMU_Str(args->prop));
    }

    return false; // Continue iteration.
}
