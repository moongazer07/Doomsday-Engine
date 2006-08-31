/* DE1: $Id$
 * Copyright (C) 2003, 2004 Jaakko Ker�nen <jaakko.keranen@iki.fi>
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
 * along with this program; if not: http://www.opensource.org/
 */

/*
 * rend_dyn.c: Dynamic Lights
 */

// HEADER FILES ------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "de_base.h"
#include "de_console.h"
#include "de_render.h"
#include "de_refresh.h"
#include "de_play.h"
#include "de_graphics.h"
#include "de_misc.h"

// MACROS ------------------------------------------------------------------

#define X_TO_DLBX(cx)           ( ((cx) - dlBlockOrig.x) >> (FRACBITS+7) )
#define Y_TO_DLBY(cy)           ( ((cy) - dlBlockOrig.y) >> (FRACBITS+7) )
#define DLB_ROOT_DLBXY(bx, by)  (dlBlockLinks + bx + by*dlBlockWidth)
#define LUM_FACTOR(dist)    (1.5f - 1.5f*(dist)/lum->radius)

// TYPES -------------------------------------------------------------------

typedef struct {
    boolean isLit;
    float   height;
    DGLuint decorMap;
} planeitervars_t;

typedef struct {
    float   color[3];
    float   size;
    float   flareSize;
    float   xOffset, yOffset;
} lightconfig_t;

typedef struct seglight_s {
    dynlight_t *wallSection[3];
} seglight_t;

typedef struct subseclight_s {
    dynlight_t *planes[NUM_PLANES];
} subseclight_t;

typedef struct lumcontact_s {
    struct lumcontact_s *next;  // Next in the subsector.
    struct lumcontact_s *nextUsed;  // Next used contact.
    lumobj_t *lum;
} lumcontact_t;

typedef struct contactfinder_data_s {
    fixed_t box[4];
    boolean didSpread;
    lumobj_t *lum;
    int     firstValid;
} contactfinder_data_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern int useDynLights;
extern int useBias;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

boolean dlInited = false;
int     useDynLights = true, dlBlend = 0;
float   dlFactor = 0.7f;        // was 0.6f
int     useWallGlow = true;
float   glowHeightFactor = 3; // glow height as a multiplier
int     glowHeightMax = 100;     // 100 is the default (0-1024)
float   glowFogBright = .15f;
lumobj_t *luminousList = 0;
int     numLuminous = 0, maxLuminous = 0;
int     dlMaxRad = 256;         // Dynamic lights maximum radius.
float   dlRadFactor = 3;
int     maxDynLights = 0;

//int       clipLights = 1;
byte    rendInfoLums = false;

int     dlMinRadForBias = 136; // Lights smaller than this will NEVER
                               // be converted to BIAS sources.

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// Dynlight nodes.
dynlight_t *dynFirst, *dynCursor;

lumobj_t **dlBlockLinks = 0;
vertex_t dlBlockOrig;
int     dlBlockWidth, dlBlockHeight;    // In 128 blocks.
lumobj_t **dlSubLinks = 0;

// A list of dynlight nodes for each surface (seg, subsector-planes[]).
// The segs are indexed by seg index, subSecs are indexed by
// subsector index.
seglight_t *segLightLinks;
subseclight_t *subSecLightLinks;

// List of unused and used lumobj-subsector contacts.
lumcontact_t *contFirst, *contCursor;

// List of lumobj contacts for each subsector.
lumcontact_t **subContacts;

// A framecount for each block. Used to prevent multiple processing of
// a block during one frame.
int    *spreadBlocks;

// CODE --------------------------------------------------------------------

/**
 * Moves all used dynlight nodes to the list of unused nodes, so they
 * can be reused.
 */
static void DL_DeleteUsed(void)
{
    // Start reusing nodes from the first one in the list.
    dynCursor = dynFirst;
    contCursor = contFirst;

    // Clear the surface light links.
    memset(segLightLinks, 0, numsegs * sizeof(seglight_t));
    memset(subSecLightLinks, 0, numsubsectors * sizeof(subseclight_t));

    // Clear lumobj contacts.
    memset(subContacts, 0, numsubsectors * sizeof(lumcontact_t *));
}

/**
 * Returns a new dynlight node. If the list of unused nodes is empty,
 * a new node is created.
 */
dynlight_t *DL_New(float *s, float *t)
{
    dynlight_t *dyn;

    // Have we run out of nodes?
    if(dynCursor == NULL)
    {
        dyn = Z_Malloc(sizeof(dynlight_t), PU_STATIC, NULL);

        // Link the new node to the list.
        dyn->nextUsed = dynFirst;
        dynFirst = dyn;
    }
    else
    {
        dyn = dynCursor;
        dynCursor = dynCursor->nextUsed;
    }

    dyn->next = NULL;
    dyn->flags = 0;

    if(s)
    {
        dyn->s[0] = s[0];
        dyn->s[1] = s[1];
    }
    if(t)
    {
        dyn->t[0] = t[0];
        dyn->t[1] = t[1];
    }

    return dyn;
}

/**
 * Links the dynlight node to the list.
 */
static void DL_Link(dynlight_t *dyn, dynlight_t **list, int index)
{
    dyn->next = list[index];
    list[index] = dyn;
}

static void DL_SubSecLink(dynlight_t *dyn, int index, int plane)
{
    DL_Link(dyn, &subSecLightLinks[index].planes[plane], 0);
}

/**
 * Returns a pointer to the list of dynlights for the subsector plane.
 */
dynlight_t *DL_GetSubSecLightLinks(int ssec, int plane)
{
    return subSecLightLinks[ssec].planes[plane];
}

static void DL_SegLink(dynlight_t *dyn, int index, int segPart)
{
    DL_Link(dyn, &segLightLinks[index].wallSection[segPart], 0);
}

/**
 * Returns a pointer to the list of dynlights for the segment part.
 */
dynlight_t *DL_GetSegLightLinks(int seg, int whichpart)
{
    return segLightLinks[seg].wallSection[whichpart];
}

/**
 * Returns a new lumobj contact. If there are nodes in the list of unused
 * nodes, the new contact is taken from there.
 */
