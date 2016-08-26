/* Copyright (C) 2016
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackcore/webdataservices.h"
#include "blackgui/editors/ownmodelsetform.h"
#include "blackgui/models/distributorlistmodel.h"
#include "blackgui/guiapplication.h"
#include "blackgui/guiutility.h"
#include "blackgui/views/distributorview.h"
#include "blackmisc/simulation/distributorlist.h"
#include "ui_ownmodelsetform.h"

#include <QRadioButton>
#include <QtGlobal>

using namespace BlackMisc::Simulation;
using namespace BlackGui::Models;
using namespace BlackGui::Components;

namespace BlackGui
{
    namespace Editors
    {
        COwnModelSetForm::COwnModelSetForm(QWidget *parent) :
            CForm(parent),
            ui(new Ui::COwnModelSetForm)
        {
            ui->setupUi(this);
            ui->tvp_Distributors->setDistributorMode(CDistributorListModel::Minimal);
            ui->comp_SimulatorSelector->setMode(CSimulatorSelector::RadioButtons);
            ui->comp_SimulatorSelector->setLeftMargin(0);

            connect(ui->comp_SimulatorSelector, &CSimulatorSelector::changed, this, &COwnModelSetForm::ps_simulatorChanged);
            connect(ui->rb_DisplayAllDistributors, &QRadioButton::clicked, this, &COwnModelSetForm::ps_changeDistributorDisplay);
            connect(ui->rb_DisplayPreferencesDistributors, &QRadioButton::clicked, this, &COwnModelSetForm::ps_changeDistributorDisplay);

            this->ps_simulatorChanged(ui->comp_SimulatorSelector->getValue());
        }

        COwnModelSetForm::~COwnModelSetForm()
        {
            // void
        }

        void COwnModelSetForm::reloadData()
        {
            const bool hasPreferences = this->hasDistributorPreferences();
            ui->cb_SortByPreferences->setChecked(hasPreferences);
            CGuiUtility::checkBoxReadOnly(ui->cb_SortByPreferences, !hasPreferences);
            ui->comp_SimulatorSelector->setValue(this->m_simulator);
            this->setDistributorView(hasPreferences);
            this->initDistributorDisplay();
        }

        bool COwnModelSetForm::useSelectedDistributors() const
        {
            return this->ui->rb_SelectedDistributors->isChecked();
        }

        bool COwnModelSetForm::dbDataOnly() const
        {
            return this->ui->rb_DbDataOnly->isChecked();
        }

        bool COwnModelSetForm::incrementalBuild() const
        {
            return ui->rb_Incremental->isChecked();
        }

        void COwnModelSetForm::ps_preferencesChanged()
        {
            // void
        }

        void COwnModelSetForm::ps_simulatorChanged(const CSimulatorInfo &simulator)
        {
            this->setSimulator(simulator);
            this->reloadData();
            emit simulatorChanged(simulator);
        }

        void COwnModelSetForm::ps_changeDistributorDisplay()
        {
            if (ui->rb_DisplayAllDistributors->isChecked())
            {
                ui->tvp_Distributors->updateContainerMaybeAsync(this->getAllDistributors());
                ui->cb_SortByPreferences->setChecked(false);
                CGuiUtility::checkBoxReadOnly(ui->cb_SortByPreferences, true);
                this->setDistributorView(false);
            }
            else
            {
                ui->tvp_Distributors->updateContainerMaybeAsync(this->getDistributorsFromPreferences());
                ui->cb_SortByPreferences->setChecked(true);
                CGuiUtility::checkBoxReadOnly(ui->cb_SortByPreferences, false);
                this->setDistributorView(true);
            }
        }

        void COwnModelSetForm::initDistributorDisplay()
        {
            if (this->hasDistributorPreferences())
            {
                ui->rb_DisplayPreferencesDistributors->setChecked(true);
            }
            else
            {
                ui->rb_DisplayAllDistributors->setChecked(true);
            }
            this->ps_changeDistributorDisplay();
        }

        void COwnModelSetForm::setDistributorView(bool hasPreferences)
        {
            ui->tvp_Distributors->setDistributorMode(hasPreferences ? CDistributorListModel::MinimalWithOrder : CDistributorListModel::Minimal);
            ui->tvp_Distributors->fullResizeToContents();
        }

        CDistributorList COwnModelSetForm::getDistributorsFromPreferences() const
        {
            Q_ASSERT_X(this->m_simulator.isSingleSimulator(), Q_FUNC_INFO, "Need single simulator");
            const CDistributorListPreferences prefs(this->m_distributorPreferences.get());
            const CDistributorList distributors(prefs.getDistributors(this->m_simulator));
            return distributors;
        }

        CDistributorList COwnModelSetForm::getAllDistributors() const
        {
            Q_ASSERT_X(this->m_simulator.isSingleSimulator(), Q_FUNC_INFO, "Need single simulator");
            Q_ASSERT_X(sGui && sGui->hasWebDataServices(), Q_FUNC_INFO, "Missing web data services");
            return sGui->getWebDataServices()->getDistributors().matchesSimulator(this->m_simulator);
        }

        bool COwnModelSetForm::dbIcaoCodesOnly() const
        {
            return this->ui->rb_DbIcaoCodesOnly->isChecked();
        }

        CDistributorList COwnModelSetForm::getSelectedDistributors() const
        {
            return ui->tvp_Distributors->selectedObjects();
        }

        void COwnModelSetForm::setSimulator(const CSimulatorInfo &simulator)
        {
            Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "Need single simulator");
            m_simulator = simulator;
        }

        bool COwnModelSetForm::hasDistributorPreferences() const
        {
            const CDistributorListPreferences prefs(this->m_distributorPreferences.get());
            return !prefs.getDistributors(this->m_simulator).isEmpty();
        }
    } // ns
} // ns
