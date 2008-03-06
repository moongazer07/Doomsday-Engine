/**\file
 *\section License
 * License: GPL
 * Online License Link: http://www.gnu.org/licenses/gpl.html
 *
 *\author Copyright © 2003-2007 Jaakko Keränen <jaakko.keranen@iki.fi>
 *\author Copyright © 2005-2008 Daniel Swanson <danij@dengine.net>
 *\author Copyright © 1993-1996 by id Software, Inc.
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
 * p_spec.h: Implements special effects:
 *
 * Texture animation, height or lighting changes according to adjacent
 * sectors, respective utility functions, etc.
 *
 * Line Tag handling. Line and Sector triggers.
 *
 * Events are operations triggered by using, crossing,
 * or shooting special lines, or by timed thinkers.
 *
 *  2006/01/17 DJS - Recreated using jDoom's p_spec.h as a base.
 */

#ifndef __P_SPEC_H__
#define __P_SPEC_H__

#ifndef __JHERETIC__
#  error "Using jHeretic headers without __JHERETIC__"
#endif

#include "h_player.h"
#include "r_data.h"

extern int     *TerrainTypes;

//      Define values for map objects
#define MO_TELEPORTMAN          14

// at game start
void            P_InitPicAnims(void);
void            P_InitTerrainTypes(void);
void            P_InitLava(void);

// at map load
void            P_SpawnSpecials(void);
void            P_InitAmbientSound(void);
void            P_AddAmbientSfx(int sequence);

// every tic
void            P_UpdateSpecials(void);
void            P_AmbientSound(void);

// when needed
int             P_GetTerrainType(sector_t* sec, int plane);
int             P_FlatToTerrainType(int flatid);
boolean         P_ActivateLine(linedef_t *ld, mobj_t *mo, int side,
                               int activationType);

void            P_PlayerInSpecialSector(player_t *player);

void            P_PlayerInWindSector(player_t *player);

//
//  SPECIAL
//
int             EV_DoDonut(linedef_t *line);

//
//  P_LIGHTS
//
typedef struct {
    thinker_t       thinker;
    sector_t       *sector;
    int             count;
    float           maxLight;
    float           minLight;
    int             maxTime;
    int             minTime;
} lightflash_t;

typedef struct {
    thinker_t       thinker;
    sector_t       *sector;
    int             count;
    float           minLight;
    float           maxLight;
    int             darkTime;
    int             brightTime;
} strobe_t;

typedef struct {
    thinker_t       thinker;
    sector_t       *sector;
    float           minLight;
    float           maxLight;
    int             direction;
} glow_t;

#define GLOWSPEED       8
#define STROBEBRIGHT    5
#define FASTDARK        15
#define SLOWDARK        35

void            T_LightFlash(lightflash_t *flash);
void            P_SpawnLightFlash(sector_t *sector);

void            T_StrobeFlash(strobe_t *flash);
void            P_SpawnStrobeFlash(sector_t *sector, int fastOrSlow, int inSync);

void            EV_StartLightStrobing(linedef_t *line);
void            EV_TurnTagLightsOff(linedef_t *line);

void            EV_LightTurnOn(linedef_t *line, float bright);

void            T_Glow(glow_t *g);
void            P_SpawnGlowingLight(sector_t *sector);

//
// P_SWITCH
//
// This struct is used to provide byte offsets when reading a custom
// SWITCHES lump thus it must be packed and cannot be altered.
#pragma pack(1)
typedef struct {
    /* Do NOT change these members in any way */
    char            name1[9];
    char            name2[9];
    short           episode;
} switchlist_t;
#pragma pack()

typedef enum linesection_e{
    LS_MIDDLE,
    LS_BOTTOM,
    LS_TOP
} linesection_t;

typedef struct button_s {
    linedef_t         *line;
    linesection_t   section;
    int             texture;
    int             timer;
    mobj_t         *soundOrg;

    struct button_s *next;
} button_t;

 // 1 second, in ticks.
#define BUTTONTIME  35

extern button_t *buttonlist;
void            P_FreeButtons(void);

void            P_ChangeSwitchTexture(linedef_t *line, int useAgain);

void            P_InitSwitchList(void);

//
// P_PLATS
//
typedef enum {
    up,
    down,
    waiting,
    in_stasis
} plat_e;

typedef enum {
    perpetualRaise,
    downWaitUpStay,
    raiseAndChange,
    raiseToNearestAndChange
} plattype_e;

typedef struct {
    thinker_t       thinker;
    sector_t       *sector;
    float           speed;
    float           low;
    float           high;
    int             wait;
    int             count;
    plat_e          status;
    plat_e          oldStatus;
    boolean         crush;
    int             tag;
    plattype_e      type;

    struct platlist_s *list;
} plat_t;

