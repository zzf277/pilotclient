/* Copyright (C) 2016
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_COMPLETER_AIRCRAFTMODELSTRINGCOMPLETER_H
#define BLACKGUI_COMPLETER_AIRCRAFTMODELSTRINGCOMPLETER_H

#include "blackmisc/simulation/aircraftmodel.h"
#include <QFrame>
#include <QScopedPointer>
#include <QCompleter>
#include <QFlags>

namespace Ui { class CAircraftModelStringCompleter; }
namespace BlackGui
{
    namespace Components
    {
        /*!
         * Completer for model strings
         */
        class CAircraftModelStringCompleter : public QFrame
        {
            Q_OBJECT

        public:

            //! Sources for string completion
            enum CompleterSourceFlag {
                DB,
                ModelSet,
                OwnModels
            };
            Q_DECLARE_FLAGS(CompleterSource, CompleterSourceFlag)

            //! Constructor
            explicit CAircraftModelStringCompleter(QWidget *parent = nullptr);

            //! Destructor
            ~CAircraftModelStringCompleter();

            //! The model string
            QString getModelString() const;

            //! Show the selection buttons
            void showSourceSelection(bool show);

            //! Set text
            void setText(const QString &completersString);

            //! Set model
            void setModel(const BlackMisc::Simulation::CAircraftModel &model);

            //! Show/hide radio buttons
            void setSourceVisible(CompleterSource source, bool visible);

            //! Set the currently selected source
            void selectSource(CompleterSourceFlag source);

            //! Clear
            void clear();

        signals:
            //! Model has been changed
            void modelStringChanged();

        private:
            //! Set the completer
            void setCompleter();

            //! How completer behaves
            static void setCompleterParameters(QCompleter *completer);

        private slots:
            //! Value has been changed
            void ps_textChanged();

            //! Init the GUI
            void ps_initGui();

            //! Simulator connected
            void ps_simulatorConnected(int status);

            //! All swift data have been read
            void ps_swiftWebDataRead();

        private:
            QScopedPointer <Ui::CAircraftModelStringCompleter> ui;
        };
    } // ns
} // ns

#endif // guard
