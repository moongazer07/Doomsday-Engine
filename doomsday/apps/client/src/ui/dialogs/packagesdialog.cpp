/** @file packagesdialog.cpp
 *
 * @authors Copyright (c) 2015 Jaakko Keränen <jaakko.keranen@iki.fi>
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

#include "ui/dialogs/packagesdialog.h"
#include "clientapp.h"

#include <de/FileSystem>
#include <de/MenuWidget>
#include <de/ChildWidgetOrganizer>
#include <de/SequentialLayout>

using namespace de;

DENG_GUI_PIMPL(PackagesDialog)
, public ChildWidgetOrganizer::IWidgetFactory
{
    MenuWidget *menu;

    /**
     * Information about an available package.
     */
    struct PackageItem : public ui::Item
    {
        File const &file;
        Record const &info;

        PackageItem(File const &packFile)
            : file(packFile)
            , info(file.info().subrecord("package"))
        {}
    };

    /**
     * Widget showing information about a package and containing buttons for manipulating
     * the package.
     */
    class Widget : public GuiWidget
    {
    public:
        Widget(PackageItem const &item)
            : _item(item)
        {
            add(_title = new LabelWidget);
            _title->setSizePolicy(ui::Fixed, ui::Expand);
            _title->setAlignment(ui::AlignLeft);
            _title->setTextLineAlignment(ui::AlignLeft);
            _title->margins().setBottom("").setLeft("unit");

            add(_subtitle = new LabelWidget);
            _subtitle->setSizePolicy(ui::Fixed, ui::Expand);
            _subtitle->setAlignment(ui::AlignLeft);
            _subtitle->setTextLineAlignment(ui::AlignLeft);
            _subtitle->margins().setTop("").setBottom("unit").setLeft("unit");

            add(_loadButton = new ButtonWidget);
            _loadButton->setSizePolicy(ui::Expand, ui::Expand);
            add(_infoButton = new ButtonWidget);
            _infoButton->setSizePolicy(ui::Expand, ui::Fixed);
            _infoButton->setText(_E(s)_E(B) + tr("..."));

            createTagButtons();

            AutoRef<Rule> titleWidth(rule().width() -
                                     _loadButton->rule().width() -
                                     _infoButton->rule().width());

            _title->rule()
                    .setInput(Rule::Width, titleWidth)
                    .setInput(Rule::Left, rule().left())
                    .setInput(Rule::Top,  rule().top());
            _subtitle->rule()
                    .setInput(Rule::Width, titleWidth)
                    .setInput(Rule::Left, rule().left())
                    .setInput(Rule::Top,  _title->rule().bottom());

            _loadButton->rule()
                    .setInput(Rule::Right, rule().right() - _infoButton->rule().width())
                    .setMidAnchorY(rule().midY());
            _infoButton->rule()
                    .setInput(Rule::Right, rule().right())
                    .setInput(Rule::Height, _loadButton->rule().height())
                    .setMidAnchorY(rule().midY());

            rule().setInput(Rule::Width,  style().rules().rule("dialog.packages.width"))
                  .setInput(Rule::Height, _title->rule().height() +
                            _subtitle->rule().height() + _tags.at(0)->rule().height());
        }

        void createTagButtons()
        {
            SequentialLayout layout(_subtitle->rule().left(),
                                    _subtitle->rule().bottom(), ui::Right);

            for(QString tag : _item.info.gets("tags").split(' ', QString::SkipEmptyParts))
            {
                auto *btn = new ButtonWidget;
                btn->setText(_E(l) + tag.toLower());
                btn->setFont("small");
                btn->setTextColor("accent");
                btn->set(Background(Background::Rounded, style().colors().colorf("accent"), 6));
                btn->setSizePolicy(ui::Expand, ui::Expand);
                btn->margins()
                        .setTop("unit").setBottom("unit")
                        .setLeft("gap").setRight("gap");
                add(btn);

                layout << *btn;
                _tags.append(btn);
            }
        }

        void updateContents()
        {
            _title->setText(_item.info.gets("title"));
            _subtitle->setText(_E(n) + packageId());

            if(isLoaded())
            {
                _loadButton->setText(tr("Unload"));
            }
            else
            {
                _loadButton->setText(tr("Load"));
            }
        }

        bool isLoaded() const
        {
            return App::packageLoader().isLoaded(packageId());
        }

        String packageId() const
        {
            return _item.info.gets("ID");
        }

    private:
        PackageItem const &_item;
        LabelWidget *_title;
        LabelWidget *_subtitle;
        QList<ButtonWidget *> _tags;
        ButtonWidget *_loadButton;
        ButtonWidget *_infoButton;
    };

    Instance(Public *i) : Base(i)
    {
        self.area().add(menu = new MenuWidget);
        menu->enableScrolling(false); // dialog content already scrolls
        menu->enablePageKeys(false);
        menu->setGridSize(1, ui::Expand, 0, ui::Expand);
        menu->rule()
                //.setInput(Rule::Width, style().rules().rule("dialog.packages.width"))
                .setInput(Rule::Left,  self.area().contentRule().left())
                .setInput(Rule::Top,   self.area().contentRule().top());
        menu->organizer().setWidgetFactory(*this);
    }

    void populate()
    {
        StringList packages = App::packageLoader().findAllPackages();
        qSort(packages);

        for(String const &path : packages)
        {
            Folder const &pack = App::rootFolder().locate<Folder>(path);

            menu->items() << new PackageItem(pack);

            //Record const &info = pack.info().subrecord("package");
            /*qDebug() << path
                     << "id:" << info.gets("ID")
                     << "title:" << info.gets("title")
                     << "version:" << info.gets("version")
                     << "license:" << info.gets("license")
                     << "tags:" << info.gets("tags");*/
        }
    }

    GuiWidget *makeItemWidget(ui::Item const &item, GuiWidget const *)
    {
        return new Widget(item.as<PackageItem>());
    }

    void updateItemWidget(GuiWidget &widget, ui::Item const &)
    {
        widget.as<Widget>().updateContents();
    }
};

PackagesDialog::PackagesDialog()
    : DialogWidget("packages", WithHeading)
    , d(new Instance(this))
{
    heading().setText(tr("Packages"));
    buttons() << new DialogButtonItem(Default | Accept, tr("Close"));

    area().setContentSize(d->menu->rule().width(), d->menu->rule().height());

    d->populate();
}