static lumcontact_t *DL_NewContact(lumobj_t * lum)
{
    lumcontact_t *con;

    if(contCursor == NULL)
    {
        con = Z_Malloc(sizeof(lumcontact_t), PU_STATIC, NULL);

        // Link to the list of lumcontact nodes.
        con->nextUsed = contFirst;
        contFirst = con;
    }
    else
    {
        con = contCursor;
        contCursor = contCursor->nextUsed;
    }

    con->lum = lum;
    return con;
}

/**
 * Link the contact to the subsector's list of contacts.
 * The lumobj is contacting the subsector.
 * This called if a light passes the sector spread test.
 * Returns true because this function is also used as an iterator.
 */
static boolean DL_AddContact(subsector_t *subsector, void *lum)
{
    lumcontact_t *con = DL_NewContact(lum);
    lumcontact_t **list = subContacts + GET_SUBSECTOR_IDX(subsector);

    con->next = *list;
    *list = con;
    return true;
}

void DL_ThingRadius(lumobj_t * lum, lightconfig_t * cf)
{
    lum->radius = cf->size * 40 * dlRadFactor;

    // Don't make a too small or too large light.
    if(lum->radius < 32)
        lum->radius = 32;
    if(lum->radius > dlMaxRad)
        lum->radius = dlMaxRad;

    lum->flareSize = cf->flareSize * 60 * (50 + haloSize) / 100.0f;

    if(lum->flareSize < 8)
        lum->flareSize = 8;
}

void DL_ThingColor(lumobj_t * lum, DGLubyte * outRGB, float light)
{
    int     i;

    if(light < 0)
        light = 0;
    if(light > 1)
        light = 1;
    light *= dlFactor;

    // If fog is enabled, make the light dimmer.
    // FIXME: This should be a cvar.
    if(usingFog)
        light *= .5f;           // Would be too much.

    if(lum->decorMap)
    {   // Decoration maps are pre-colored.
        light *= 255;
    }

    // Multiply with the light color.
    for(i = 0; i < 3; i++)
    {
        if(!lum->decorMap)
        {
            outRGB[i] = (DGLubyte) (light * lum->rgb[i]);
        }
        else
        {
            // Decoration maps are pre-colored.
            outRGB[i] = (DGLubyte) (light);
        }
    }
}

void DL_InitLinks(void)
{
    vertex_t min, max;

    // First initialize the subsector links (root pointers).
    dlSubLinks = Z_Calloc(sizeof(lumobj_t *) * numsubsectors, PU_LEVEL, 0);

    // Then the blocklinks.
    R_GetMapSize(&min, &max);

    // Origin has fixed-point coordinates.
    memcpy(&dlBlockOrig, &min, sizeof(min));
    max.x -= min.x;
    max.y -= min.y;
    dlBlockWidth = (max.x >> (FRACBITS + 7)) + 1;
    dlBlockHeight = (max.y >> (FRACBITS + 7)) + 1;

    // Blocklinks is a table of lumobj_t pointers.
    dlBlockLinks =
        realloc(dlBlockLinks,
                sizeof(lumobj_t *) * dlBlockWidth * dlBlockHeight);

    // Initialize the dynlight -> surface links.
    segLightLinks =
        Z_Calloc(numsegs * sizeof(seglight_t), PU_LEVELSTATIC, 0);
    subSecLightLinks =
        Z_Calloc(numsubsectors * sizeof(subseclight_t), PU_LEVELSTATIC, 0);

    // Initialize lumobj -> subsector contacts.
    subContacts =
        Z_Calloc(numsubsectors * sizeof(lumcontact_t *), PU_LEVELSTATIC, 0);

    // A framecount was each block.
    spreadBlocks =
        Z_Malloc(sizeof(int) * dlBlockWidth * dlBlockHeight, PU_LEVEL, 0);
}

/**
 * Returns true if the coords are in range.
 */
static boolean DL_SegTexCoords(float *t, float top, float bottom,
                               lumobj_t * lum)
{
    float   lightZ = FIX2FLT(lum->thing->pos[VZ]) + lum->center;
    float   radius = lum->radius / DYN_ASPECT;

    t[0] = (lightZ + radius - top) / (2 * radius);
    t[1] = t[0] + (top - bottom) / (2 * radius);

    return t[0] < 1 && t[1] > 0;
}

/**
 * The front sector must be given because of polyobjs.
 */
