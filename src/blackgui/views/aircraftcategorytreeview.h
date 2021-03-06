/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_VIEWS_AIRCRAFTCATEGORYTREEVIEW_H
#define BLACKGUI_VIEWS_AIRCRAFTCATEGORYTREEVIEW_H

#include "blackgui/blackguiexport.h"
#include "blackmisc/aviation/aircraftcategorylist.h"
#include "blackmisc/digestsignal.h"

#include <QTreeView>
#include <QObject>
#include <QPoint>
#include <QMap>
#include <QModelIndex>

namespace BlackGui
{
    namespace Models
    {
        class CAircraftCategoryTreeModel;
        class CColumns;
    }

    namespace Views
    {
        //! ATC stations view
        class BLACKGUI_EXPORT CAircraftCategoryTreeView : public QTreeView
        {
            Q_OBJECT

        public:
            //! Constructor
            explicit CAircraftCategoryTreeView(QWidget *parent = nullptr);

            //! Update container
            void updateContainer(const BlackMisc::Aviation::CAircraftCategoryList &categories);

            //! Clear
            void clear();

            //! Resize all columns
            void fullResizeToContents();

            //! Set columns
            void setColumns(const Models::CColumns &columns);

            //! Empty data
            bool isEmpty() const;

        private:
            //! Used model
            const Models::CAircraftCategoryTreeModel *categoryModel() const;

            //! Used model
            BlackGui::Models::CAircraftCategoryTreeModel *categoryModel();

            //! Resize all columns
            void fullResizeToContentsImpl();

            //! The selected object
            BlackMisc::Aviation::CAircraftCategory selectedObject() const;

            //! Expanded
            void onExpanded(const QModelIndex &index);

            //! Custom menu
            void customMenu(const QPoint &point);

            BlackMisc::CDigestSignal m_dsFullResize { this, &CAircraftCategoryTreeView::fullResizeToContentsImpl, 1000, 25 };
        };
    } // ns
} // ns

#endif // guard
