/** @file gamesession.cpp  Logical game session and saved session marshalling.
 *
 * @authors Copyright © 2003-2014 Jaakko Keränen <jaakko.keranen@iki.fi>
 * @authors Copyright © 2005-2014 Daniel Swanson <danij@dengine.net>
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

#include "gamesession.h"

#include <de/App>
#include <de/game/SavedSession>
#include <de/game/SavedSessionRepository>
#include <de/Time>
#include <de/ZipArchive>
#include "d_netsv.h"
#include "g_common.h"
#include "gamerules.h"
#include "hu_inventory.h"
#include "mapstatewriter.h"
#include "p_inventory.h"
#include "p_map.h"
#include "p_mapsetup.h"
#include "p_tick.h"
#include "p_saveg.h"
#include "p_saveio.h"
#include "p_sound.h"
#include "saveslots.h"

using namespace de;
using de::game::SavedSession;
using de::game::SessionMetadata;

namespace common {

static GameSession *singleton;

static String const internalSavePath = "/home/cache/internal.save";

DENG2_PIMPL(GameSession)
{
    bool inProgress; ///< @c true= session is in progress / internal.save exists.

    Instance(Public *i) : Base(i), inProgress(false)
    {
        singleton = i;
    }

    String userSavePath(String const &fileName)
    {
        return String("/home/savegames/%1/%2.save").arg(G_IdentityKey()).arg(fileName);
    }

    void cleanupInternalSave()
    {
        // Ensure the internal save folder exists.
        App::fileSystem().makeFolder(internalSavePath.fileNamePath());

        // Ensure that any pre-existing internal save is destroyed.
        // This may happen if the game was not shutdown properly in the event of a crash.
        /// @todo It may be possible to recover this session if it was written successfully
        /// before the fatal error occurred.
        deleteSaved(internalSavePath);
    }

    void deleteSaved(String const &path)
    {
        if(File *existing = App::rootFolder().tryLocateFile(path))
        {
            self.saveIndex().remove(existing->path());
            // We need write access to remove.
            existing->setMode(File::Write);
            App::rootFolder().removeFile(existing->path());
        }
    }

    void copySaved(String const &destPath, String const &sourcePath)
    {
        if(!destPath.compareWithoutCase(sourcePath)) return;

        LOG_AS("GameSession");

        deleteSaved(destPath);

        SavedSession const &sourceSession = DENG2_APP->rootFolder().locate<SavedSession>(sourcePath);

        // Copy the .save package.
        File &save = DENG2_APP->rootFolder().replaceFile(destPath);
        de::Writer(save) << sourceSession.archive();
        save.setMode(File::ReadOnly);
        LOG_RES_XVERBOSE("Wrote ") << save.description();

        // We can now reinterpret and populate the contents of the archive.
        File *updated = save.reinterpret();
        updated->as<Folder>().populate();

        SavedSession &session = updated->as<SavedSession>();
        session.cacheMetadata(sourceSession.metadata()); // Avoid immediately opening the .save package.
        self.saveIndex().add(session);

        // Announce the copy if both are user savegame paths.
        if(!sourcePath.beginsWith(internalSavePath.fileNamePath()) &&
           !destPath.beginsWith(internalSavePath.fileNamePath()))
        {
            LOG_MSG("Copied savegame \"%s\" to \"%s\"") << sourcePath << destPath;
        }
    }

    static String composeSaveInfo(SessionMetadata const &metadata)
    {
        String info;
        QTextStream os(&info);
        os.setCodec("UTF-8");

        // Write header and misc info.
        Time now;
        os <<   "# Doomsday Engine saved game session package.\n#"
           << "\n# Generator: GameSession (libcommon)"
           << "\n# Generation Date: " + now.asDateTime().toString(Qt::SystemLocaleShortDate);

        // Write metadata.
        os << "\n\n" + metadata.asTextWithInfoSyntax() + "\n";

        return info;
    }

    Block serializeCurrentMapState()
    {
        Block data;
        SV_OpenFileForWrite(data);
        writer_s *writer = SV_NewWriter();
        MapStateWriter().write(writer);
        Writer_Delete(writer);
        SV_CloseFile();
        return data;
    }

    struct saveworker_params_t
    {
        Instance *inst;
        String userDescription;
        String slotId;
    };

    /**
     * Serialize game state to a new .save package in the repository.
     *
     * @return  Non-zero iff the game state was saved successfully.
     */
    static int saveWorker(void *context)
    {
        saveworker_params_t const &p = *static_cast<saveworker_params_t *>(context);

        dd_bool didSave = false;
        try
        {
            SaveSlot &destSlot = G_SaveSlots()[p.slotId];

            LOG_AS("GameSession");
            LOG_RES_VERBOSE("Serializing to \"%s\"...") << internalSavePath;

            SessionMetadata metadata;
            G_ApplyCurrentSessionMetadata(metadata);

            // Apply the given user description.
            if(!p.userDescription.isEmpty())
            {
                metadata.set("userDescription", p.userDescription);
            }
            else // We'll generate a suitable description automatically.
            {
                metadata.set("userDescription", G_DefaultSavedSessionUserDescription(destSlot.id()));
            }

            // In networked games the server tells the clients to save also.
            NetSv_SaveGame(metadata.geti("sessionId"));

            // Serialize the game state to a new saved session.
            ZipArchive arch;
            arch.add("Info", composeSaveInfo(metadata).toUtf8());

#if __JHEXEN__
            de::Writer(arch.entryBlock("ACScriptState")).withHeader()
                    << Game_ACScriptInterpreter().serializeWorldState();
#endif

            arch.add(String("maps") / Str_Text(Uri_Compose(gameMapUri)) + "State",
                     p.inst->serializeCurrentMapState());

            p.inst->self.saveIndex().remove(internalSavePath);

            Folder &saveFolder = DENG2_APP->rootFolder().locate<Folder>(internalSavePath.fileNamePath());
            if(SavedSession *existing = saveFolder.tryLocate<SavedSession>(internalSavePath.fileName()))
            {
                existing->setMode(File::Write);
            }
            File &save = saveFolder.replaceFile(internalSavePath.fileName());
            de::Writer(save) << arch;
            save.setMode(File::ReadOnly);
            LOG_RES_XVERBOSE("Wrote ") << save.description();

            // We can now reinterpret and populate the contents of the archive.
            File *updated = save.reinterpret();
            updated->as<Folder>().populate();

            SavedSession &session = updated->as<SavedSession>();
            session.cacheMetadata(metadata); // Avoid immediately reopening the .save package.
            p.inst->self.saveIndex().add(session);

            // Copy the internal saved session to the destination slot.
            p.inst->copySaved(destSlot.savePath(), internalSavePath);

            // Make note of the last used save slot.
            Con_SetInteger2("game-save-last-slot", destSlot.id().toInt(), SVF_WRITE_OVERRIDE);

            didSave = true;
        }
        catch(Error const &er)
        {
            LOG_RES_WARNING("Error writing to save slot '%s':\n")
                    << p.slotId << er.asText();
        }

        BusyMode_WorkerEnd();
        return didSave;
    }

    void setMap(Uri const &mapUri)
    {
        DENG2_ASSERT(inProgress);

        Uri_Copy(::gameMapUri, &mapUri);
        ::gameEpisode     = G_EpisodeNumberFor(::gameMapUri);
        ::gameMap         = G_MapNumberFor(::gameMapUri);

        // Ensure that the episode and map numbers are good.
        if(!G_ValidateMap(&::gameEpisode, &::gameMap))
        {
            Uri *validMapUri = G_ComposeMapUri(::gameEpisode, ::gameMap);
            Uri_Copy(::gameMapUri, validMapUri);
            ::gameEpisode = G_EpisodeNumberFor(::gameMapUri);
            ::gameMap     = G_MapNumberFor(::gameMapUri);
            Uri_Delete(validMapUri);
        }

        // Update game status cvars:
        ::gsvMap     = (unsigned)::gameMap;
        ::gsvEpisode = (unsigned)::gameEpisode;
    }

    /**
     * Reload the @em current map.
     *
     * @param revisit  @c true= load progress in this map from a previous visit in the current
     * game session. If no saved progress exists then the map will be in the default state.
     */
    void changeMap(bool revisit = false)
    {
        DENG2_ASSERT(inProgress);

        Pause_End();

        // Close open HUDs.
        for(uint i = 0; i < MAXPLAYERS; ++i)
        {
            player_t *plr = players + i;
            if(plr->plr->inGame)
            {
                ST_AutomapOpen(i, false, true);
#if __JHERETIC__ || __JHEXEN__
                Hu_InventoryOpen(i, false);
#endif
            }
        }

        // Delete raw images to conserve texture memory.
        DD_Executef(true, "texreset raw");

        // Are we playing a briefing?
        char const *briefing = G_InFineBriefing(gameMapUri);
        if(!briefing)
        {
            // Restart the map music.
#if __JHEXEN__
            /**
             * @note Kludge: Due to the way music is managed with Hexen, unless we explicitly stop
             * the current playing track the engine will not change tracks. This is due to the use
             * of the runtime-updated "currentmap" definition (the engine thinks music has not changed
             * because the current Music definition is the same).
             *
             * It only worked previously was because the waiting-for-map-load song was started prior
             * to map loading.
             *
             * @todo Rethink the Music definition stuff with regard to Hexen. Why not create definitions
             * during startup by parsing MAPINFO?
             */
            S_StopMusic();
            //S_StartMusic("chess", true); // Waiting-for-map-load song
#endif

            S_MapMusic(gameMapUri);
            S_PauseMusic(true);
        }

        // If we're the server, let clients know the map will change.
        NetSv_UpdateGameConfigDescription();
        NetSv_SendGameState(GSF_CHANGE_MAP, DDSP_ALL_PLAYERS);

        P_SetupMap(gameMapUri);

        if(revisit)
        {
            // We've been here before; deserialize this map's save state.
#if __JHEXEN__
            targetPlayerAddrs = 0; // player mobj redirection...
#endif

            SavedSession const &session = DENG2_APP->rootFolder().locate<SavedSession>(internalSavePath);
            String const mapUriStr = Str_Text(Uri_Compose(gameMapUri));
            SV_MapStateReader(session, mapUriStr)->read(mapUriStr);
        }

        if(!G_StartFinale(briefing, 0, FIMODE_BEFORE, 0))
        {
            // No briefing; begin the map.
            HU_WakeWidgets(-1/* all players */);
            G_BeginMap();
        }

        Z_CheckHeap();
    }

