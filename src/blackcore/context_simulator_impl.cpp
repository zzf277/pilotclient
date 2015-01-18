/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "context_simulator_impl.h"
#include "context_ownaircraft.h"
#include "context_settings.h"
#include "context_application.h"
#include "context_network_impl.h"
#include "context_runtime.h"
#include "blackmisc/propertyindexvariantmap.h"
#include "blackmisc/logmessage.h"
#include "blackmisc/loghandler.h"
#include <QPluginLoader>
#include <QLibrary>

using namespace BlackMisc;
using namespace BlackMisc::PhysicalQuantities;
using namespace BlackMisc::Aviation;
using namespace BlackMisc::Network;
using namespace BlackMisc::Simulation;
using namespace BlackMisc::Geo;
using namespace BlackSim;
using namespace BlackSim::Settings;

namespace BlackCore
{
    CContextSimulator::CContextSimulator(CRuntimeConfig::ContextMode mode, CRuntime *runtime) : IContextSimulator(mode, runtime)
    {
        m_updateTimer = new QTimer(this);
        findSimulatorPlugins();
        connect(m_updateTimer, &QTimer::timeout, this, &CContextSimulator::ps_updateOwnAircraftContext);

        // do not load plugin here, as it depends on settings
        // it has to be guaranteed the settings are alredy loaded
    }

    CContextSimulator::~CContextSimulator()
    {
        disconnectFrom();
        unloadSimulatorPlugin();
    }

    CSimulatorInfoList CContextSimulator::getAvailableSimulatorPlugins() const
    {
        CSimulatorInfoList simulatorPlugins;
        foreach(ISimulatorFactory * factory, m_simulatorFactories)
        {
            simulatorPlugins.push_back(factory->getSimulatorInfo());
        }
        simulatorPlugins.sortBy(&CSimulatorInfo::getShortName);
        return simulatorPlugins;
    }

    bool CContextSimulator::isConnected() const
    {
        CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO;
        if (!m_simulator) return false;
        return m_simulator->isConnected();
    }

    bool CContextSimulator::canConnect() const
    {
        CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO;
        if (!m_simulator) return false;
        return m_simulator->canConnect();
    }

    bool CContextSimulator::connectTo()
    {
        CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO;
        if (!m_simulator) return false;
        return m_simulator->connectTo();
    }

    void CContextSimulator::asyncConnectTo()
    {
        CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO;
        if (!m_simulator || m_canConnectResult.isRunning()) return; // already checking
        m_simulator->asyncConnectTo();
    }

    bool CContextSimulator::disconnectFrom()
    {
        CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO;
        if (!m_simulator) return false;
        return m_simulator->disconnectFrom();
    }

    BlackSim::CSimulatorInfo CContextSimulator::getSimulatorInfo() const
    {
        CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO;
        if (!m_simulator) return BlackSim::CSimulatorInfo::UnspecifiedSim();
        return m_simulator->getSimulatorInfo();
    }

    Simulation::CAircraftModel CContextSimulator::getOwnAircraftModel() const
    {
        CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO;
        // If no ISimulator object is available, return a dummy.
        if (!m_simulator) { return Simulation::CAircraftModel(); }

        return this->m_simulator->getOwnAircraftModel();
    }

    CAirportList CContextSimulator::getAirportsInRange() const
    {
        CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO;
        // If no ISimulator object is available, return a dummy.
        if (!m_simulator) { return CAirportList(); }

        return this->m_simulator->getAirportsInRange();
    }

    CAircraftModelList CContextSimulator::getInstalledModels() const
    {
        CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO;
        // If no ISimulator object is available, return a dummy.
        if (!m_simulator) { return CAircraftModelList(); }

        return this->m_simulator->getInstalledModels();
    }

    CSimulatedAircraftList CContextSimulator::getRemoteAircraft() const
    {
        CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO;
        // If no ISimulator object is available, return a dummy.
        if (!m_simulator) { return CSimulatedAircraftList(); }

        return this->m_simulator->getRemoteAircraft();
    }

    int CContextSimulator::changeRemoteAircraft(const CSimulatedAircraft &changedAircraft, const CPropertyIndexVariantMap &changedValues)
    {
        CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO;
        if (!m_simulator) { return 0; }
        int c = this->m_simulator->changeRemoteAircraft(changedAircraft, changedValues);
        if (c > 0)
        {
            // really changed something
            emit this->remoteAircraftChanged(changedAircraft);
        }
        return c;
    }

