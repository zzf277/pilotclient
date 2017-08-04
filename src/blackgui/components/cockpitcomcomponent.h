/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_COCKPITCOMCOMPONENT_H
#define BLACKGUI_COCKPITCOMCOMPONENT_H

#include "blackgui/blackguiexport.h"
#include "blackgui/components/enablefordockwidgetinfoarea.h"
#include "blackmisc/audio/voiceroomlist.h"
#include "blackmisc/aviation/selcal.h"
#include "blackmisc/aviation/transponder.h"
#include "blackmisc/identifiable.h"
#include "blackmisc/identifier.h"
#include "blackmisc/simulation/simulatedaircraft.h"

#include <QFrame>
#include <QObject>
#include <QScopedPointer>

class QPaintEvent;
class QWidget;

namespace BlackMisc { namespace Aviation { class CComSystem; } }
namespace Ui { class CCockpitComComponent; }
namespace BlackGui
{
    namespace Components
    {
        //! The main cockpit area
        class BLACKGUI_EXPORT CCockpitComComponent :
            public QFrame,
            public BlackMisc::CIdentifiable,
            public CEnableForDockWidgetInfoArea
        {
            Q_OBJECT

        public:
            //! Constructor
            explicit CCockpitComComponent(QWidget *parent = nullptr);

            //! Destructor
            ~CCockpitComComponent();

        signals:
            //! \copydoc BlackGui::Components::CTransponderModeSelector::transponderModeChanged
            void transponderModeChanged(BlackMisc::Aviation::CTransponder::TransponderMode newMode);

            //! \copydoc BlackGui::Components::CTransponderModeSelector::transponderStateIdentEnded
            void transponderStateIdentEnded();

        public slots:
            //!\ Set to ident
            void setSelectedTransponderModeStateIdent();

        protected:
            //! \copydoc QWidget::paintEvent
            virtual void paintEvent(QPaintEvent *event) override;

        private slots:
            //! Cockpit values have been changed in GUI
            void ps_guiChangedCockpitValues();

            //! SELCAL changed in GUI
            void ps_guiChangedSelcal();

            //! Update cockpit from context
            void ps_updateCockpitFromContext(const BlackMisc::Simulation::CSimulatedAircraft &ownAircraft, const BlackMisc::CIdentifier &originator);

            //! Cockpit values have been changed in GUI
            void ps_testSelcal();

            //! SELCAL was changed
            void ps_onChangedSelcal(const BlackMisc::Aviation::CSelcal &selcal, const BlackMisc::CIdentifier &originator);

            //! Update voice room related information
            void ps_onChangedVoiceRoomStatus(const BlackMisc::Audio::CVoiceRoomList &selectedVoiceRooms, bool connected);

        private:
            //! Init LEDs
            void initLeds();

            //! Cockpit values to aircraft
            BlackMisc::Simulation::CSimulatedAircraft cockpitValuesToAircraftObject();

            //! Get own aircraft
            BlackMisc::Simulation::CSimulatedAircraft getOwnAircraft() const;

            //! Current SELCAL code
            BlackMisc::Aviation::CSelcal getSelcal() const;

            //! Cockpit updates
            bool updateOwnCockpitInContext(const BlackMisc::Simulation::CSimulatedAircraft &ownAircraft);

            //! COM frequencies displayed
            void updateFrequencyDisplaysFromComSystems(const BlackMisc::Aviation::CComSystem &com1, const BlackMisc::Aviation::CComSystem &com2);

            QScopedPointer<Ui::CCockpitComComponent> ui;
       };

    } // namespace
} // namespace

#endif // guard
