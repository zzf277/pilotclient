/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "interpolationsetupcomponent.h"
#include "ui_interpolationsetupcomponent.h"
#include "blackgui/views/interpolationsetupview.h"
#include "blackgui/guiapplication.h"
#include "blackcore/context/contextsimulator.h"
#include "blackmisc/simulation/interpolationsetuplist.h"
#include "blackmisc/statusmessage.h"
#include <QPointer>

using namespace BlackGui::Views;
using namespace BlackMisc;
using namespace BlackMisc::Aviation;
using namespace BlackMisc::Simulation;
using namespace BlackCore::Context;

namespace BlackGui
{
    namespace Components
    {
        CInterpolationSetupComponent::CInterpolationSetupComponent(QWidget *parent) :
            COverlayMessagesFrame(parent),
            ui(new Ui::CInterpolationSetupComponent)
        {
            ui->setupUi(this);
            ui->tvp_InterpolationSetup->menuAddItems(CInterpolationSetupView::MenuRemoveSelectedRows);

            connect(ui->pb_RenderingSetup, &QPushButton::clicked, this, &CInterpolationSetupComponent::requestRenderingRestrictionsWidget);
            connect(ui->pb_Save, &QPushButton::clicked, this, &CInterpolationSetupComponent::saveSetup);
            connect(ui->pb_Delete, &QPushButton::clicked, this, &CInterpolationSetupComponent::removeSetup);
            connect(ui->pb_Reload, &QPushButton::clicked, this, &CInterpolationSetupComponent::reloadSetup);
            connect(ui->tvp_InterpolationSetup, &CInterpolationSetupView::doubleClicked, this, &CInterpolationSetupComponent::onRowDoubleClicked);
            connect(ui->tvp_InterpolationSetup, &CInterpolationSetupView::modelChanged, this, &CInterpolationSetupComponent::onModelChanged);
            connect(ui->rb_Callsign, &QRadioButton::released, this, &CInterpolationSetupComponent::onModeChanged);
            connect(ui->rb_Global, &QRadioButton::released, this, &CInterpolationSetupComponent::onModeChanged);
            if (sGui &&  sGui->getIContextSimulator())
            {
                connect(sGui->getIContextSimulator(), &IContextSimulator::interpolationAndRenderingSetupChanged, this, &CInterpolationSetupComponent::onSetupChanged);
            }

            ui->rb_Global->setChecked(true);

            QPointer<CInterpolationSetupComponent> myself(this);
            QTimer::singleShot(1000, this, [ = ]
            {
                if (myself.isNull()) { return; }
                this->onModeChanged();
            });

            QTimer::singleShot(30 * 1000, this, [ = ]
            {
                if (myself.isNull()) { return; }
                this->onSetupChanged();
            });
        }

        CInterpolationSetupComponent::~CInterpolationSetupComponent()
        { }

        CInterpolationSetupComponent::Mode CInterpolationSetupComponent::getSetupMode() const
        {
            return ui->rb_Callsign->isChecked() ? CInterpolationSetupComponent::PerCallsign : CInterpolationSetupComponent::Global;
        }

        void CInterpolationSetupComponent::onRowDoubleClicked(const QModelIndex &index)
        {
            if (!index.isValid()) { return; }
            const CInterpolationAndRenderingSetupPerCallsign setup = ui->tvp_InterpolationSetup->at(index);
            ui->form_InterpolationSetup->setValue(setup);
            ui->comp_CallsignCompleter->setCallsign(setup.getCallsign());
            ui->comp_CallsignCompleter->setReadOnly(false);
            ui->rb_Callsign->setChecked(true);
        }

        void CInterpolationSetupComponent::onModeChanged()
        {
            bool enableCallsign = false;
            if (this->getSetupMode() == CInterpolationSetupComponent::Global)
            {
                this->setUiValuesFromGlobal();
            }
            else
            {
                enableCallsign = true;
            }
            this->displaySetupsPerCallsign();
            ui->comp_CallsignCompleter->setReadOnly(!enableCallsign);
            ui->pb_Delete->setEnabled(enableCallsign);
        }

        void CInterpolationSetupComponent::onModelChanged()
        {
            const CInterpolationSetupList setups = ui->tvp_InterpolationSetup->container();
            this->setSetupsToContext(setups);
        }

        void CInterpolationSetupComponent::reloadSetup()
        {
            const bool overlay = QObject::sender() == ui->pb_Reload;
            if (!this->checkPrerequisites(overlay)) { return; }
            if (this->getSetupMode() == CInterpolationSetupComponent::Global)
            {
                CInterpolationAndRenderingSetupGlobal gs = sGui->getIContextSimulator()->getInterpolationAndRenderingSetupGlobal();
                ui->form_InterpolationSetup->setValue(gs);
            }
            else
            {
                const CCallsign cs = ui->comp_CallsignCompleter->getCallsign(false);
                if (!cs.isValid()) { return; }
                const CInterpolationAndRenderingSetupPerCallsign setup = sGui->getIContextSimulator()->getInterpolationAndRenderingSetupPerCallsignOrDefault(cs);
                ui->form_InterpolationSetup->setValue(setup);
            }
        }