    void CContextSimulator::setTimeSynchronization(bool enable, CTime offset)
    {
        CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO;
        if (!m_simulator) return;
        this->m_simulator->setTimeSynchronization(enable, offset);
    }

    bool CContextSimulator::isTimeSynchronized() const
    {
        CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO;
        if (!m_simulator) return false;
        return this->m_simulator->isTimeSynchronized();
    }

    int CContextSimulator::getMaxRenderedRemoteAircraft() const
    {
        CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO;
        if (!m_simulator) return 0;
        return 13;
    }

    void CContextSimulator::setMaxRenderedRemoteAircraft(int number)
    {
        CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO;
        //! \todo implement
        Q_UNUSED(number);
    }

    CTime CContextSimulator::getTimeSynchronizationOffset() const
    {
        CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO;
        if (!m_simulator) return CTime(0, CTimeUnit::hrmin());
        return this->m_simulator->getTimeSynchronizationOffset();
    }

    bool CContextSimulator::loadSimulatorPlugin(const CSimulatorInfo &simulatorInfo)
    {
        Q_ASSERT(this->getIContextApplication());
        Q_ASSERT(this->getIContextApplication()->isUsingImplementingObject());

        if (this->m_simulator && this->m_simulator->getSimulatorInfo() == simulatorInfo) { return true; } // already loaded
        if (simulatorInfo.isUnspecified()) { return false; }

        // warning if we do not have any plugins
        if (m_simulatorFactories.isEmpty())
        {
            CLogMessage(this).error("No simulator plugins");
            return false;
        }

        auto iterator = std::find_if(m_simulatorFactories.begin(), m_simulatorFactories.end(), [ = ](const ISimulatorFactory * factory)
        {
            return factory->getSimulatorInfo() == simulatorInfo;
        });

        // no plugin found
        if (iterator == m_simulatorFactories.end())
        {
            CLogMessage(this).error("Plugin not found: '%1'") << simulatorInfo.toQString(true);
            return false;
        }

        ISimulatorFactory *factory = *iterator;
        Q_ASSERT(factory);

        ISimulator *newSimulator = factory->create(this);
        Q_ASSERT(newSimulator);

        this->unloadSimulatorPlugin(); // old plugin unloaded
        m_simulator = newSimulator;

        connect(m_simulator, &ISimulator::connectionStatusChanged, this, &CContextSimulator::ps_onConnectionStatusChanged);
        connect(m_simulator, &ISimulator::simulatorStatusChanged, this, &CContextSimulator::simulatorStatusChanged);
        connect(m_simulator, &ISimulator::ownAircraftModelChanged, this, &IContextSimulator::ownAircraftModelChanged);
        connect(m_simulator, &ISimulator::modelMatchingCompleted, this, &IContextSimulator::modelMatchingCompleted);
        connect(m_simulator, &ISimulator::installedAircraftModelsChanged, this, &IContextSimulator::installedAircraftModelsChanged);

        // log from context to simulator
        connect(CLogHandler::instance(), &CLogHandler::localMessageLogged, m_simulator, &ISimulator::displayStatusMessage);
        connect(CLogHandler::instance(), &CLogHandler::remoteMessageLogged, m_simulator, &ISimulator::displayStatusMessage);

        // connect with network
        //! \todo using airspace monitor directly is maybe wrong (KB: I think it is). It assumes simulator and network
        //!       context reside in the same process, which is not guranteed IMHO
        CAirspaceMonitor *airspace = this->getRuntime()->getCContextNetwork()->getAirspaceMonitor();
        // use readyForModelMatching instead of CAirspaceMonitor::addedAircraft, as it contains client information
        connect(airspace, &CAirspaceMonitor::readyForModelMatching, this, &CContextSimulator::ps_addRemoteAircraft);
        connect(airspace, &CAirspaceMonitor::changedAircraftSituation, this, &CContextSimulator::ps_addAircraftSituation);
        connect(airspace, &CAirspaceMonitor::removedAircraft, this, &CContextSimulator::ps_removeRemoteAircraft);
        for (const auto &aircraft : airspace->getAircraftInRange())
        {
            Q_ASSERT(!aircraft.getCallsign().isEmpty());
            CClient client = airspace->getOtherClients().findFirstByCallsign(aircraft.getCallsign());
            CSimulatedAircraft simAircraft(aircraft);
            simAircraft.setClient(client);
            m_simulator->addRemoteAircraft(simAircraft);
        }

        // apply latest settings
        this->settingsChanged(static_cast<uint>(IContextSettings::SettingsSimulator));

        // try to connect
        asyncConnectTo();

        // info about what is going on
        CLogMessage(this).info("Simulator plugin loaded: '%1'") << this->m_simulator->getSimulatorInfo().toQString(true);
        return true;
    }