void DL_ProcessWallSeg(lumobj_t * lum, seg_t *seg, sector_t *frontsec)
{
#define SMIDDLE 0x1
#define STOP    0x2
#define SBOTTOM 0x4
    int     present = 0;
    sector_t *backsec = seg->backsector;
    side_t *sdef = seg->sidedef;
    float   pos[2][2], s[2], t[2];
    float   dist, pntLight[2];
    float   fceil, ffloor, bceil, bfloor;
    float   top[2], bottom[2];
    dynlight_t *dyn;
    int     segindex = GET_SEG_IDX(seg);
    boolean backSide = false;

    fceil = SECT_CEIL(frontsec);
    ffloor = SECT_FLOOR(frontsec);

    // A zero-volume sector?
    if(fceil <= ffloor)
        return;

    // Which side?
    if(SIDE_PTR(seg->linedef->sidenum[0]) != seg->sidedef)
        backSide = true;

    if(backsec)
    {
        bceil = SECT_CEIL(backsec);
        bfloor = SECT_FLOOR(backsec);
    }
    else
    {
        bceil = bfloor = 0;
    }

    // Let's begin with an analysis of the visible surfaces.
    // Is there a mid wall segment?
    if(Rend_IsWallSectionPVisible(seg->linedef, SEG_MIDDLE, backSide))
    {
        present |= SMIDDLE;
        if(backsec)
        {
            // Check the middle texture's mask status.
            if(sdef->middle.texture > 0)
            {
                if(sdef->middle.isflat)
                    GL_PrepareFlat2(sdef->middle.texture, true);
                else
                    GL_GetTextureInfo(sdef->middle.texture);
            }
            /*if(texmask)
               {
               // We can't light masked textures.
               // FIXME: Use vertex lighting.
               present &= ~SMIDDLE;
               } */
        }
    }

    // Is there a top wall segment?
    if(Rend_IsWallSectionPVisible(seg->linedef, SEG_TOP, backSide))
        present |= STOP;

    // Is there a lower wall segment?
    if(Rend_IsWallSectionPVisible(seg->linedef, SEG_BOTTOM, backSide))
        present |= SBOTTOM;

    // There are no surfaces to light!
    if(!present)
        return;

    pos[0][VX] = FIX2FLT(seg->v1->x);
    pos[0][VY] = FIX2FLT(seg->v1->y);
    pos[1][VX] = FIX2FLT(seg->v2->x);
    pos[1][VY] = FIX2FLT(seg->v2->y);

    // We will only calculate light placement for segs that are facing
    // the viewpoint.
    if(!Rend_SegFacingDir(pos[0], pos[1]))
        return;

    pntLight[VX] = FIX2FLT(lum->thing->pos[VX]);
    pntLight[VY] = FIX2FLT(lum->thing->pos[VY]);

    // Calculate distance between seg and light source.
    dist =
        ((pos[0][VY] - pntLight[VY]) * (pos[1][VX] - pos[0][VX]) -
         (pos[0][VX] - pntLight[VX]) * (pos[1][VY] -
                                        pos[0][VY])) / seg->length;

    // Is it close enough and on the right side?
    if(dist < 0 || dist > lum->radius)
        return; // Nope.

    // Do a scalar projection for the offset.
    s[0] =
        (-
         ((pos[0][VY] - pntLight[VY]) * (pos[0][VY] - pos[1][VY]) -
          (pos[0][VX] - pntLight[VX]) * (pos[1][VX] -
                                         pos[0][VX])) / seg->length +
         lum->radius) / (2 * lum->radius);

    s[1] = s[0] + seg->length / (2 * lum->radius);

    // Would the light be visible?
    if(s[0] >= 1 || s[1] <= 0)
        return;  // Outside the seg.

    // Process the visible parts of the segment.
    if(present & SMIDDLE)
    {
        if(backsec)
        {
            top[0] = top[1] = (fceil < bceil ? fceil : bceil);
            bottom[0] = bottom[1] = (ffloor > bfloor ? ffloor : bfloor);
            Rend_MidTexturePos(&bottom[0], &bottom[1], &top[0], &top[1],
                               NULL, sdef->middle.offy,
                               seg->linedef ? (seg->linedef->
                                               flags & ML_DONTPEGBOTTOM) !=
                               0 : false);
        }
        else
        {
            top[0] = top[1] = fceil;
            bottom[0] = bottom[1] = ffloor;
        }

        if(DL_SegTexCoords(t, top[0], bottom[0], lum) &&
           DL_SegTexCoords(t, top[1], bottom[1], lum))
        {
            dyn = DL_New(s, t);
            DL_ThingColor(lum, dyn->color, LUM_FACTOR(dist));
            dyn->texture = lum->tex;
            DL_SegLink(dyn, segindex, SEG_MIDDLE);
        }
    }
    if(present & STOP)
    {
        if(DL_SegTexCoords(t, fceil, MAX_OF(ffloor, bceil), lum))
        {
            dyn = DL_New(s, t);
            DL_ThingColor(lum, dyn->color, LUM_FACTOR(dist));
            dyn->texture = lum->tex;
            DL_SegLink(dyn, segindex, SEG_TOP);
        }
    }
    if(present & SBOTTOM)
    {
        if(DL_SegTexCoords(t, MIN_OF(bfloor, fceil), ffloor, lum))
        {
            dyn = DL_New(s, t);
            DL_ThingColor(lum, dyn->color, LUM_FACTOR(dist));
            dyn->texture = lum->tex;
            DL_SegLink(dyn, segindex, SEG_BOTTOM);
        }
    }
#undef SMIDDLE
#undef STOP
#undef SBOTTOM
}

/**
 * Generates one dynlight node per plane glow. The light is attached to
 * the appropriate seg part.
 */
static void DL_CreateGlowLights(seg_t *seg, int part, float segtop,
                                float segbottom, boolean glow_floor,
                                boolean glow_ceil)
{
    dynlight_t *dyn;
    int     i, g, segindex = GET_SEG_IDX(seg);
    float   ceil, floor, top, bottom, s[2], t[2];
    float   glowHeight;
    sector_t *sect = seg->sidedef->sector;

    // Check the heights.
    if(segtop <= segbottom)
        return;                 // No height.

    ceil = SECT_CEIL(sect);
    floor = SECT_FLOOR(sect);

    if(segtop > ceil)
        segtop = ceil;
    if(segbottom < floor)
        segbottom = floor;

    for(g = 0; g < 2; ++g)
    {
        // Only do what's told.
        if((g == PLN_CEILING && !glow_ceil) || (g == PLN_FLOOR && !glow_floor))
            continue;

        // Calculate texture coords for the light.
        // The horizontal direction is easy.
        s[0] = 0;
        s[1] = 1;

        glowHeight =
            (MAX_GLOWHEIGHT * sect->planes[g].glow) * glowHeightFactor;

        // Don't make too small or too large glows.
        if(glowHeight <= 2)
            continue;

        if(glowHeight > glowHeightMax)
            glowHeight = glowHeightMax;

        if(g == PLN_CEILING)
        {   // Ceiling glow.
            top = ceil;
            bottom = ceil - glowHeight;

            t[1] = t[0] = (top - segtop) / glowHeight;
            t[1]+= (segtop - segbottom) / glowHeight;

            if(t[0] > 1 || t[1] < 0)
                continue;
        }
        else
        {   // Floor glow.
            bottom = floor;
            top = floor + glowHeight;

            t[0] = t[1] = (segbottom - bottom) / glowHeight;
            t[0]+= (segtop - segbottom) / glowHeight;

            if(t[1] > 1 || t[0] < 0)
                continue;
        }

        dyn = DL_New(s, t);
        memcpy(dyn->color, sect->planes[g].glowrgb, 3);

        dyn->texture = GL_PrepareLSTexture(LST_GRADIENT);

        for(i = 0; i < 3; ++i)
        {
            dyn->color[i] *= dlFactor;

            // In fog, additive blending is used. The normal fog color
            // is way too bright.
            if(usingFog)
                dyn->color[i] *= glowFogBright;
        }
        DL_SegLink(dyn, segindex, part);
    }
}

/**
 * If necessary, generate dynamic lights for plane glow.
 */
