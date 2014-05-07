/** @file notificationareawidget.cpp  Notification area.
 *
 * @authors Copyright (c) 2013 Jaakko Keränen <jaakko.keranen@iki.fi>
 *
 * @par License
 * LGPL: http://www.gnu.org/licenses/lgpl.html
 *
 * <small>This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version. This program is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details. You should have received a copy of
 * the GNU Lesser General Public License along with this program; if not, see:
 * http://www.gnu.org/licenses</small> 
 */

#include "de/NotificationAreaWidget"
#include "de/SequentialLayout"

#include <de/Drawable>
#include <de/Matrix>
#include <de/ScalarRule>

#include <QMap>
#include <QTimer>

namespace de {

static TimeDelta const ANIM_SPAN = .5;

DENG_GUI_PIMPL(NotificationAreaWidget)
, DENG2_OBSERVES(Widget, ChildAddition)
, DENG2_OBSERVES(Widget, ChildRemoval)
, DENG2_OBSERVES(Widget, Deletion)
{
    ScalarRule *shift;

    typedef QMap<GuiWidget *, Widget *> OldParents;
    OldParents oldParents;
    QTimer dismissTimer;
    QList<GuiWidget *> pendingDismiss;

    // GL objects:
    typedef DefaultVertexBuf VertexBuf;
    Drawable drawable;
    GLUniform uMvpMatrix;
    GLUniform uColor;

    Instance(Public *i)
        : Base(i),
          uMvpMatrix("uMvpMatrix", GLUniform::Mat4),
          uColor    ("uColor",     GLUniform::Vec4)
    {
        self.audienceForChildAddition() += this;
        self.audienceForChildRemoval() += this;

        dismissTimer.setSingleShot(true);
        dismissTimer.setInterval(ANIM_SPAN.asMilliSeconds());
        QObject::connect(&dismissTimer, SIGNAL(timeout()), thisPublic, SLOT(dismiss()));

        shift = new ScalarRule(0);
        updateStyle();
    }

    ~Instance()
    {
        // Give current notifications back to their old parents.
        DENG2_FOR_EACH_CONST(OldParents, i, oldParents)
        {
            dismissChild(*i.key());
            i.value()->audienceForDeletion() -= this;
        }

        releaseRef(shift);
    }

    void widgetBeingDeleted(Widget &widget)
    {
        // Is this one of the old parents?
        QMutableMapIterator<GuiWidget *, Widget *> iter(oldParents);
        while(iter.hasNext())
        {
            iter.next();
            if(iter.value() == &widget)
            {
                GuiWidget *notif = iter.key();
                iter.remove();

                // The old parent is gone, so its notifications should also be deleted.
                dismissChild(*notif);
                notif->guiDeleteLater();
            }
        }
    }

    void updateStyle()
    {
        //self.
        //self.set(Background(self.style().colors().colorf("background")));
    }

    void glInit()
    {
        drawable.addBuffer(new VertexBuf);

        shaders().build(drawable.program(), "generic.color_ucolor")
                << uMvpMatrix << uColor;
    }

    void glDeinit()
    {
        drawable.clear();
    }

    void updateGeometry()
    {
        Rectanglei pos;
        if(self.hasChangedPlace(pos) || self.geometryRequested())
        {
            self.requestGeometry(false);

            VertexBuf::Builder verts;
            self.glMakeGeometry(verts);
            drawable.buffer<VertexBuf>().setVertices(gl::TriangleStrip, verts, gl::Static);
        }
    }

    void updateChildLayout()
    {
        Rule const &gap = style().rules().rule("unit");

        // The children are laid out simply in a row from right to left.
        SequentialLayout layout(self.rule().right(), self.rule().top(), ui::Left);

        bool first = true;
        foreach(Widget *child, self.childWidgets())
        {
            GuiWidget &w = child->as<GuiWidget>();
            if(!first)
            {
                layout << gap;
            }
            first = false;

            layout << w;
        }

        // Update the total size of the notification area.
        self.rule().setSize(layout.width(), layout.height());
    }

    void show()
    {
        //self.setOpacity(1, ANIM_SPAN);
        shift->set(0, ANIM_SPAN);
        shift->setStyle(Animation::EaseOut);
    }

    void hide(TimeDelta const &span = ANIM_SPAN)
    {
        //self.setOpacity(0, span);
        shift->set(self.rule().height() + style().rules().rule("gap"), span);
        shift->setStyle(Animation::EaseIn);
    }

    void dismissChild(GuiWidget &notif)
    {
        notif.hide();
        self.remove(notif);

        if(oldParents.contains(&notif))
        {
            Widget *oldParent = oldParents[&notif];
            oldParent->add(&notif);
            oldParent->audienceForDeletion() -= this;
            oldParents.remove(&notif);            
        }
    }

    void performPendingDismiss()
    {
        dismissTimer.stop();

        // The pending children were already asked to be dismissed.
        foreach(GuiWidget *w, pendingDismiss)
        {
            dismissChild(*w);
        }
        pendingDismiss.clear();
    }

    void widgetChildAdded(Widget &child)
    {
        // Set a background for all notifications.
        child.as<GuiWidget>().set(Background(style().colors().colorf("background")));

        updateChildLayout();
        self.show();
    }

    void widgetChildRemoved(Widget &)
    {
        updateChildLayout();
        if(!self.childCount())
        {
            self.hide();
        }
    }
};

NotificationAreaWidget::NotificationAreaWidget(String const &name)
    : GuiWidget(name)
    , d(new Instance(this))
{
    // Initially the widget is empty.
    rule().setSize(Const(0), Const(0));
    d->shift->set(style().fonts().font("default").height().valuei() +
                  style().rules().rule("gap").valuei() * 3);
    hide();
}

void NotificationAreaWidget::useDefaultPlacement(RuleRectangle const &area)
{
    rule().setInput(Rule::Top,   area.top() + style().rules().rule("gap") - shift())
          .setInput(Rule::Right, area.right() - style().rules().rule("gap"));
}

Rule const &NotificationAreaWidget::shift()
{
    return *d->shift;
}

void NotificationAreaWidget::showChild(GuiWidget *notif)
{
    DENG2_ASSERT(notif != 0);

    if(isChildShown(*notif))
    {
        // Already in the notification area.
        return;
    }

    // Cancel a pending dismissal.
    d->performPendingDismiss();

    if(notif->parentWidget())
    {
        d->oldParents.insert(notif, notif->parentWidget());
        notif->parentWidget()->audienceForDeletion() += d;
        notif->parentWidget()->remove(*notif);
    }
    add(notif);
    notif->show();
    d->show();
}

void NotificationAreaWidget::hideChild(GuiWidget &notif)
{
    if(!isChildShown(notif))
    {
        // Already in the notification area.
        return;
    }

    if(childCount() > 1)
    {
        // Dismiss immediately, the area itself remains open.
        d->dismissChild(notif);
    }
    else
    {
        // The last one should be deferred until the notification area
        // itself is dismissed.
        d->dismissTimer.start();
        d->pendingDismiss << &notif;
        d->hide();
    }
}

void NotificationAreaWidget::dismiss()
{
    d->performPendingDismiss();
}

bool NotificationAreaWidget::isChildShown(GuiWidget &notif) const
{
    if(d->pendingDismiss.contains(&notif))
    {
        return false;
    }
    return notif.parentWidget() == this;
}

void NotificationAreaWidget::viewResized()
{
    GuiWidget::viewResized();

    d->uMvpMatrix = root().projMatrix2D();
}

void NotificationAreaWidget::drawContent()
{
    d->updateGeometry();

    d->uColor = Vector4f(1, 1, 1, visibleOpacity());
    d->drawable.draw();
}

void NotificationAreaWidget::glInit()
{
    d->glInit();
}

void NotificationAreaWidget::glDeinit()
{
    d->glDeinit();
}

} // namespace de