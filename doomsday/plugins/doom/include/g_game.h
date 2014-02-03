/**\file g_game.h
 *\section License
 * License: GPL
 * Online License Link: http://www.gnu.org/licenses/gpl.html
 *
 *\author Copyright © 2003-2013 Jaakko Keränen <jaakko.keranen@iki.fi>
 *\author Copyright © 2005-2013 Daniel Swanson <danij@dengine.net>
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
 * Top-level (common) game routines jDoom - specific.
 */

#ifndef LIBJDOOM_G_GAME_H
#define LIBJDOOM_G_GAME_H

#ifndef __JDOOM__
#  error "Using jDoom headers without __JDOOM__"
#endif

#include "doomdef.h"
#include "d_event.h"
#include "d_player.h"
#include "gamerules.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int gaSaveGameSlot;
extern int gaLoadGameSlot;

extern player_t players[MAXPLAYERS];

extern dd_bool gameInProgress;
extern skillmode_t gameSkill;
extern uint gameEpisode;
extern uint gameMap;
extern uint gameMapEntryPoint;

extern uint nextMap; // If non zero this will be the next map.
extern dd_bool secretExit;
extern int totalKills, totalItems, totalSecret;
extern dd_bool deathmatch;
extern dd_bool respawnMonsters;
extern dd_bool paused;
extern dd_bool precache;
extern dd_bool customPal;
extern wbstartstruct_t wmInfo;
extern int bodyQueueSlot;
extern int mapStartTic;
extern dd_bool briefDisabled;

extern int gsvMapMusic;

void            G_Register(void);
void            G_CommonPreInit(void);
void            G_CommonPostInit(void);
void            G_CommonShutdown(void);

/**
 * Configure the given @a rules structure according to the @em current game rules.
 * @todo Refactor away.
 */
void G_GetGameRules(gamerules_t *rules);

void            R_InitRefresh(void);

void            G_PrintMapList(void);

void            G_DeferredPlayDemo(char* demo);

void            G_QuitGame(void);

/// @return  @c true = loading is presently possible.
dd_bool G_IsLoadGamePossible(void);

/**
 * To be called to schedule a load game-save action.
 * @param slot  Logical identifier of the save slot to use.
 * @return  @c true iff @a slot is in use and loading is presently possible.
 */
dd_bool G_LoadGame(int slot);

/// @return  @c true = saving is presently possible.
dd_bool G_IsSaveGamePossible(void);

/**
 * To be called to schedule a save game-save action.
 * @param slot  Logical identifier of the save slot to use.
 * @param name  New name for the game-save. Can be @c NULL in which case
 *      the name will not change if the slot has already been used.
 *      If an empty string a new name will be generated automatically.
 * @return  @c true iff @a slot is valid and saving is presently possible.
 */
dd_bool G_SaveGame2(int slot, const char* name);
dd_bool G_SaveGame(int slot);

void            G_StopDemo(void);

/**
 * Check if there is a finale before the map.
 * Returns true if a finale was found.
 */
int G_BriefingEnabled(Uri const *mapUri, ddfinale_t *fin);

/**
 * Check if there is a finale after the map.
 * Returns true if a finale was found.
 */
int G_DebriefingEnabled(Uri const *mapUri, ddfinale_t *fin);

void            G_DoReborn(int playernum);
void            G_PlayerReborn(int player);

/**
 * Called after intermission ends.
 */
void            G_IntermissionDone(void);

void            G_Ticker(timespan_t ticLength);

/// @return  @c true if the input event @a ev was eaten.
int G_PrivilegedResponder(event_t* ev);

/// @return  @c true if the input event @a ev was eaten.
int G_Responder(event_t* ev);

void            G_ScreenShot(void);

void            G_PrepareWIData(void);

void            G_QueueBody(mobj_t* body);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* LIBJDOOM_G_GAME_H */