static void DL_ProcessWallGlow(seg_t *seg, sector_t *sect)
{
    boolean do_floor = (sect->SP_floorglow > 0)? true : false;
    boolean do_ceil = (sect->SP_ceilglow > 0)? true : false;
    sector_t *back = seg->backsector;
    side_t *sdef = seg->sidedef;
    float   fceil, ffloor, bceil, bfloor;
    float   opentop, openbottom;    //, top, bottom;
    float   v1[2], v2[2];
    boolean backSide = false;

    // Check if this segment is actually facing our way.
    v1[VX] = FIX2FLT(seg->v1->x);
    v1[VY] = FIX2FLT(seg->v1->y);
    v2[VX] = FIX2FLT(seg->v2->x);
    v2[VY] = FIX2FLT(seg->v2->y);
    if(!Rend_SegFacingDir(v1, v2))
        return;                 // Nope...

    // Which side?
    if(SIDE_PTR(seg->linedef->sidenum[0]) != seg->sidedef)
        backSide = true;

    // Visible plane heights.
    fceil = SECT_CEIL(sect);
    ffloor = SECT_FLOOR(sect);
    if(back)
    {
        bceil = SECT_CEIL(back);
        bfloor = SECT_FLOOR(back);
    }

    // Determine which portions of the segment get lit.
    if(!back)
    {
        // One sided.
        DL_CreateGlowLights(seg, SEG_MIDDLE, fceil, ffloor, do_floor, do_ceil);
    }
    else
    {
        // Two-sided.
        opentop = MIN_OF(fceil, bceil);
        openbottom = MAX_OF(ffloor, bfloor);

        // The glow can only be visible in the front sector's height range.

        // Is there a middle?
        if(Rend_IsWallSectionPVisible(seg->linedef, SEG_MIDDLE, backSide))
        {
            if(sdef->middle.texture > 0)
            {
                if(sdef->middle.isflat)
                    GL_PrepareFlat2(sdef->middle.texture, true);
                else
                    GL_GetTextureInfo(sdef->middle.texture);
            }

            if(!texmask)
            {
                DL_CreateGlowLights(seg, SEG_MIDDLE, opentop, openbottom,
                                    do_floor, do_ceil);
            }
        }

        // Top?
        if(Rend_IsWallSectionPVisible(seg->linedef, SEG_TOP, backSide))
        {
            DL_CreateGlowLights(seg, SEG_TOP, fceil, bceil, do_floor, do_ceil);
        }

        // Bottom?
        if(Rend_IsWallSectionPVisible(seg->linedef, SEG_BOTTOM, backSide))
        {
            DL_CreateGlowLights(seg, SEG_BOTTOM, bfloor, ffloor, do_floor,
                                do_ceil);
        }
    }
}

void DL_Clear(void)
{
    if(luminousList)
        Z_Free(luminousList);
    luminousList = 0;
    maxLuminous = numLuminous = 0;

    free(dlBlockLinks);
    dlBlockLinks = 0;
    dlBlockOrig.x = dlBlockOrig.y = 0;
    dlBlockWidth = dlBlockHeight = 0;
}

void DL_ClearForFrame(void)
{
#ifdef DD_PROFILE
    static int i;

    if(++i > 40)
    {
        i = 0;
        PRINT_PROF(PROF_DYN_INIT_DEL);
        PRINT_PROF(PROF_DYN_INIT_ADD);
        PRINT_PROF(PROF_DYN_INIT_LINK);
    }
#endif

    // Clear all the roots.
    memset(dlSubLinks, 0, sizeof(lumobj_t *) * numsubsectors);
    memset(dlBlockLinks, 0, sizeof(lumobj_t *) * dlBlockWidth * dlBlockHeight);

    numLuminous = 0;
}

/**
 * Allocates a new lumobj and returns a pointer to it.
 */
int DL_NewLuminous(void)
{
    lumobj_t *newList;

    numLuminous++;

    // Only allocate memory when it's needed.
    // FIXME: No upper limit?
    if(numLuminous > maxLuminous)
    {
        maxLuminous *= 2;

        // The first time, allocate eight lumobjs.
        if(!maxLuminous)
            maxLuminous = 8;

        newList = Z_Malloc(sizeof(lumobj_t) * maxLuminous, PU_STATIC, 0);

        // Copy the old data over to the new list.
        if(luminousList)
        {
            memcpy(newList, luminousList,
                   sizeof(lumobj_t) * (numLuminous - 1));
            Z_Free(luminousList);
        }
        luminousList = newList;
    }

    // Clear the new lumobj.
    memset(luminousList + numLuminous - 1, 0, sizeof(lumobj_t));

    return numLuminous;         // == index + 1
}

/**
 * Returns a pointer to the lumobj with the given 1-based index.
 */
lumobj_t *DL_GetLuminous(int index)
{
    if(index <= 0 || index > numLuminous)
        return NULL;
    return luminousList + index - 1;
}

/**
 * Returns true if we HAVE to use a dynamic light for this light defintion.
 */
static boolean DL_MustUseDynamic(ded_light_t *def)
{
    // Are any of the light directions disabled or use a custom lightmap?
    if(def && (def->sides.tex != 0 || def->up.tex != 0 || def->down.tex != 0))
        return true;
    else
        return false;
}

/**
 * Registers the given thing as a luminous, light-emitting object.
 * Note that this is called each frame for each luminous object!
 */