#if __JHEXEN__
    struct playerbackup_t
    {
        player_t player;
        uint numInventoryItems[NUM_INVENTORYITEM_TYPES];
        inventoryitemtype_t readyItem;
    };

    void backupPlayersInHub(playerbackup_t playerBackup[MAXPLAYERS])
    {
        DENG2_ASSERT(playerBackup != 0);

        for(int i = 0; i < MAXPLAYERS; ++i)
        {
            playerbackup_t *pb = playerBackup + i;
            player_t *plr      = players + i;

            std::memcpy(&pb->player, plr, sizeof(player_t));

            // Make a copy of the inventory states also.
            for(int k = 0; k < NUM_INVENTORYITEM_TYPES; ++k)
            {
                pb->numInventoryItems[k] = P_InventoryCount(i, inventoryitemtype_t(k));
            }
            pb->readyItem = P_InventoryReadyItem(i);
        }
    }

    /**
     * @param playerBackup  Player state backup.
     * @param mapEntrance   Logical entry point number used to enter the map.
     */
    void restorePlayersInHub(playerbackup_t playerBackup[MAXPLAYERS], uint mapEntrance)
    {
        DENG2_ASSERT(playerBackup != 0);

        mobj_t *targetPlayerMobj = 0;

        for(int i = 0; i < MAXPLAYERS; ++i)
        {
            playerbackup_t *pb = playerBackup + i;
            player_t *plr      = players + i;
            ddplayer_t *ddplr  = plr->plr;
            int oldKeys = 0, oldPieces = 0;
            dd_bool oldWeaponOwned[NUM_WEAPON_TYPES];

            if(!ddplr->inGame) continue;

            std::memcpy(plr, &pb->player, sizeof(player_t));

            for(int k = 0; k < NUM_INVENTORYITEM_TYPES; ++k)
            {
                // Don't give back the wings of wrath if reborn.
                if(k == IIT_FLY && plr->playerState == PST_REBORN)
                    continue;

                for(uint m = 0; m < pb->numInventoryItems[k]; ++m)
                {
                    P_InventoryGive(i, inventoryitemtype_t(k), true);
                }
            }
            P_InventorySetReadyItem(i, pb->readyItem);

            ST_LogEmpty(i);
            plr->attacker = 0;
            plr->poisoner = 0;

            if(IS_NETGAME || G_Rules().deathmatch)
            {
                // In a network game, force all players to be alive
                if(plr->playerState == PST_DEAD)
                {
                    plr->playerState = PST_REBORN;
                }

                if(!G_Rules().deathmatch)
                {
                    // Cooperative net-play; retain keys and weapons.
                    oldKeys   = plr->keys;
                    oldPieces = plr->pieces;

                    for(int k = 0; k < NUM_WEAPON_TYPES; ++k)
                    {
                        oldWeaponOwned[k] = plr->weapons[k].owned;
                    }
                }
            }

            bool wasReborn = (plr->playerState == PST_REBORN);

            if(G_Rules().deathmatch)
            {
                de::zap(plr->frags);
                ddplr->mo = 0;
                G_DeathMatchSpawnPlayer(i);
            }
            else
            {
                if(playerstart_t const *start = P_GetPlayerStart(mapEntrance, i, false))
                {
                    mapspot_t const *spot = &mapSpots[start->spot];
                    P_SpawnPlayer(i, cfg.playerClass[i], spot->origin[VX],
                                  spot->origin[VY], spot->origin[VZ], spot->angle,
                                  spot->flags, false, true);
                }
                else
                {
                    P_SpawnPlayer(i, cfg.playerClass[i], 0, 0, 0, 0,
                                  MSF_Z_FLOOR, true, true);
                }
            }

            if(wasReborn && IS_NETGAME && !G_Rules().deathmatch)
            {
                int bestWeapon = 0;

                // Restore keys and weapons when reborn in co-op.
                plr->keys   = oldKeys;
                plr->pieces = oldPieces;

                for(int k = 0; k < NUM_WEAPON_TYPES; ++k)
                {
                    if(oldWeaponOwned[k])
                    {
                        bestWeapon = k;
                        plr->weapons[k].owned = true;
                    }
                }

                plr->ammo[AT_BLUEMANA].owned = 25; /// @todo values.ded
                plr->ammo[AT_GREENMANA].owned = 25; /// @todo values.ded

                // Bring up the best weapon.
                if(bestWeapon)
                {
                    plr->pendingWeapon = weapontype_t(bestWeapon);
                }
            }
        }

        for(int i = 0; i < MAXPLAYERS; ++i)
        {
            player_t *plr     = players + i;
            ddplayer_t *ddplr = plr->plr;

            if(!ddplr->inGame) continue;

            if(!targetPlayerMobj)
            {
                targetPlayerMobj = ddplr->mo;
            }
        }

        /// @todo Redirect anything targeting a player mobj
        /// FIXME! This only supports single player games!!
        if(targetPlayerAddrs)
        {
            for(targetplraddress_t *p = targetPlayerAddrs; p; p = p->next)
            {
                *(p->address) = targetPlayerMobj;
            }

            SV_ClearTargetPlayers();

            /*
             * When XG is available in Hexen, call this after updating target player
             * references (after a load) - ds
            // The activator mobjs must be set.
            XL_UpdateActivators();
            */
        }

        // Destroy all things touching players.
        P_TelefragMobjsTouchingPlayers();
    }
