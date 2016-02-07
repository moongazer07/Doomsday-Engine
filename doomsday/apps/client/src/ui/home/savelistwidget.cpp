/** @file savelistwidget.cpp  List showing the available saves of a game.
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

#include "ui/home/savelistwidget.h"
#include "ui/home/gamedrawerbutton.h"
#include "ui/savedsessionlistdata.h"

#include <doomsday/game.h>

#include <de/CallbackAction>

using namespace de;

DENG_GUI_PIMPL(SaveListWidget)
, DENG2_OBSERVES(ChildWidgetOrganizer, WidgetUpdate)
{
    GameDrawerButton &owner;

    Instance(Public *i, GameDrawerButton &owner) : Base(i), owner(owner)
    {
        self.organizer().audienceForWidgetUpdate() += this;
    }

    void widgetUpdatedForItem(GuiWidget &widget, ui::Item const &item)
    {
        auto &button = widget.as<ButtonWidget>();
        button.setTextAlignment(ui::AlignRight);
        button.setAlignment(ui::AlignLeft);
        button.setTextLineAlignment(ui::AlignLeft);
        button.setSizePolicy(ui::Filled, ui::Expand);
        button.setText(item.label());
        button.margins().set("dialog.gap");
        button.set(Background(Vector4f()));

        button.setAction(new CallbackAction([this, &button] ()
        {
            if(button.isUsingInfoStyle())
            {
                button.useNormalStyle();
                button.set(Background());
            }
            else
            {
                button.useInfoStyle();
                button.set(Background(style().colors().colorf("inverted.background")));
            }
            emit owner.mouseActivity();
        }));

        auto const &saveItem = item.as<SavedSessionListData::SaveItem>();
        button.setImage(style().images().image(Game::logoImageForId(saveItem.gameId())));
    }
};

SaveListWidget::SaveListWidget(GameDrawerButton &owner)
    : d(new Instance(this, owner))
{
    setGridSize(1, ui::Filled, 0, ui::Expand);
    enableScrolling(false);
    enablePageKeys(false);
}