typedef struct platlist_s {
  plat_t           *plat;
  struct platlist_s *next,**prev;
} platlist_t;

#define PLATWAIT        3
#define PLATSPEED       1

void            T_PlatRaise(plat_t *plat);

int             EV_DoPlat(linedef_t *line, plattype_e type, int amount);
boolean         EV_StopPlat(linedef_t *line);

void            P_AddActivePlat(plat_t *plat);
void            P_RemoveActivePlat(plat_t *plat);
void            P_RemoveAllActivePlats(void);
int             P_ActivateInStasisPlat(int tag);

//
// P_DOORS
//
typedef enum {
    normal,
    close30ThenOpen,
    close,
    open,
    raiseIn5Mins,
    blazeOpen
} vldoor_e;

typedef struct {
    thinker_t       thinker;
    vldoor_e        type;
    sector_t       *sector;
    float           topHeight;
    float           speed;

    // 1 = up, 0 = waiting at top, -1 = down
    int             direction;

    // tics to wait at the top
    int             topWait;
    // (keep in case a door going down is reset)
    // when it reaches 0, start going down
    int             topCountDown;
} vldoor_t;

#define VDOORSPEED  2
#define VDOORWAIT   150

boolean         EV_VerticalDoor(linedef_t *line, mobj_t *thing);
int             EV_DoDoor(linedef_t *line, vldoor_e type);
void            T_VerticalDoor(vldoor_t *door);
void            P_SpawnDoorCloseIn30(sector_t *sec);
void            P_SpawnDoorRaiseIn5Mins(sector_t *sec);

//
// P_CEILNG
//
typedef enum {
    lowerToFloor,
    raiseToHighest,
    lowerAndCrush,
    crushAndRaise,
    fastCrushAndRaise
} ceiling_e;

typedef struct {
    thinker_t       thinker;
    ceiling_e       type;
    sector_t       *sector;
    float           bottomHeight;
    float           topHeight;
    float           speed;
    boolean         crush;

    // 1 = up, 0 = waiting, -1 = down
    int             direction;

    // ID
    int             tag;
    int             oldDirection;

    struct ceilinglist_s *list;
} ceiling_t;

typedef struct ceilinglist_s {
  ceiling_t        *ceiling;
  struct ceilinglist_s *next,**prev;
} ceilinglist_t;

#define CEILSPEED       1
#define CEILWAIT        150

int             EV_DoCeiling(linedef_t *line, ceiling_e type);

void            T_MoveCeiling(ceiling_t *ceiling);
void            P_AddActiveCeiling(ceiling_t *c);
void            P_RemoveActiveCeiling(ceiling_t *c);
void            P_RemoveAllActiveCeilings(void);
int             EV_CeilingCrushStop(linedef_t *line);
int             P_ActivateInStasisCeiling(linedef_t *line);

//
// P_FLOOR
//
typedef enum {
    // lower floor to highest surrounding floor
    lowerFloor,

    // lower floor to lowest surrounding floor
    lowerFloorToLowest,

    // lower floor to highest surrounding floor VERY FAST
    turboLower,

    // raise floor to lowest surrounding CEILING
    raiseFloor,

    // raise floor to next highest surrounding floor
    raiseFloorToNearest,

    // raise floor to shortest height texture around it
    raiseToTexture,

    // lower floor to lowest surrounding floor
    //  and change floorpic
    lowerAndChange,

    raiseFloor24,
    raiseFloor24AndChange,
    raiseFloorCrush,
    donutRaise,
    raiseBuildStep
} floor_e;

typedef enum {
    build8,                        // slowly build by 8
    build16                        // slowly build by 16
} stair_e;

typedef struct {
    thinker_t       thinker;
    floor_e         type;
    boolean         crush;
    sector_t       *sector;
    int             direction;
    int             newSpecial;
    short           texture;
    float           floorDestHeight;
    float           speed;
} floormove_t;

#define FLOORSPEED  1

typedef enum {
    ok,
    crushed,
    pastdest
} result_e;

result_e        T_MovePlane(sector_t *sector, float speed, float dest,
                            int crush, int floorOrCeiling, int direction);

int             EV_BuildStairs(linedef_t *line, stair_e type);
int             EV_DoFloor(linedef_t *line, floor_e floortype);
void            T_MoveFloor(floormove_t *floor);

//
// P_TELEPT
//
#define         TELEFOGHEIGHTF      (32)
boolean         P_TeleportF(mobj_t *thing, float x, float y, angle_t angle);
boolean         EV_Teleport(linedef_t *line, int side, mobj_t *thing);
void            P_ArtiTele(player_t *player);

#endif