#endif // __JHEXEN__
};

GameSession::GameSession() : d(new Instance(this))
{}

GameSession::~GameSession()
{
    LOG_AS("~GameSession");
    d.reset();
    singleton = 0;
}

GameSession &GameSession::gameSession()
{
    DENG2_ASSERT(singleton != 0);
    return *singleton;
}

bool GameSession::hasBegun() const
{
    return d->inProgress;
}

bool GameSession::loadingPossible() const
{
    return !(IS_CLIENT && !Get(DD_PLAYBACK));
}

bool GameSession::savingPossible() const
{
    if(IS_CLIENT || Get(DD_PLAYBACK)) return false;

    if(!hasBegun()) return false;
    if(GS_MAP != G_GameState()) return false;

    /// @todo fixme: what about splitscreen!
    player_t *player = &players[CONSOLEPLAYER];
    if(PST_DEAD == player->playerState) return false;

    return true;
}

void GameSession::end()
{
    if(!hasBegun()) return;

#if __JHEXEN__
    Game_ACScriptInterpreter().reset();
#endif
    d->deleteSaved(internalSavePath);

    d->inProgress = false;
    LOG_MSG("Game ended");
}

void GameSession::endAndBeginTitle()
{
    end();

    if(char const *script = G_InFine("title"))
    {
        G_StartFinale(script, FF_LOCAL, FIMODE_NORMAL, "title");
        return;
    }
    /// @throw Error A title script must always be defined.
    throw Error("GameSession::endAndBeginTitle", "An InFine 'title' script must be defined");
}