void DL_AddLuminous(mobj_t *thing)
{
    spritedef_t *sprdef;
    spriteframe_t *sprframe;
    int     i, lump;
    float   mul;
    lumobj_t *lum;
    lightconfig_t cf;
    ded_light_t *def = 0;
    modeldef_t *mf, *nextmf;
    float xOff;
    float center;
    int   flags = 0;
    int   radius, flareSize;
    float  rgb[3];

    // Has BIAS lighting been disabled?
    // If this thing has aquired a BIAS source we need to delete it.
    if(thing->usingBias)
    {
        if(!useBias)
        {
            SB_Delete(thing->light - 1);

            thing->light = 0;
            thing->usingBias = false;
        }
    }
    else
        thing->light = 0;

    if(((thing->state && (thing->state->flags & STF_FULLBRIGHT)) &&
         !(thing->ddflags & DDMF_DONTDRAW)) ||
       (thing->ddflags & DDMF_ALWAYSLIT))
    {
        // Are the automatically calculated light values for fullbright
        // sprite frames in use?
        if(thing->state && (thing->state->flags & STF_NOAUTOLIGHT) &&
           !thing->state->light)
           return;

        // Determine the sprite frame lump of the source.
        sprdef = &sprites[thing->sprite];
        sprframe = &sprdef->spriteframes[thing->frame];
        if(sprframe->rotate)
        {
            lump =
                sprframe->
                lump[(R_PointToAngle(thing->pos[VX], thing->pos[VY]) - thing->angle +
                      (unsigned) (ANG45 / 2) * 9) >> 29];
        }
        else
        {
            lump = sprframe->lump[0];
        }

        // This'll ensure we have up-to-date information about the texture.
        GL_PrepareSprite(lump, 0);

        // Let's see what our light should look like.
        cf.size = cf.flareSize = spritelumps[lump].lumsize;
        cf.xOffset = spritelumps[lump].flarex;
        cf.yOffset = spritelumps[lump].flarey;

        // X offset to the flare position.
        xOff = cf.xOffset - spritelumps[lump].width / 2.0f;

        // Does the thing have an active light definition?
        if(thing->state && thing->state->light)
        {
            def = (ded_light_t *) thing->state->light;
            if(def->size)
                cf.size = def->size;
            if(def->offset[VX])
            {
                // Set the x offset here.
                xOff = cf.xOffset = def->offset[VX];
            }
            if(def->offset[VY])
                cf.yOffset = def->offset[VY];
            if(def->halo_radius)
                cf.flareSize = def->halo_radius;
            flags |= def->flags;
        }

        center =
            spritelumps[lump].topoffset -
            FIX2FLT(thing->floorclip + R_GetBobOffset(thing)) -
            cf.yOffset;

        // Will the sprite be allowed to go inside the floor?
        mul =
            FIX2FLT(thing->pos[VZ]) + spritelumps[lump].topoffset -
            spritelumps[lump].height -
            FIX2FLT(thing->subsector->sector->planes[PLN_FLOOR].height);
        if(!(thing->ddflags & DDMF_NOFITBOTTOM) && mul < 0)
        {
            // Must adjust.
            center -= mul;
        }

        // Sets the dynlight and flare radii.
        //DL_ThingRadius(lum, &cf);

        radius = cf.size * 40 * dlRadFactor;

        // Don't make a too small light.
        if(radius < 32)
            radius = 32;

        flareSize = cf.flareSize * 60 * (50 + haloSize) / 100.0f;

        if(flareSize < 8)
            flareSize = 8;

        // Does the mobj use a light scale?
        if(thing->ddflags & DDMF_LIGHTSCALE)
        {
            // Also reduce the size of the light according to
            // the scale flags. *Won't affect the flare.*
            mul =
                1.0f -
                ((thing->ddflags & DDMF_LIGHTSCALE) >> DDMF_LIGHTSCALESHIFT) /
                4.0f;
            radius *= mul;
        }

        if(def && (def->color[0] || def->color[1] || def->color[2]))
        {
            // If any of the color components are != 0, use the
            // definition's color.
            for(i = 0; i < 3; ++i)
                rgb[i] = def->color[i];
        }
        else
        {
            // Use the sprite's (amplified) color.
            GL_GetSpriteColorf(lump, rgb);
        }

        if(useBias && thing->usingBias)
        {   // We have previously acquired a BIAS source for this thing.
            if(radius < dlMinRadForBias || DL_MustUseDynamic(def))
            {   // We can nolonger use a BIAS source for this light.
                // Delete the bias source (it will be replaced with a
                // dynlight shortly).
                SB_Delete(thing->light - 1);

                thing->light = 0;
                thing->usingBias = false;
            }
            else
            {   // Update BIAS source properties.
                SB_UpdateSource(thing->light - 1,
                                FIX2FLT(thing->pos[VX]),
                                FIX2FLT(thing->pos[VY]),
                                FIX2FLT(thing->pos[VZ]) + center,
                                radius * 0.3f, 0, 255, rgb);
                return;
            }
        }

        // Should we attempt to acquire a BIAS light source for this?
        if(useBias && radius >= dlMinRadForBias &&
           !DL_MustUseDynamic(def))
        {
            if((thing->light =
                    SB_NewSourceAt(FIX2FLT(thing->pos[VX]),
                                   FIX2FLT(thing->pos[VY]),
                                   FIX2FLT(thing->pos[VZ]) + center,
                                   radius * 0.3f, 0, 255, rgb)) != -1)
            {
                // We've acquired a BIAS source for this light.
                thing->usingBias = true;
            }
        }

        if(!thing->usingBias) // Nope, a dynlight then.
        {
            // This'll allow a halo to be rendered. If the light is hidden from
            // view by world geometry, the light pointer will be set to NULL.
            thing->light = DL_NewLuminous();

            lum = DL_GetLuminous(thing->light);
            lum->thing = thing;
            lum->patch = lump;
            lum->center = center;
            lum->xOff = xOff;
            lum->flags = flags;
            lum->flags |= LUMF_CLIPPED;

            // Don't make too large a light.
            if(radius > dlMaxRad)
                radius = dlMaxRad;

            lum->radius = radius;
            lum->flareMul = 1;
            lum->flareSize = flareSize;
            for(i = 0; i < 3; ++i)
                lum->rgb[i] = (byte) (255 * rgb[i]);

            // Approximate the distance in 3D.
            lum->distance =
                P_ApproxDistance3(thing->pos[VX] - viewx, thing->pos[VY] - viewy,
                                  thing->pos[VZ] - viewz);

            // Is there a model definition?
            R_CheckModelFor(thing, &mf, &nextmf);
            if(mf && useModels)
                lum->xyScale = MAX_OF(mf->scale[VX], mf->scale[VZ]);
            else
                lum->xyScale = 1;

            // This light source is not associated with a decormap.
            lum->decorMap = 0;

            if(def)
            {
                lum->tex = def->sides.tex;
                lum->ceilTex = def->up.tex;
                lum->floorTex = def->down.tex;

                if(def->flare.disabled)
                    lum->flags |= LUMF_NOHALO;
                else
                {
                    lum->flareCustom = def->flare.custom;
                    lum->flareTex = def->flare.tex;
                }
            }
            else
            {
                // Use the same default light texture for all directions.
                lum->tex = lum->ceilTex = lum->floorTex =
                    GL_PrepareLSTexture(LST_DYNAMIC);
            }
        }
    }
    else if(thing->usingBias)
    {   // light is nolonger needed & there is a previously aquired BIAS source.
        // Delete the existing Bias source.
        SB_Delete(thing->light - 1);

        thing->light = 0;
        thing->usingBias = false;
    }
}