    bool CContextSimulator::loadSimulatorPluginFromSettings()
    {
        Q_ASSERT(this->getIContextSettings());
        if (!this->getIContextSettings()) return false;

        // TODO warnings if we didn't load the plugin which the settings asked for

        CSimulatorInfoList plugins = this->getAvailableSimulatorPlugins();
        if (plugins.size() == 1)
        {
            // load, independent from settings, we have only driver
            return this->loadSimulatorPlugin(plugins.front());
        }
        else if (plugins.size() > 1)
        {
            if (this->loadSimulatorPlugin(
                        this->getIContextSettings()->getSimulatorSettings().getSelectedPlugin()
                    )) return true;

            // we have plugins, but none got loaded
            // just load first one
            return this->loadSimulatorPlugin(plugins.front());
        }
        else
        {
            return false;
        }
    }

    void CContextSimulator::unloadSimulatorPlugin()
    {
        if (m_simulator)
        {
            // depending on shutdown order, network might already have been deleted
            //! \todo link with airspace monitor to be reviewed when airspace monitor becomes context
            if (this->getRuntime()->getIContextNetwork()->isUsingImplementingObject())
            {
                CContextNetwork *network = this->getRuntime()->getCContextNetwork();
                network->getAirspaceMonitor()->QObject::disconnect(this);
            }

            this->QObject::disconnect(m_simulator); // disconnect as receiver straight away
            m_simulator->disconnectFrom(); // disconnect from simulator
            m_simulator->deleteLater();
        }
        m_simulator = nullptr;
    }

    void CContextSimulator::ps_updateOwnAircraftContext()
    {
        Q_ASSERT(this->getIContextOwnAircraft());
        Q_ASSERT(this->m_simulator);

        // we make sure not to override values we do not have
        CAircraft contextAircraft = this->getIContextOwnAircraft()->getOwnAircraft(); // own aircraft from context
        CAircraft simulatorAircraft = this->m_simulator->getOwnAircraft();            // own aircraft from simulator

        // update from simulator to context
        contextAircraft.setSituation(simulatorAircraft.getSituation());
        contextAircraft.setCockpit(simulatorAircraft.getCom1System(), simulatorAircraft.getCom2System(), simulatorAircraft.getTransponderCode(), simulatorAircraft.getTransponderMode());

        Q_ASSERT(this->getIContextOwnAircraft()); // paranoia against context having been deleted from another thread - redmine issue #270
        if (this->getIContextOwnAircraft())
        {
            // the method will check, if an update is really required
            // these are local (non DBus) calls
            this->getIContextOwnAircraft()->updateAircraft(contextAircraft, this->getPathAndContextId());
        }
    }

    void CContextSimulator::ps_addRemoteAircraft(const CSimulatedAircraft &remoteAircraft)
    {
        Q_ASSERT(this->m_simulator);
        Q_ASSERT(!remoteAircraft.getCallsign().isEmpty());
        if (!this->m_simulator) { return; }
        this->m_simulator->addRemoteAircraft(remoteAircraft);
    }

    void CContextSimulator::ps_addAircraftSituation(const CCallsign &callsign, const CAircraftSituation &situation)
    {
        Q_ASSERT(this->m_simulator);
        if (!this->m_simulator) { return; }
        this->m_simulator->addAircraftSituation(callsign, situation);
    }

    void CContextSimulator::ps_removeRemoteAircraft(const CCallsign &callsign)
    {
        Q_ASSERT(this->m_simulator);
        if (!this->m_simulator) return;
        this->m_simulator->removeRemoteAircraft(callsign);
    }

