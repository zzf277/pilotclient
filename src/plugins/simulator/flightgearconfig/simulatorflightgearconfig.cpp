/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "simulatorflightgearconfig.h"
#include "simulatorflightgearconfigwindow.h"

class QWidget;

namespace BlackSimPlugin
{
    namespace XPlane
    {
        CSimulatorXPlaneConfig::CSimulatorXPlaneConfig(QObject *parent) : QObject(parent)
        {

        }

        BlackGui::CPluginConfigWindow *CSimulatorXPlaneConfig::createConfigWindow(QWidget *parent)
        {
            CSimulatorXPlaneConfigWindow* w = new CSimulatorXPlaneConfigWindow(parent);
            return w;
        }
    }
}