void GameSession::begin(Uri const &mapUri, uint mapEntrance, GameRuleset const &rules)
{
    if(hasBegun())
    {
        /// @throw InProgressError Cannot begin a new session before the current session has ended.
        throw InProgressError("GameSession::begin", "The game session has already begun");
    }

    LOG_MSG("Game begins...");

    // Perform necessary prep.
    d->cleanupInternalSave();

    G_StopDemo();

    // Close the menu if open.
    Hu_MenuCommand(MCMD_CLOSEFAST);

    // If there are any InFine scripts running, they must be stopped.
    FI_StackClear();

    if(!IS_CLIENT)
    {
        for(uint i = 0; i < MAXPLAYERS; ++i)
        {
            player_t *plr = players + i;
            if(plr->plr->inGame)
            {
                // Force players to be initialized upon first map load.
                plr->playerState = PST_REBORN;
#if __JHEXEN__
                plr->worldTimer  = 0;
#else
                plr->didSecret   = false;
#endif
            }
        }
    }

    M_ResetRandom();

    G_ApplyNewGameRules(rules);
    d->inProgress = true;
    d->setMap(mapUri);
    ::gameMapEntrance = mapEntrance;

    d->changeMap();

    // Serialize the game state to the internal saved session.
    LOG_AS("GameSession");
    LOG_RES_VERBOSE("Serializing to \"%s\"...") << internalSavePath;

    SessionMetadata metadata;
    G_ApplyCurrentSessionMetadata(metadata);

    ZipArchive arch;
    arch.add("Info", Instance::composeSaveInfo(metadata).toUtf8());

#if __JHEXEN__
    de::Writer(arch.entryBlock("ACScriptState")).withHeader()
            << Game_ACScriptInterpreter().serializeWorldState();
#endif

    arch.add(String("maps") / Str_Text(Uri_Compose(gameMapUri)) + "State",
             d->serializeCurrentMapState());

    saveIndex().remove(internalSavePath);

    Folder &saveFolder        = DENG2_APP->rootFolder().locate<Folder>(internalSavePath.fileNamePath());
    String const saveFileName = internalSavePath.fileName();
    if(SavedSession *existing = saveFolder.tryLocate<SavedSession>(saveFileName))
    {
        existing->setMode(File::Write);
    }
    File &save = saveFolder.replaceFile(saveFileName);
    de::Writer(save) << arch;
    save.setMode(File::ReadOnly);
    LOG_RES_XVERBOSE("Wrote ") << save.description();

    // We can now reinterpret and populate the contents of the archive.
    File *updated = save.reinterpret();
    updated->as<Folder>().populate();

    SavedSession &session = updated->as<SavedSession>();
    session.cacheMetadata(metadata); // Avoid immediately reopening the .save package.
    saveIndex().add(session);

    // In a non-network, make a copy of the internal save to the autosave slot.
    /*if(!IS_NETGAME)
    {
        d->copySaved(G_SaveSlots()["auto"].savePath(), session.path());
    }*/
}

