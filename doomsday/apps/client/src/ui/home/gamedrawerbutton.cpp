/** @file gamedrawerbutton.cpp  Button for
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

#include "ui/home/gamedrawerbutton.h"
#include "ui/home/savelistwidget.h"
#include "ui/savedsessionlistdata.h"

#include <de/ChildWidgetOrganizer>

using namespace de;

DENG_GUI_PIMPL(GameDrawerButton)
, public ChildWidgetOrganizer::IFilter
{
    Game const &game;
    SavedSessionListData const &savedItems;
    SaveListWidget *saves;
    ButtonWidget *playButton;

    Instance(Public *i, Game const &game, SavedSessionListData const &savedItems)
        : Base(i)
        , game(game)
        , savedItems(savedItems)
    {
        ButtonWidget *packages = new ButtonWidget;
        packages->setImage(style().images().image("package"));
        packages->setOverrideImageSize(style().fonts().font("default").height().value());
        packages->setSizePolicy(ui::Expand, ui::Expand);
        self.addButton(packages);

        playButton = new ButtonWidget;
        playButton->useInfoStyle();
        playButton->setImage(style().images().image("play"));
        playButton->setImageColor(style().colors().colorf("inverted.text"));
        playButton->setOverrideImageSize(style().fonts().font("default").height().value());
        playButton->setSizePolicy(ui::Expand, ui::Expand);
        self.addButton(playButton);

        // List of saved games.
        saves = new SaveListWidget(self);
        saves->rule().setInput(Rule::Width, self.rule().width()); // - self.margins().width());
        saves->organizer().setFilter(*this);
        saves->setItems(savedItems);
        saves->margins().setZero().setLeft(self.icon().rule().width());

        self.drawer().setContent(saves);
        self.drawer().open();
    }

//- ChildWidgetOrganizer::IFilter ---------------------------------------------

    bool isItemAccepted(ChildWidgetOrganizer const &organizer,
                        ui::Data const &data, ui::Data::Pos pos) const
    {
        // Only saved sessions for this game are to be included.
        auto const &item = data.at(pos).as<SavedSessionListData::SaveItem>();
        return item.gameId() == game.id();
    }
};

GameDrawerButton::GameDrawerButton(Game const &game, SavedSessionListData const &savedItems)
    : d(new Instance(this, game, savedItems))
{
    //if(d->saves->childCount() > 0) drawer().open();
}

void GameDrawerButton::updateContent()
{
    enable(d->game.isPlayable());

    String meta;
    if(isSelected())
    {
        //root().setFocus(d->playButton);
        meta = String("%1 saved games").arg(d->saves->childCount());
    }
    else
    {
        meta = String::number(d->game.releaseDate().year());
    }

    label().setText(String(_E(b) "%1\n" _E(l) "%2")
                    .arg(d->game.title())
                    .arg(meta));
}

ButtonWidget &GameDrawerButton::playButton()
{
    return *d->playButton;
}
