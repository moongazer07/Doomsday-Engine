/** @file multiplayerservermenuwidget.cpp
 *
 * @authors Copyright (c) 2016 Jaakko Keränen <jaakko.keranen@iki.fi>
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
 * General Public License along with this program; if not, see:
 * http://www.gnu.org/licenses</small>
 */

#include "ui/widgets/multiplayerservermenuwidget.h"
#include "ui/home/multiplayerpanelbuttonwidget.h"
#include "ui/widgets/homemenuwidget.h"
#include "network/serverlink.h"
#include "clientapp.h"

#include <doomsday/Games>
#include <de/MenuWidget>
#include <de/Address>

using namespace de;

DENG2_PIMPL(MultiplayerServerMenuWidget)
, DENG2_OBSERVES(DoomsdayApp, GameChange)
, DENG2_OBSERVES(Games, Readiness)
, DENG2_OBSERVES(ServerLink, DiscoveryUpdate)
, DENG2_OBSERVES(MultiplayerPanelButtonWidget, AboutToJoin)
, public ChildWidgetOrganizer::IWidgetFactory
{
    static ServerLink &link() { return ClientApp::serverLink(); }
    static String hostId(serverinfo_t const &sv) {
        return String("%1:%2").arg(sv.address).arg(sv.port);
    }

    /**
     * Data item with information about a found server.
     */
    class ServerListItem : public ui::Item
    {
    public:
        ServerListItem(serverinfo_t const &serverInfo, bool isLocal)
            : _lan(isLocal)
        {
            setData(hostId(serverInfo));
            _info = serverInfo;
        }

        bool isLocal() const
        {
            return _lan;
        }

        serverinfo_t const &info() const
        {
            return _info;
        }

        void setInfo(serverinfo_t const &serverInfo)
        {
            _info = serverInfo;
            notifyChange();
        }

        String title() const
        {
            return _info.name;
        }

        String gameId() const
        {
            return _info.gameIdentityKey;
        }

    private:
        serverinfo_t _info;
        bool _lan;
    };

    DiscoveryMode         mode = NoDiscovery;
    ServerLink::FoundMask mask = ServerLink::Any;

    Impl(Public *i) : Base(i)
    {
        DoomsdayApp::app().audienceForGameChange()  += this;
        DoomsdayApp::games().audienceForReadiness() += this;
        link().audienceForDiscoveryUpdate += this;

        self.organizer().setWidgetFactory(*this);
    }

    void linkDiscoveryUpdate(ServerLink const &link) override
    {
        bool changed = false;

        ui::Data &items = self.items();

        // Remove obsolete entries.
        for (ui::Data::Pos idx = 0; idx < items.size(); ++idx)
        {
            String const id = items.at(idx).data().toString();
            if (!link.isFound(Address::parse(id), mask))
            {
                items.remove(idx--);
                changed = true;
            }
        }

        // Add new entries and update existing ones.
        foreach (Address const &host, link.foundServers(mask))
        {
            serverinfo_t info;
            if (!link.foundServerInfo(host, &info, mask)) continue;

            ui::Data::Pos found = items.findData(hostId(info));
            if (found == ui::Data::InvalidPos)
            {
                // Needs to be added.
                items.append(new ServerListItem(info,
                        link.isServerOnLocalNetwork(Address::parse(hostId(info)))));
                changed = true;
            }
            else
            {
                // Update the info.
                items.at(found).as<ServerListItem>().setInfo(info);
            }
        }

        if (changed)
        {
            items.sort([] (ui::Item const &a, ui::Item const &b)
            {
                auto const &first  = a.as<ServerListItem>();
                auto const &second = b.as<ServerListItem>();

                // LAN games shown first.
                if (first.isLocal() == second.isLocal())
                {
                    // Sort by number of players.
                    if (first.info().numPlayers == second.info().numPlayers)
                    {
                        // Finally, by game ID.
                        int cmp = qstrcmp(first .info().gameIdentityKey,
                                          second.info().gameIdentityKey);
                        if (!cmp)
                        {
                            // Lastly by server name.
                            return qstricmp(first.info().name,
                                            second.info().name) < 0;
                        }
                        return cmp < 0;
                    }
                    return first.info().numPlayers - second.info().numPlayers > 0;
                }
                return first.isLocal();
            });
        }
    }

    void currentGameChanged(Game const &newGame) override
    {
        if (newGame.isNull() && mode == DiscoverUsingMaster)
        {
            // If the session menu exists across game changes, it's good to
            // keep it up to date.
            link().discoverUsingMaster();
        }
    }

    void gameReadinessUpdated() override
    {
        foreach (Widget *w, self.childWidgets())
        {
            updateAvailability(w->as<GuiWidget>());
        }
    }

    void updateAvailability(GuiWidget &menuItemWidget)
    {
        auto const &item = self.organizer().findItemForWidget(menuItemWidget)->as<ServerListItem>();

        bool playable = false;
        String gameId = item.gameId();
        if (DoomsdayApp::games().contains(gameId))
        {
            playable = DoomsdayApp::games()[gameId].isPlayable();
        }
        menuItemWidget.enable(playable);
    }

    void aboutToJoinMultiplayerGame(serverinfo_t const &sv) override
    {
        DENG2_FOR_PUBLIC_AUDIENCE2(AboutToJoin, i) i->aboutToJoinMultiplayerGame(sv);
    }

//- ChildWidgetOrganizer::IWidgetFactory --------------------------------------

    GuiWidget *makeItemWidget(ui::Item const &, GuiWidget const *) override
    {
        auto *b = new MultiplayerPanelButtonWidget;
        b->audienceForAboutToJoin() += this;
        return b;
    }

    void updateItemWidget(GuiWidget &widget, ui::Item const &item) override
    {
        auto const &serverItem = item.as<ServerListItem>();

        widget.as<MultiplayerPanelButtonWidget>()
                .updateContent(serverItem.info());

        // Is it playable?
        updateAvailability(widget);
    }

    DENG2_PIMPL_AUDIENCE(AboutToJoin)
};

DENG2_AUDIENCE_METHOD(MultiplayerServerMenuWidget, AboutToJoin);

MultiplayerServerMenuWidget::MultiplayerServerMenuWidget(DiscoveryMode discovery,
                                                         String const &name)
    : HomeMenuWidget(name)
    , d(new Impl(this))
{
    d->mode = discovery;

    switch (d->mode)
    {
    case DiscoverUsingMaster:
        d->mask = ServerLink::LocalNetwork | ServerLink::MasterServer;
        d->link().discoverUsingMaster();
        break;

    case DirectDiscoveryOnly:
        d->mask = ServerLink::Direct;
        break;

    case NoDiscovery:
        break;
    }
}