    void CContextSimulator::ps_updateSimulatorCockpitFromContext(const CAircraft &ownAircraft, const QString &originator)
    {
        Q_ASSERT(this->m_simulator);
        if (!this->m_simulator) { return; }

        // avoid loops
        if (originator.isEmpty() || originator == IContextSimulator::InterfaceName()) return;

        // update
        this->m_simulator->updateOwnSimulatorCockpit(ownAircraft);
    }

    void CContextSimulator::ps_onConnectionStatusChanged(ISimulator::ConnectionStatus status)
    {
        bool connected;
        if (status == ISimulator::Connected)
        {
            connected = true;
            m_updateTimer->start(100);
        }
        else
        {
            connected = false;
            m_updateTimer->stop();
        }
        emit connectionChanged(connected);
    }

    void CContextSimulator::ps_textMessagesReceived(const Network::CTextMessageList &textMessages)
    {
        if (!this->m_simulator) return;
        foreach(CTextMessage tm, textMessages)
        {
            this->m_simulator->displayTextMessage(tm);
        }
    }

    void CContextSimulator::settingsChanged(uint type)
    {
        Q_ASSERT(this->getIContextSettings());
        Q_ASSERT(this->m_simulator);
        if (!this->getIContextSettings()) return;
        auto settingsType = static_cast<IContextSettings::SettingsType>(type);
        if (settingsType != IContextSettings::SettingsSimulator) return;

        // plugin
        CSettingsSimulator settingsSim = this->getIContextSettings()->getSimulatorSettings();
        CSimulatorInfo plugin = settingsSim.getSelectedPlugin();
        if (!this->getSimulatorInfo().isSameSimulator(plugin))
        {
            if (this->loadSimulatorPlugin(plugin))
            {
                CLogMessage(this).info("Plugin loaded: '%1'") << plugin.toQString(true);
            }
            else
            {
                CLogMessage(this).error("Cannot load driver: '%1'") << plugin.toQString(true);
            }
        }

        // time sync
        bool timeSync = settingsSim.isTimeSyncEnabled();
        CTime syncOffset = settingsSim.getSyncTimeOffset();
        this->m_simulator->setTimeSynchronization(timeSync, syncOffset);
    }

    CPixmap CContextSimulator::iconForModel(const QString &modelString) const
    {
        if (!this->m_simulator) { return CPixmap(); }
        return this->m_simulator->iconForModel(modelString);
    }

    bool CContextSimulator::isPaused() const
    {
        if (!this->m_simulator) return false;
        return this->m_simulator->isPaused();
    }

    bool CContextSimulator::isSimulating() const
    {
        if (!this->m_simulator) return false;
        return this->m_simulator->isSimulating();
    }

    void CContextSimulator::findSimulatorPlugins()
    {
        const QString path = qApp->applicationDirPath().append("/plugins/simulator");
        m_pluginsDir = QDir(path);
        if (!m_pluginsDir.exists())
        {
            CLogMessage(this).error("No plugin directory: %1") << m_pluginsDir.currentPath();
            return;
        }

        QStringList fileNames = m_pluginsDir.entryList(QDir::Files);
        fileNames.sort(Qt::CaseInsensitive); // give a certain order, rather than random file order
        foreach(QString fileName, fileNames)
        {
            if (!QLibrary::isLibrary(fileName)) { continue; }
            CLogMessage(this).info("Try to load plugin: %1") << fileName;
            QString pluginPath = m_pluginsDir.absoluteFilePath(fileName);
            QPluginLoader loader(pluginPath);
            QObject *plugin = loader.instance();
            if (plugin)
            {
                ISimulatorFactory *factory = qobject_cast<ISimulatorFactory *>(plugin);
                if (factory)
                {
                    CSimulatorInfo simulatorInfo = factory->getSimulatorInfo();
                    m_simulatorFactories.insert(factory);
                    CLogMessage(this).info("Loaded plugin: %1") << simulatorInfo.toQString();
                }
            }
            else
            {
                QString errorMsg = loader.errorString().append(" ").append("Also check if required dll/libs of plugin exists");
                CLogMessage(this).error(errorMsg);
            }
        }
    }

} // namespace
