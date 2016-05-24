/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackconfig/buildconfig.h"
#include "blackgui/components/datainfoareacomponent.h"
#include "blackgui/components/datamaininfoareacomponent.h"
#include "blackgui/components/dbmappingcomponent.h"
#include "blackgui/guiapplication.h"
#include "blackmisc/icons.h"
#include "swiftdata.h"
#include "ui_swiftdata.h"

#include <QAction>
#include <QDir>
#include <QMenu>
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>

using namespace BlackConfig;
using namespace BlackGui;
using namespace BlackCore;
using namespace BlackGui::Components;
using namespace BlackMisc;

void CSwiftData::ps_onMenuClicked()
{
    QObject *sender = QObject::sender();
    if (sender == this->ui->menu_WindowFont)
    {
        // this->ps_setMainPageToInfoArea();
        // this->ui->comp_MainInfoArea->selectSettingsTab(BlackGui::Components::CSettingsComponent::SettingTabGui);
    }
    else if (sender == this->ui->menu_MappingMaxData)
    {
        CDbMappingComponent *mappingComponent = this->ui->comp_MainInfoArea->getMappingComponent();
        mappingComponent->resizeForSelect();
    }
    else if (sender == this->ui->menu_MappingMaxMapping)
    {
        CDbMappingComponent *mappingComponent = this->ui->comp_MainInfoArea->getMappingComponent();
        mappingComponent->resizeForMapping();
    }
}

void CSwiftData::initDynamicMenus()
{
    Q_ASSERT_X(this->ui->menu_InfoAreas, Q_FUNC_INFO, "missing info areas");
    Q_ASSERT_X(this->ui->comp_MainInfoArea, Q_FUNC_INFO, "missing main area");
    this->ui->menu_InfoAreas->addActions(this->ui->comp_MainInfoArea->getInfoAreaSelectActions(this->ui->menu_InfoAreas));

    QString resourceDir(CBuildConfig::getSwiftResourceDir());
    if (!resourceDir.isEmpty() && QDir(resourceDir).exists())
    {
        Q_ASSERT_X(this->ui->comp_MainInfoArea, Q_FUNC_INFO, "Missing main info area");
        Q_ASSERT_X(this->ui->comp_MainInfoArea->getDataInfoAreaComponent(), Q_FUNC_INFO, "Missing DB info area");
        this->ui->menu_Mapping->addAction(CIcons::database16(), "Load all DB data", this->ui->comp_MainInfoArea->getDataInfoAreaComponent(), SLOT(requestUpdateOfAllDbData()));
        this->ui->menu_Mapping->addAction(CIcons::load16(), "Load DB test data from disk", this->ui->comp_MainInfoArea->getDataInfoAreaComponent(), SLOT(readDbDataFromResourceDir()));
        if (sGui->isRunningInDeveloperEnvironment())
        {
            this->ui->menu_Mapping->addAction(CIcons::save16(), "Save DB test data to disk", this->ui->comp_MainInfoArea->getDataInfoAreaComponent(), SLOT(writeDbDataToResourceDir()));
        }
    }
}