void GameSession::reloadMap()//bool revisit)
{
    if(!hasBegun())
    {
        /// @throw InProgressError Cannot load a map unless the game is in progress.
        throw InProgressError("GameSession::reloadMap", "No game session is in progress");
    }

    if(G_Rules().deathmatch)
    {
        // Restart the session entirely.
        end();
        begin(*::gameMapUri, ::gameMapEntrance, G_Rules());
    }
    else
    {
        load("");
    }
}

void GameSession::leaveMap()
{
    if(!hasBegun())
    {
        /// @throw InProgressError Cannot leave a map unless the game is in progress.
        throw InProgressError("GameSession::leaveMap", "No game session is in progress");
    }

    //LOG_AS("GameSession");

    // If there are any InFine scripts running, they must be stopped.
    FI_StackClear();

    // Ensure that the episode and map indices are good.
    G_ValidateMap(&gameEpisode, &nextMap);
    Uri const *nextMapUri = G_ComposeMapUri(gameEpisode, nextMap);

#if __JHEXEN__
    // Take a copy of the player objects (they will be cleared in the process
    // of calling P_SetupMap() and we need to restore them after).
    Instance::playerbackup_t playerBackup[MAXPLAYERS];
    d->backupPlayersInHub(playerBackup);

    // Disable class randomization (all players must spawn as their existing class).
    byte oldRandomClassesRule = G_Rules().randomClasses;
    G_Rules().randomClasses = false;
#endif

    // Are we saving progress?
    SavedSession *savedSession = 0;
    if(!G_Rules().deathmatch) // Never save in deathmatch.
    {
        savedSession = &DENG2_APP->rootFolder().locate<SavedSession>(internalSavePath);

        savedSession->setMode(File::Write);

        Folder &mapsFolder = savedSession->locate<Folder>("maps");
        mapsFolder.setMode(File::Write);

        // Are we entering a new hub?
#if __JHEXEN__
        if(P_MapInfo(0/*current map*/)->hub != P_MapInfo(nextMapUri)->hub)
#endif
        {
            //qDebug() << "Clearing all map states";
            // Clear all saved map states in the old hub.
            Folder::Contents contents = mapsFolder.contents();
            DENG2_FOR_EACH_CONST(Folder::Contents, i, contents)
            {
                mapsFolder.removeFile(i->first);
            }
        }
#if __JHEXEN__
        else
        {
            //qDebug() << "Updating current map state";
            // Update the saved state for the current map.
            if(File *existing = mapsFolder.tryLocateFile(String(Str_Text(Uri_Compose(gameMapUri))) + "State"))
            {
                qDebug() << "Exisiting" << (String(Str_Text(Uri_Compose(gameMapUri))) + "State") << "now Write";
                existing->setMode(File::Write);
            }
            File &outFile = mapsFolder.replaceFile(String(Str_Text(Uri_Compose(gameMapUri))) + "State");
            Block mapStateData;
            SV_OpenFileForWrite(mapStateData);
            writer_s *writer = SV_NewWriter();
            MapStateWriter().write(writer);
            outFile << mapStateData;
            outFile.setMode(File::ReadOnly);
            Writer_Delete(writer);
            SV_CloseFile();
        }
#endif

        mapsFolder.setMode(File::ReadOnly);
        savedSession->setMode(File::ReadOnly);
        savedSession->populate();
    }

#if __JHEXEN__
    /// @todo Necessary?
    if(!IS_CLIENT)
    {
        // Force players to be initialized upon first map load.
        for(uint i = 0; i < MAXPLAYERS; ++i)
        {
            player_t *plr = players + i;
            if(plr->plr->inGame)
            {
                plr->playerState = PST_REBORN;
                plr->worldTimer = 0;
            }
        }
    }
    //<- todo end.

    // In Hexen the RNG is re-seeded each time the map changes.
    M_ResetRandom();
#endif

    // Change the current map.
    d->setMap(*nextMapUri);
    ::gameMapEntrance = ::nextMapEntrance;

    Uri_Delete(const_cast<Uri *>(nextMapUri)); nextMapUri = 0;

    // Are we revisiting a previous map?
    bool revisit = savedSession && savedSession->hasState(String("maps") / Str_Text(Uri_Compose(gameMapUri)));
    //qDebug() << "Revisit:" << revisit;

    // We don't want to see a briefing if we've already visited this map.
    if(revisit)
    {
        briefDisabled = true;
    }
    d->changeMap(revisit);

    // On exit logic:
#if __JHEXEN__
    if(!revisit)
    {
        // First visit; destroy all freshly spawned players (??).
        P_RemoveAllPlayerMobjs();
    }

    d->restorePlayersInHub(playerBackup, nextMapEntrance);

    // Restore the random class rule.
    G_Rules().randomClasses = oldRandomClassesRule;

    // Launch waiting scripts.
    Game_ACScriptInterpreter_RunDeferredTasks(gameMapUri);
#endif

    if(savedSession && !revisit)
    {
        //qDebug() << "Writing new map state" << Str_Text(Uri_Compose(gameMapUri));

        savedSession->setMode(File::Write);

        SessionMetadata metadata;
        G_ApplyCurrentSessionMetadata(metadata);
        /// @todo Use the existing sessionId?
        //metadata.set("sessionId", savedSession->metadata().geti("sessionId"));
        /// @todo Update the description if it was autogenerated.
        metadata.set("userDescription", savedSession->metadata().gets("userDescription"));
        savedSession->replaceFile("Info") << Instance::composeSaveInfo(metadata).toUtf8();

#if __JHEXEN__
        // Save the world-state of the ACScript interpreter.
        de::Writer(savedSession->replaceFile("ACScriptState")).withHeader()
                << Game_ACScriptInterpreter().serializeWorldState();
#endif

        // Save the state of the current map.
        Folder &mapsFolder = savedSession->locate<Folder>("maps");
        mapsFolder.setMode(File::Write);

        if(File *existing = mapsFolder.tryLocateFile(String(Str_Text(Uri_Compose(gameMapUri))) + "State"))
        {
            existing->setMode(File::Write);
        }
        File &outFile = mapsFolder.replaceFile(String(Str_Text(Uri_Compose(gameMapUri))) + "State");
        Block mapStateData;
        SV_OpenFileForWrite(mapStateData);
        writer_s *writer = SV_NewWriter();
        MapStateWriter().write(writer);
        outFile << mapStateData;
        outFile.setMode(File::ReadOnly);
        Writer_Delete(writer);
        SV_CloseFile();

        mapsFolder.setMode(File::ReadOnly);
        savedSession->setMode(File::ReadOnly);
        savedSession->populate();

        savedSession->cacheMetadata(metadata); // Avoid immediately reopening the .save package.
    }

    // In a non-network, make a copy of the internal save to the autosave slot.
    /*if(!IS_NETGAME && savedSession)
    {
        d->copySaved(G_SaveSlots()["auto"].savePath(), savedSession->path());
    }*/
}