static void DL_ContactSector(lumobj_t * lum, fixed_t *box, sector_t *sector)
{
    P_SubsectorBoxIterator(box, sector, DL_AddContact, lum);
}

static boolean DLIT_ContactFinder(line_t *line, void *data)
{
    contactfinder_data_t *light = data;
    sector_t *source, *dest;
    lineinfo_t *info;
    float   distance;

    if(!line->backsector || !line->frontsector ||
       line->frontsector == line->backsector)
    {
        // Line must be between two different sectors.
        return true;
    }

    // Which way does the spread go?
    if(line->frontsector->validcount == validcount)
    {
        source = line->frontsector;
        dest = line->backsector;
    }
    else if(line->backsector->validcount == validcount)
    {
        source = line->backsector;
        dest = line->frontsector;
    }
    else
    {
        // Not eligible for spreading.
        return true;
    }

    if(dest->validcount >= light->firstValid &&
       dest->validcount <= validcount + 1)
    {
        // This was already spreaded to.
        return true;
    }

    // Is this line inside the light's bounds?
    if(line->bbox[BOXRIGHT] <= light->box[BOXLEFT] ||
       line->bbox[BOXLEFT] >= light->box[BOXRIGHT] ||
       line->bbox[BOXTOP] <= light->box[BOXBOTTOM] ||
       line->bbox[BOXBOTTOM] >= light->box[BOXTOP])
    {
        // The line is not inside the light's bounds.
        return true;
    }

    // Can the spread happen?
    if(dest->planes[PLN_CEILING].height <= dest->planes[PLN_FLOOR].height ||
       dest->planes[PLN_CEILING].height <= source->planes[PLN_FLOOR].height ||
       dest->planes[PLN_FLOOR].height >= source->planes[PLN_CEILING].height)
    {
        // No; destination sector is closed with no height.
        return true;
    }

    info = lineinfo + GET_LINE_IDX(line);
    if(info->length <= 0)
    {
        // This can't be a good line.
        return true;
    }

    // Calculate distance to line.
    distance =
        (FIX2FLT(line->v1->y - light->lum->thing->pos[VY]) * FIX2FLT(line->dx) -
         FIX2FLT(line->v1->x -
                 light->lum->thing->pos[VX]) * FIX2FLT(line->dy)) / info->length;

    if((source == line->frontsector && distance < 0) ||
       (source == line->backsector && distance > 0))
    {
        // Can't spread in this direction.
        return true;
    }

    // Check distance against the light radius.
    if(distance < 0)
        distance = -distance;
    if(distance >= light->lum->radius)
    {
        // The light doesn't reach that far.
        return true;
    }

    // Light spreads to the destination sector.
    light->didSpread = true;

    // During next step, light will continue spreading from there.
    dest->validcount = validcount + 1;

    // Add this lumobj to the destination's subsectors.
    DL_ContactSector(light->lum, light->box, dest);

    return true;
}

/**
 * Create a contact for this lumobj in all the subsectors this light
 * source is contacting (tests done on bounding boxes and the sector
 * spread test).
 */
