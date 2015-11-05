/* Copyright (C) 2014
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "enableforframelesswindow.h"
#include "blackmisc/icons.h"
#include "blackmisc/blackmiscfreefunctions.h"
#include <QStatusBar>
#include <QPushButton>

using namespace BlackMisc;

namespace BlackGui
{

    CEnableForFramelessWindow::CEnableForFramelessWindow(CEnableForFramelessWindow::WindowMode mode, bool isMainApplicationWindow, const char *framelessPropertyName, QWidget *correspondingWidget) :
        m_windowMode(mode), m_mainApplicationWindow(isMainApplicationWindow), m_widget(correspondingWidget), m_framelessPropertyName(framelessPropertyName)
    {
        Q_ASSERT(correspondingWidget);
        Q_ASSERT(!m_framelessPropertyName.isEmpty());
        this->setWindowAttributes(mode);
    }

    void CEnableForFramelessWindow::setMode(CEnableForFramelessWindow::WindowMode mode)
    {
        if (mode == this->m_windowMode) { return; }
        // set the main window or dock widget flags and attributes
        this->m_widget->setWindowFlags(modeToWindowFlags(mode));
        this->setWindowAttributes(mode);
        this->m_widget->show();
        this->m_windowMode = mode;
    }

    void CEnableForFramelessWindow::setFrameless(bool frameless)
    {
        setMode(frameless ? WindowFrameless : WindowTool);
    }

    CEnableForFramelessWindow::WindowMode CEnableForFramelessWindow::stringToWindowMode(const QString &s)
    {
        QString ws(s.trimmed().toLower());
        if (ws.isEmpty()) { return WindowNormal; }
        if (ws.contains("frameless") || ws.startsWith("f")) { return WindowFrameless; }
        if (ws.contains("tool") || ws.startsWith("t")) { return WindowTool; }
        return WindowNormal;
    }

    QString CEnableForFramelessWindow::windowModeToString(CEnableForFramelessWindow::WindowMode m)
    {
        switch (m)
        {
        case WindowFrameless: return "frameless";
        case WindowNormal: return "normal";
        case WindowTool: return "tool";
        default:
            break;
        }
        return "normal";
    }

    void CEnableForFramelessWindow::setWindowAttributes(CEnableForFramelessWindow::WindowMode mode)
    {
        Q_ASSERT_X(this->m_widget, "CEnableForFramelessWindow::setWindowAttributes", "Missing widget representing window");
        Q_ASSERT_X(!this->m_framelessPropertyName.isEmpty(), "CEnableForFramelessWindow::setWindowAttributes", "Missing property name");

        bool frameless = (mode == WindowFrameless);
        // http://stackoverflow.com/questions/18316710/frameless-and-transparent-window-qt5
        this->m_widget->setAttribute(Qt::WA_NoSystemBackground, frameless);
        this->m_widget->setAttribute(Qt::WA_TranslucentBackground, frameless);

        // Qt::WA_PaintOnScreen leads to a warning
        // setMask(QRegion(10, 10, 10, 10) would work, but requires "complex" calcs for rounded corners
        //! \todo Transparent dock widget,try out void QWidget::setMask
        this->setDynamicProperties(frameless);
    }

    void CEnableForFramelessWindow::setDynamicProperties(bool frameless)
    {
        Q_ASSERT_X(this->m_widget, "CEnableForFramelessWindow::setDynamicProperties", "Missing widget representing window");
        Q_ASSERT_X(!this->m_framelessPropertyName.isEmpty(), "CEnableForFramelessWindow::setDynamicProperties", "Missing property name");

        // property selector will check on string, so I directly provide a string
        const QString f(BlackMisc::boolToTrueFalse(frameless));
        this->m_widget->setProperty(this->m_framelessPropertyName.constData(), f);
        for (QObject *w : this->m_widget->children())
        {
            if (w && w->isWidgetType())
            {
                w->setProperty(this->m_framelessPropertyName.constData(), f);
            }
        }
    }

    bool CEnableForFramelessWindow::handleMouseMoveEvent(QMouseEvent *event)
    {
        Q_ASSERT(this->m_widget);
        if (this->m_windowMode == WindowFrameless && event->buttons() & Qt::LeftButton)
        {
            this->m_widget->move(event->globalPos() - this->m_framelessDragPosition);
            event->accept();
            return true;
        }
        return false;
    }

    bool CEnableForFramelessWindow::handleMousePressEvent(QMouseEvent *event)
    {
        Q_ASSERT(this->m_widget);
        if (this->m_windowMode == WindowFrameless && event->button() == Qt::LeftButton)
        {
            this->m_framelessDragPosition = event->globalPos() - this->m_widget->frameGeometry().topLeft();
            event->accept();
            return true;
        }
        return false;
    }

    void CEnableForFramelessWindow::addFramelessSizeGripToStatusBar(QStatusBar *statusBar)
    {
        if (!statusBar) { return; }
        if (!this->m_framelessSizeGrip)
        {
            this->m_framelessSizeGrip = new QSizeGrip(this->m_widget);
            this->m_framelessSizeGrip->setObjectName("sg_FramelessSizeGrip");
            statusBar->addPermanentWidget(this->m_framelessSizeGrip);
        }
        else
        {
            this->m_framelessSizeGrip->show();
        }
        statusBar->repaint();
    }

    void CEnableForFramelessWindow::hideFramelessSizeGripInStatusBar()
    {
        if (!this->m_framelessSizeGrip) { return; }
        this->m_framelessSizeGrip->hide();
    }

    QHBoxLayout *CEnableForFramelessWindow::addFramelessCloseButton(QMenuBar *menuBar)
    {
        Q_ASSERT(isFrameless());
        Q_ASSERT(menuBar);
        Q_ASSERT(this->m_widget);

        if (!m_framelessCloseButton)
        {
            m_framelessCloseButton = new QPushButton(this->m_widget);
            m_framelessCloseButton->setObjectName("pb_FramelessCloseButton");
            m_framelessCloseButton->setIcon(CIcons::close16());
            QObject::connect(m_framelessCloseButton, &QPushButton::clicked, this->m_widget, &QWidget::close);
        }

        QHBoxLayout *menuBarLayout = new QHBoxLayout;
        menuBarLayout->setObjectName("hl_MenuBar");
        menuBarLayout->addWidget(menuBar, 0, Qt::AlignTop | Qt::AlignLeft);
        menuBarLayout->addWidget(m_framelessCloseButton, 0, Qt::AlignTop | Qt::AlignRight);
        return menuBarLayout;
    }

    void CEnableForFramelessWindow::toolToNormalWindow()
    {
        this->m_widget->setWindowFlags((this->m_widget->windowFlags() & (~Qt::Tool)) | Qt::Window);
    }

    void CEnableForFramelessWindow::normalToToolWindow()
    {
        this->m_widget->setWindowFlags(this->m_widget->windowFlags() | Qt::Tool);
    }

    bool CEnableForFramelessWindow::isToolWindow() const
    {
        return (this->m_widget->windowFlags() & Qt::Tool) == Qt::Tool;
    }

    Qt::WindowFlags CEnableForFramelessWindow::modeToWindowFlags(CEnableForFramelessWindow::WindowMode mode)
    {
        switch (mode)
        {
        case WindowFrameless:
            return (Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        case WindowTool:
            // tool window and minimized not supported on windows
            // tool window always with close button on windows
            return (Qt::Tool | Qt::WindowStaysOnTopHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        case WindowNormal:
        default:
            return (Qt::Window | Qt::WindowStaysOnTopHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        }
    }

} // namespace