void GameSession::save(String const &slotId, String const &userDescription)
{
    String const sessionPath = G_SaveSlots()[slotId].savePath();

    if(!hasBegun())
    {
        /// @throw InProgressError Cannot save a map unless the game is in progress.
        throw InProgressError("GameSession::save", "No game session is in progress");
    }

    LOG_MSG("Saving game to \"%s\"...") << sessionPath;

    Instance::saveworker_params_t parm;
    parm.inst            = d;
    parm.userDescription = userDescription;
    parm.slotId          = slotId;

    bool didSave = (BusyMode_RunNewTaskWithName(BUSYF_ACTIVITY | (verbose? BUSYF_CONSOLE_OUTPUT : 0),
                                                Instance::saveWorker, &parm, "Saving game...") != 0);
    if(didSave)
    {
        P_SetMessage(&players[CONSOLEPLAYER], 0, TXT_GAMESAVED);

        // Notify the engine that the game was saved.
        /// @todo After the engine has the primary responsibility of saving the game, this
        /// notification is unnecessary.
        Plug_Notify(DD_NOTIFY_GAME_SAVED, NULL);
    }
}

void GameSession::load(String const &slotId)
{
    String const savePath         = slotId.isEmpty()? internalSavePath : G_SaveSlots()[slotId].savePath();
/*#if __JHEXEN__
    bool const mustCopyToAutoSlot = (slotId.compareWithoutCase("auto") && !IS_NETGAME);
#endif*/

    ::briefDisabled = true;

    LOG_MSG("Loading game from \"%s\"...") << savePath;

    G_StopDemo();
    Hu_MenuCommand(MCMD_CLOSEFAST);
    FI_StackClear(); // Stop any running InFine scripts.

    M_ResetRandom();
    if(!IS_CLIENT)
    {
        for(int i = 0; i < MAXPLAYERS; ++i)
        {
            player_t *plr = players + i;
            if(plr->plr->inGame)
            {
                // Force players to be initialized upon first map load.
                plr->playerState = PST_REBORN;
#if __JHEXEN__
                plr->worldTimer = 0;
#else
                plr->didSecret = false;
#endif
            }
        }
    }

    d->inProgress = false;

    if(!slotId.isEmpty())
    {
        // Perform necessary prep.
        d->cleanupInternalSave();

        // Copy the save for the specified slot to the internal savegame.
        d->copySaved(internalSavePath, savePath);
    }

    /*
     * SavedSession deserialization begins.
     */
    SavedSession const &session     = DENG2_APP->rootFolder().locate<SavedSession>(internalSavePath);
    SessionMetadata const &metadata = session.metadata();

    // Ensure a complete game ruleset is available.
    GameRuleset *rules;
    try
    {
        rules = GameRuleset::fromRecord(metadata.subrecord("gameRules"));
    }
    catch(Record::NotFoundError const &)
    {
        // The game rules are incomplete. Likely because they were missing from a savegame that
        // was converted from a vanilla format (in which most of these values are omitted).
        // Therefore we must assume the user has correctly configured the session accordingly.
        LOG_WARNING("Using current game rules as basis for loading savegame \"%s\"."
                    " (The original save format omits this information).")
                << session.path();

        // Use the current rules as our basis.
        rules = GameRuleset::fromRecord(metadata.subrecord("gameRules"), &G_Rules());
    }
    G_ApplyNewGameRules(*rules);
    delete rules; rules = 0;

#if __JHEXEN__
    // Deserialize the world ACS state.
    if(File const *file = session.tryLocateStateFile("ACScript"))
    {
        Game_ACScriptInterpreter().readWorldState(de::Reader(*file).withHeader());
    }
#endif

    d->inProgress = true;

    Uri *mapUri = Uri_NewWithPath2(metadata.gets("mapUri").toUtf8().constData(), RC_NULL);
    d->setMap(*mapUri);
    Uri_Delete(mapUri); mapUri = 0;
    //::gameMapEntrance = mapEntrance; // not saved??

    d->changeMap();
#if !__JHEXEN__
    ::mapTime = metadata.geti("mapTime");
#endif

    String mapUriStr = Str_Text(Uri_Resolved(::gameMapUri));
    SV_MapStateReader(session, mapUriStr)->read(mapUriStr);

/*#if __JHEXEN__
    if(mustCopyToAutoSlot)
    {
        d->copySaved(G_SaveSlots()["auto"].savePath(), internalSavePath);
    }
#endif*/

    if(!slotId.isEmpty())
    {
        P_SetMessage(&players[CONSOLEPLAYER], 0, "Game loaded");
    }
}

void GameSession::deleteSaved(String const &saveName)
{
    d->deleteSaved(d->userSavePath(saveName));
}

void GameSession::copySaved(String const &destName, String const &sourceName)
{
    d->copySaved(d->userSavePath(destName), d->userSavePath(sourceName));
}

game::SavedSessionRepository &GameSession::saveIndex() const
{
    return *static_cast<game::SavedSessionRepository *>(DD_SavedSessionRepository());
}

} // namespace common