        void CInterpolationSetupComponent::saveSetup()
        {
            if (!this->checkPrerequisites(true)) { return; }
            CInterpolationAndRenderingSetupPerCallsign setup = ui->form_InterpolationSetup->getValue();
            if (this->getSetupMode() == CInterpolationSetupComponent::Global)
            {
                CInterpolationAndRenderingSetupGlobal gs = sGui->getIContextSimulator()->getInterpolationAndRenderingSetupGlobal();
                gs.setBaseValues(setup);
                gs.setLogInterpolation(false); // that would globally log all values
                sGui->getIContextSimulator()->setInterpolationAndRenderingSetupGlobal(gs);
                CLogMessage(this).info("Set global setup: '%1'") << gs.toQString(true);

                const QPointer<CInterpolationSetupComponent> myself(this);
                QTimer::singleShot(250, this, [ = ]
                {
                    if (myself.isNull()) { return; }
                    this->reloadSetup();
                });
            }
            else
            {
                const CCallsign cs = ui->comp_CallsignCompleter->getCallsign(false);
                if (!cs.isValid()) { return; }
                setup.setCallsign(cs);
                CInterpolationSetupList setups = ui->tvp_InterpolationSetup->container();
                const int replaced = setups.replaceOrAddObjectByCallsign(setup);
                if (replaced < 1) { return; }
                const bool set = this->setSetupsToContext(setups);
                if (!set) { return; }

                const QPointer<CInterpolationSetupComponent> myself(this);
                QTimer::singleShot(250, this, [ = ]
                {
                    if (myself.isNull()) { return; }
                    this->displaySetupsPerCallsign();
                });
            }
        }

        void CInterpolationSetupComponent::removeSetup()
        {
            if (!this->checkPrerequisites(true)) { return; }
            if (this->getSetupMode() == CInterpolationSetupComponent::Global) { return; }
            const CCallsign cs = ui->comp_CallsignCompleter->getCallsign(false);
            CInterpolationSetupList setups = ui->tvp_InterpolationSetup->container();
            const int removed = setups.removeByCallsign(cs);
            if (removed < 1) { return; } // nothing done
            const bool set = this->setSetupsToContext(setups);
            if (!set) { return; }

            const QPointer<CInterpolationSetupComponent> myself(this);
            QTimer::singleShot(100, this, [ = ]
            {
                if (myself.isNull()) { return; }
                this->displaySetupsPerCallsign();
            });
        }

        void CInterpolationSetupComponent::setUiValuesFromGlobal()
        {
            Q_ASSERT_X(sGui, Q_FUNC_INFO, "Missing sGui");
            const CInterpolationAndRenderingSetupGlobal setup = sGui->getIContextSimulator()->getInterpolationAndRenderingSetupGlobal();
            ui->form_InterpolationSetup->setValue(setup);
        }

        void CInterpolationSetupComponent::displaySetupsPerCallsign()
        {
            Q_ASSERT_X(sGui, Q_FUNC_INFO, "Missing sGui");
            const CInterpolationSetupList setups = sGui->getIContextSimulator()->getInterpolationAndRenderingSetupsPerCallsign();
            ui->tvp_InterpolationSetup->updateContainerMaybeAsync(setups);
        }

        bool CInterpolationSetupComponent::checkPrerequisites(bool showOverlay)
        {
            if (!sGui || !sGui->getIContextSimulator() || sGui->isShuttingDown())
            {
                if (showOverlay)
                {
                    const CStatusMessage m = CStatusMessage(this).validationError("No context");
                    this->showOverlayMessage(m);
                }
                return false;
            }
            if (!sGui->getIContextSimulator()->isSimulatorAvailable())
            {
                if (showOverlay)
                {
                    const CStatusMessage m = CStatusMessage(this).validationError("No simulator avialable");
                    this->showOverlayMessage(m);
                }
                return false;
            }
            return true;
        }

        bool CInterpolationSetupComponent::setSetupsToContext(const CInterpolationSetupList &setups)
        {
            if (!sGui || sGui->isShuttingDown() || !sGui->getIContextSimulator()) { return false; }
            if (setups == m_lastSetSetups) { return false; }
            sGui->getIContextSimulator()->setInterpolationAndRenderingSetupsPerCallsign(setups);
            m_lastSetSetups = setups;
            return true;
        }

        void CInterpolationSetupComponent::onSetupChanged()
        {
            this->displaySetupsPerCallsign();
        }
    } // ns
} // ns
