/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_FILTERS_FILTERBARAIRCRAFTMODEL_H
#define BLACKGUI_FILTERS_FILTERBARAIRCRAFTMODEL_H

#include "blackgui/blackguiexport.h"
#include "blackgui/filters/filterwidget.h"
#include "blackgui/models/modelfilter.h"
#include "blackmisc/simulation/distributor.h"
#include "blackmisc/simulation/simulatorinfo.h"

#include <QObject>
#include <QScopedPointer>
#include <memory>

class QWidget;

namespace BlackMisc { namespace Simulation { class CAircraftModelList; } }
namespace Ui { class CAircraftModelFilterBar; }

namespace BlackGui
{
    namespace Filters
    {
        /*!
         * Filter bar for aircraft models
         */
        class BLACKGUI_EXPORT CAircraftModelFilterBar :
            public CFilterWidget,
            public BlackGui::Models::IModelFilterProvider<BlackMisc::Simulation::CAircraftModelList>
        {
            Q_OBJECT

        public:
            //! Constructor
            explicit CAircraftModelFilterBar(QWidget *parent = nullptr);

            //! Destructor
            virtual ~CAircraftModelFilterBar() override;

            //! Show count
            void displayCount(bool show);

            //! \copydoc Models::IModelFilterProvider::createModelFilter
            virtual std::unique_ptr<BlackGui::Models::IModelFilter<BlackMisc::Simulation::CAircraftModelList>> createModelFilter() const override;

        public slots:
            //! \copydoc CFilterWidget::onRowCountChanged
            virtual void onRowCountChanged(int count, bool withFilter) override;

        protected:
            //! \copydoc CFilterWidget::clearForm
            virtual void clearForm() override;

        private slots:
            //! Simulator selection changed
            void ps_simulatorSelectionChanged(const BlackMisc::Simulation::CSimulatorInfo &info);

            //! Distributor changed
            void ps_distributorChanged(const BlackMisc::Simulation::CDistributor &distributor);

            //! Checkbox has been changed
            void ps_checkBoxChanged(bool state);

        private:
            QScopedPointer<Ui::CAircraftModelFilterBar> ui;
        };
    } // ns
} // ns

#endif // guard
