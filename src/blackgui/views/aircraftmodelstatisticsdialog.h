/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_VIEW_AIRCRAFTMODELSTATISTICSDIALOG_H
#define BLACKGUI_VIEW_AIRCRAFTMODELSTATISTICSDIALOG_H

#include "blackmisc/simulation/aircraftmodellist.h"
#include <QDialog>
#include <QScopedPointer>

namespace Ui { class CAircraftModelStatisticsDialog; }
namespace BlackGui
{
    namespace Views
    {
        //! Info about the models
        class CAircraftModelStatisticsDialog : public QDialog
        {
            Q_OBJECT

        public:
            //! Constructor
            explicit CAircraftModelStatisticsDialog(QWidget *parent = nullptr);

            //! Destructor
            virtual ~CAircraftModelStatisticsDialog();

            //! Set and analyze the models
            void analyzeModels(const BlackMisc::Simulation::CAircraftModelList &models);

        private:
            //! Display the HTML matrix
            void displayHTMLMatrix();

            QScopedPointer<Ui::CAircraftModelStatisticsDialog> ui;
            BlackMisc::Simulation::CAircraftModelList m_models;
        };
    } // ns
} // ns

#endif // guard
