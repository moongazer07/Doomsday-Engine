/** @file nogamescolumnwidget.cpp
 *
 * @authors Copyright (c) 2016-2017 Jaakko Keränen <jaakko.keranen@iki.fi>
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

#include "ui/home/nogamescolumnwidget.h"
#include "ui/clientwindow.h"
#include "ui/widgets/taskbarwidget.h"
#include "clientapp.h"

#include <de/ButtonWidget>
#include <de/Config>
#include <de/SignalAction>
#include <QFileDialog>

using namespace de;

NoGamesColumnWidget::NoGamesColumnWidget()
    : ColumnWidget("nogames-column")
{
    header().hide();

    LabelWidget *notice = LabelWidget::newWithText(
                _E(b) + tr("No playable games were found.\n") + _E(.) +
                tr("Please select the folder where you have one or more game WAD files."),
                this);
    notice->setMaximumTextWidth(maximumContentWidth());
    notice->setTextColor("text");
    notice->setSizePolicy(ui::Expand, ui::Expand);
    notice->rule()
            .setMidAnchorX(rule().midX())
            .setInput(Rule::Bottom, rule().midY());

    ButtonWidget *chooseIwad = new ButtonWidget;
    chooseIwad->setText(tr("Select WAD Folder..."));
    chooseIwad->setSizePolicy(ui::Expand, ui::Expand);
    chooseIwad->rule()
            .setMidAnchorX(rule().midX())
            .setInput(Rule::Top, notice->rule().bottom());
    chooseIwad->setAction(new SignalAction(this, SLOT(browseForDataFiles())));
    add(chooseIwad);
}

String NoGamesColumnWidget::tabHeading() const
{
    return tr("Data Files?");
}

void NoGamesColumnWidget::browseForDataFiles()
{
    // Use a native dialog to select the IWAD folder.
    ClientApp::app().beginNativeUIMode();

    QFileDialog dlg(nullptr,
                    tr("Select IWAD Folder"),
                    App::config().gets("resource.iwadFolder", ""));
    dlg.setFileMode(QFileDialog::Directory);
    dlg.setReadOnly(true);
    //dlg.setNameFilter("*.wad");
    dlg.setLabelText(QFileDialog::Accept, tr("Select"));
    bool reload = false;
    if (dlg.exec())
    {
        App::config().set("resource.iwadFolder", dlg.selectedFiles().at(0));
        reload = true;
    }

    ClientApp::app().endNativeUIMode();

    // Reload packages and recheck for game availability.
    if (reload)
    {
        ClientWindow::main().console().closeLogAndUnfocusCommandLine();
        DoomsdayApp::app().initWadFolders();
    }
}