static void DL_FindContacts(lumobj_t * lum)
{
    int     firstValid = ++validcount;
    int     xl, yl, xh, yh, bx, by;
    contactfinder_data_t light;

    // Use a slightly smaller radius than what the light really is.
    fixed_t radius = lum->radius * FRACUNIT - 2 * FRACUNIT;
    static uint numSpreads = 0, numFinds = 0;

    // Do the sector spread. Begin from the light's own sector.
    lum->thing->subsector->sector->validcount = validcount;

    light.lum = lum;
    light.firstValid = firstValid;
    light.box[BOXTOP] = lum->thing->pos[VY] + radius;
    light.box[BOXBOTTOM] = lum->thing->pos[VY] - radius;
    light.box[BOXRIGHT] = lum->thing->pos[VX] + radius;
    light.box[BOXLEFT] = lum->thing->pos[VX] - radius;

    DL_ContactSector(lum, light.box, lum->thing->subsector->sector);

    xl = (light.box[BOXLEFT] - bmaporgx) >> MAPBLOCKSHIFT;
    xh = (light.box[BOXRIGHT] - bmaporgx) >> MAPBLOCKSHIFT;
    yl = (light.box[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
    yh = (light.box[BOXTOP] - bmaporgy) >> MAPBLOCKSHIFT;

    numFinds++;

    // We'll keep doing this until the light has spreaded everywhere
    // inside the bounding box.
    do
    {
        light.didSpread = false;

        numSpreads++;

        for(bx = xl; bx <= xh; bx++)
            for(by = yl; by <= yh; by++)
                P_BlockLinesIterator(bx, by, DLIT_ContactFinder, &light);

        // Increment validcount for the next round of spreading.
        validcount++;
    }
    while(light.didSpread);

    /*#ifdef _DEBUG
       if(!((numFinds + 1) % 1000))
       {
       Con_Printf("finds=%i avg=%.3f\n", numFinds,
       numSpreads / (float)numFinds);
       }
       #endif */
}

static void DL_SpreadBlocks(subsector_t *subsector)
{
    int     xl, xh, yl, yh, x, y, *count;
    lumobj_t *iter;

    xl = X_TO_DLBX(FLT2FIX(subsector->bbox[0].x - dlMaxRad));
    xh = X_TO_DLBX(FLT2FIX(subsector->bbox[1].x + dlMaxRad));
    yl = Y_TO_DLBY(FLT2FIX(subsector->bbox[0].y - dlMaxRad));
    yh = Y_TO_DLBY(FLT2FIX(subsector->bbox[1].y + dlMaxRad));

    // Are we completely outside the blockmap?
    if(xh < 0 || xl >= dlBlockWidth || yh < 0 || yl >= dlBlockHeight)
        return;

    // Clip to blockmap bounds.
    if(xl < 0)
        xl = 0;
    if(xh >= dlBlockWidth)
        xh = dlBlockWidth - 1;
    if(yl < 0)
        yl = 0;
    if(yh >= dlBlockHeight)
        yh = dlBlockHeight - 1;

    for(x = xl; x <= xh; x++)
        for(y = yl; y <= yh; y++)
        {
            count = &spreadBlocks[x + y * dlBlockWidth];
            if(*count != framecount)
            {
                *count = framecount;

                // Spread the lumobjs in this block.
                for(iter = *DLB_ROOT_DLBXY(x, y); iter; iter = iter->next)
                    DL_FindContacts(iter);
            }
        }
}

/**
 * Used to sort lumobjs by distance from viewpoint.
 */
static int C_DECL lumobjSorter(const void *e1, const void *e2)
{
    lumobj_t *lum1 = DL_GetLuminous(*(const ushort *) e1);
    lumobj_t *lum2 = DL_GetLuminous(*(const ushort *) e2);

    if(lum1->distance > lum2->distance)
        return 1;
    if(lum1->distance < lum2->distance)
        return -1;
    return 0;
}

/**
 * Clears the dlBlockLinks and then links all the listed luminous objects.
 */
static void DL_LinkLuminous(void)
{
#define MAX_LUMS 8192           // Normally 100-200, heavy: 1000
    ushort  order[MAX_LUMS];

    lumobj_t **root, *lum;
    int     i, bx, by, num = numLuminous;

    // Should the proper order be determined?
    if(maxDynLights)
    {
        if(num > maxDynLights)
            num = maxDynLights;

        // Init the indices.
        for(i = 0; i < numLuminous; i++)
            order[i] = i + 1;

        qsort(order, numLuminous, sizeof(ushort), lumobjSorter);
    }

    for(i = 0; i < num; i++)
    {
        lum = (maxDynLights ? DL_GetLuminous(order[i]) : luminousList + i);

        // Link this lumobj to the dlBlockLinks, if it can be linked.
        lum->next = NULL;
        bx = X_TO_DLBX(lum->thing->pos[VX]);
        by = Y_TO_DLBY(lum->thing->pos[VY]);
        if(bx >= 0 && by >= 0 && bx < dlBlockWidth && by < dlBlockHeight)
        {
            root = DLB_ROOT_DLBXY(bx, by);
            lum->next = *root;
            *root = lum;
        }

        // Link this lumobj into its subsector (always possible).
        root = dlSubLinks + GET_SUBSECTOR_IDX(lum->thing->subsector);
        lum->ssNext = *root;
        *root = lum;
    }
}

/**
 * Returns true if the texture is already used in the list of dynlights.
 */
static boolean DL_IsTexUsed(dynlight_t *node, DGLuint texture)
{
    for(; node; node = node->next)
        if(node->texture == texture)
            return true;
    return false;
}

/**
 * Process the given lumobj to maybe add a dynamic light for the plane.
 */
static void DL_ProcessPlane(lumobj_t *lum, subsector_t *subsector,
                            int planeID, planeitervars_t* planeVars)
{
    dynlight_t *dyn;
    DGLuint lightTex;
    float   diff, lightStrength, srcRadius;
    float   s[2], t[2];
    float   pos[3];
    subsectorinfo_t *ssInfo;

    pos[VX] = FIX2FLT(lum->thing->pos[VX]);
    pos[VY] = FIX2FLT(lum->thing->pos[VY]);
    pos[VZ] = FIX2FLT(lum->thing->pos[VZ]);

    // Center the Z.
    pos[VZ] += lum->center;
    srcRadius = lum->radius / 4;
    if(srcRadius == 0)
        srcRadius = 1;

    // Determine on which side the light is for all planes.
    ssInfo = SUBSECT_INFO(subsector);

    lightTex = 0;
    lightStrength = 0;

    if(ssInfo->plane[planeID].type == PLN_FLOOR)
    {
        if((lightTex = lum->floorTex) != 0)
        {
            if(pos[VZ] > planeVars->height)
                lightStrength = 1;
            else if(pos[VZ] > planeVars->height - srcRadius)
                lightStrength = 1 - (planeVars->height - pos[VZ]) / srcRadius;
        }
    }
    else
    {
        if((lightTex = lum->ceilTex) != 0)
        {
            if(pos[VZ] < planeVars->height)
                lightStrength = 1;
            else if(pos[VZ] < planeVars->height + srcRadius)
                lightStrength = 1 - (pos[VZ] - planeVars->height) / srcRadius;
        }
    }

    // Is there light in this direction? Is it strong enough?
    if(!lightTex || !lightStrength)
        return;

    // Check that the height difference is tolerable.
    if(ssInfo->plane[planeID].type == PLN_CEILING)
        diff = planeVars->height - pos[VZ];
    else
        diff = pos[VZ] - planeVars->height;

    // Clamp it.
    if(diff < 0)
        diff = 0;

    if(diff < lum->radius)
    {
        // Calculate dynlight position. It may still be outside
        // the bounding box the subsector.
        s[0] = -pos[VX] + lum->radius;
        t[0] = pos[VY] + lum->radius;
        s[1] = t[1] = 1.0f / (2 * lum->radius);

        // A dynamic light will be generated.
        dyn = DL_New(s, t);
        dyn->flags = 0;
        dyn->texture = lightTex;

        DL_ThingColor(lum, dyn->color, LUM_FACTOR(diff) * lightStrength);

        // Link to this plane's list.
        DL_SubSecLink(dyn, GET_SUBSECTOR_IDX(subsector), planeID);
    }
}

static boolean DL_LightSegIteratorFunc(lumobj_t *lum, subsector_t *ssec)
{
    int     j;
    seg_t  *seg;

    // The wall segments.
    for(j = 0, seg = &segs[ssec->firstline];
        j < ssec->linecount; ++j, seg++)
    {
        if(seg->linedef)    // "minisegs" have no linedefs.
            DL_ProcessWallSeg(lum, seg, ssec->sector);
    }

    // Is there a polyobj on board? Light it, too.
    if(ssec->poly)
        for(j = 0; j < ssec->poly->numsegs; ++j)
        {
            DL_ProcessWallSeg(lum, ssec->poly->segs[j], ssec->sector);
        }

    return true;
}

/**
 * Return the texture name of the decoration light map for the flat.
 * (Zero if no such texture exists.)
 */
#if 0 // Unused
DGLuint DL_GetFlatDecorLightMap(surface_t *surface)
{
    ded_decor_t *decor;

    if(R_IsSkySurface(surface) || !surface->isflat)
        return 0;

    decor = R_GetFlat(surface->texture)->decoration;
    return decor ? decor->pregen_lightmap : 0;
}
#endif

/**
 * Process dynamic lights for the specified subsector.
 */
void DL_ProcessSubsector(subsector_t *ssec)
{
    int     pln, j;
    seg_t  *seg;
    lumcontact_t *con;
    sector_t *sect = ssec->sector;
    subsectorinfo_t *ssInfo;
    planeitervars_t planeVars[NUM_PLANES];

    // First make sure we know which lumobjs are contacting us.
    DL_SpreadBlocks(ssec);

    // Check if lighting can be skipped for each plane.
    ssInfo = SUBSECT_INFO(ssec);
    for(pln = 0; pln < NUM_PLANES; ++pln)
    {
        planeVars[pln].height = SECT_PLANE_HEIGHT(sect, pln);
        planeVars[pln].isLit = (!R_IsSkySurface(&sect->planes[pln].surface));

        // View height might prevent us from seeing the light.
        if(ssInfo->plane[pln].type == PLN_FLOOR)
        {
            if(vy < planeVars[pln].height)
                planeVars[pln].isLit = false;
        }
        else
        {
            if(vy > planeVars[pln].height)
                planeVars[pln].isLit = false;
        }
    }

    // Process each lumobj contacting the subsector.
    for(con = subContacts[GET_SUBSECTOR_IDX(ssec)]; con; con = con->next)
    {
        lumobj_t *lum = con->lum;

        if(haloMode)
        {
            if(lum->thing->subsector == ssec)
                lum->flags |= LUMF_RENDERED;
        }

        // Process the planes
        for(pln = 0; pln < NUM_PLANES; ++pln)
        {
            if(planeVars[pln].isLit)
                DL_ProcessPlane(lum, ssec, pln, &planeVars[pln]);
        }

        // If the light has no texture for the 'sides', there's no point in
        // going through the wall segments.
        if(lum->tex)
            DL_LightSegIteratorFunc(con->lum, ssec);
    }

    // Check glowing planes.
    if(useWallGlow &&
       (sect->SP_floorglow || sect->SP_ceilglow))
    {
        // The wall segments.
        for(j = 0, seg = &segs[ssec->firstline]; j < ssec->linecount;
            ++j, seg++)
        {
            if(seg->linedef)    // "minisegs" have no linedefs.
                DL_ProcessWallGlow(seg, sect);
        }

        // Is there a polyobj on board? Light it, too.
        if(ssec->poly)
        {
            for(j = 0; j < ssec->poly->numsegs; ++j)
                DL_ProcessWallGlow(ssec->poly->segs[j], sect);
        }
    }
}

/**
 * Creates the dynlight links by removing everything and then linking
 * this frame's luminous objects.
 */
void DL_InitForNewFrame(void)
{
    sector_t *seciter;
    int     i;

    // Clear the dynlight lists, which are used to track the lights on
    // each surface of the map.
    DL_DeleteUsed();

    // The luminousList already contains lumobjs if there are any light
    // decorations in use.
    dlInited = true;

    for(i = 0; i < numsectors; ++i)
    {
        mobj_t *iter;

        seciter = SECTOR_PTR(i);
        for(iter = seciter->thinglist; iter; iter = iter->snext)
        {
            DL_AddLuminous(iter);
        }
    }

    // Link the luminous objects into the blockmap.
    DL_LinkLuminous();
}

/**
 * Calls func for all luminous objects within the specified range from (x,y).
 * 'subsector' is the subsector in which (x,y) resides.
 */
boolean DL_RadiusIterator(subsector_t *subsector, fixed_t x, fixed_t y,
                          fixed_t radius, boolean (*func) (lumobj_t *,
                                                           fixed_t))
{
    lumcontact_t *con;
    fixed_t dist;

    if(!subsector)
        return true;

    for(con = subContacts[GET_SUBSECTOR_IDX(subsector)]; con; con = con->next)
    {
        dist = P_ApproxDistance(con->lum->thing->pos[VX] - x,
                                con->lum->thing->pos[VY] - y);

        if(dist <= radius && !func(con->lum, dist))
            return false;
    }
    return true;
}

/**
 * In the situation where a subsector contains both dynamic lights and
 * a polyobj, the lights must be clipped more carefully.  Here we
 * check if the line of sight intersects any of the polyobj segs that
 * face the camera.
 */
void DL_ClipBySight(int ssecidx)
{
    subsector_t *ssec = SUBSECTOR_PTR(ssecidx);
    lumobj_t *lumi;
    seg_t *seg;
    int i;
    vec2_t v1, v2, eye, source;

    // Only checks the polyobj.
    if(ssec->poly == NULL) return;

    V2_Set(eye, vx, vz);

    for(lumi = dlSubLinks[ssecidx]; lumi; lumi = lumi->ssNext)
    {
        if(lumi->flags & LUMF_CLIPPED)
            continue;

        // We need to figure out if any of the polyobj's segments lies
        // between the viewpoint and the light source.
        for(i = 0; i < ssec->poly->numsegs; ++i)
        {
            seg = ssec->poly->segs[i];

            v1[VX] = FIX2FLT(seg->v1->x);
            v1[VY] = FIX2FLT(seg->v1->y);
            v2[VX] = FIX2FLT(seg->v2->x);
            v2[VY] = FIX2FLT(seg->v2->y);

            // Ignore segs facing the wrong way.
            if(!Rend_SegFacingDir(v1, v2))
                continue;

            V2_Set(source, FIX2FLT(lumi->thing->pos[VX]),
                   FIX2FLT(lumi->thing->pos[VY]));

            if(V2_Intercept2(source, eye, v1, v2, NULL, NULL, NULL))
            {
                lumi->flags |= LUMF_CLIPPED;
            }
        }
    }
}
