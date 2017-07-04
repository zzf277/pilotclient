/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackcore/airspaceanalyzer.h"
#include "blackmisc/aviation/aircraftsituation.h"
#include "blackmisc/aviation/callsign.h"
#include "blackmisc/aviation/transponder.h"
#include "blackmisc/logmessage.h"
#include "blackmisc/simulation/simulatedaircraftlist.h"
#include "blackmisc/statusmessage.h"
#include "blackmisc/threadutils.h"

#include <QDateTime>
#include <QMetaObject>
#include <QReadLocker>
#include <QString>
#include <QThread>
#include <QWriteLocker>

using namespace BlackMisc;
using namespace BlackMisc::Simulation;
using namespace BlackMisc::Aviation;
using namespace BlackMisc::PhysicalQuantities;

namespace BlackCore
{
    CAirspaceAnalyzer::CAirspaceAnalyzer(IOwnAircraftProvider *ownAircraftProvider, IRemoteAircraftProvider *remoteAircraftProvider, INetwork *network, QObject *parent) :
        CContinuousWorker(parent, "CAirspaceAnalyzer"),
        COwnAircraftAware(ownAircraftProvider),
        CRemoteAircraftAware(remoteAircraftProvider)
    {
        Q_ASSERT_X(network, Q_FUNC_INFO, "Network object required to connect");

        // start in thread
        this->setObjectName("CAirspaceAnalyzer");

        // all in new thread from here on
        m_timer.setObjectName(this->objectName().append(":m_timer"));
        m_timer.start(7500);
        m_lastWatchdogCallMsSinceEpoch = QDateTime::currentMSecsSinceEpoch();
        bool c = connect(&m_timer, &QTimer::timeout, this, &CAirspaceAnalyzer::ps_timeout);
        Q_ASSERT(c);

        // disconnect
        c = connect(network, &INetwork::pilotDisconnected, this, &CAirspaceAnalyzer::ps_watchdogRemoveAircraftCallsign);
        Q_ASSERT(c);
        c = connect(network, &INetwork::atcDisconnected, this, &CAirspaceAnalyzer::ps_watchdogRemoveAtcCallsign);
        Q_ASSERT(c);

        // update
        c = connect(network, &INetwork::aircraftPositionUpdate, this, &CAirspaceAnalyzer::ps_watchdogTouchAircraftCallsign);
        Q_ASSERT(c);
        c = connect(network, &INetwork::atcPositionUpdate, this, &CAirspaceAnalyzer::ps_watchdogTouchAtcCallsign);
        Q_ASSERT(c);

        // network
        c = connect(network, &INetwork::connectionStatusChanged, this, &CAirspaceAnalyzer::ps_onConnectionStatusChanged);
        Q_ASSERT(c);
        Q_UNUSED(c);

        // start in own thread
        this->start(QThread::LowestPriority);
    }

    CAirspaceAircraftSnapshot CAirspaceAnalyzer::getLatestAirspaceAircraftSnapshot() const
    {
        QReadLocker l(&m_lockSnapshot);
        return m_latestAircraftSnapshot;
    }

    void CAirspaceAnalyzer::setSimulatorRenderRestrictionsChanged(bool restricted, bool enabled, int maxAircraft, const CLength &maxRenderedDistance)
    {
        QWriteLocker l(&m_lockRestrictions);
        this->m_simulatorRenderedAircraftRestricted = restricted;
        this->m_simulatorRenderingEnabled = enabled;
        this->m_simulatorMaxRenderedAircraft = maxAircraft;
        this->m_simulatorMaxRenderedDistance = maxRenderedDistance;
    }

    void CAirspaceAnalyzer::gracefulShutdown()
    {
        const bool s = QMetaObject::invokeMethod(&m_timer, "stop");
        Q_ASSERT_X(s, Q_FUNC_INFO, "invoke failed");
        Q_UNUSED(s);
    }

    CAirspaceAnalyzer::~CAirspaceAnalyzer()
    {
        gracefulShutdown();
    }

    void CAirspaceAnalyzer::ps_watchdogTouchAircraftCallsign(const CAircraftSituation &situation, const CTransponder &transponder)
    {
        Q_ASSERT_X(!situation.getCallsign().isEmpty(), Q_FUNC_INFO, "No callsign in situaton");
        Q_UNUSED(transponder);
        m_aircraftCallsignTimestamps[situation.getCallsign()] = QDateTime::currentMSecsSinceEpoch();
    }

    void CAirspaceAnalyzer::ps_watchdogTouchAtcCallsign(const CCallsign &callsign, const CFrequency &frequency, const Geo::CCoordinateGeodetic &position, const CLength &range)
    {
        Q_UNUSED(frequency);
        Q_UNUSED(position);
        Q_UNUSED(range);
        m_atcCallsignTimestamps[callsign] = QDateTime::currentMSecsSinceEpoch();
    }

    void CAirspaceAnalyzer::ps_onConnectionStatusChanged(BlackCore::INetwork::ConnectionStatus oldStatus, BlackCore::INetwork::ConnectionStatus newStatus)
    {
        Q_UNUSED(oldStatus);
        if (newStatus == INetwork::Disconnected)
        {
            this->clear();
            this->m_timer.stop();
        }
        else if (newStatus == INetwork::Connected)
        {
            this->m_timer.start();
        }
    }

    void CAirspaceAnalyzer::ps_timeout()
    {
        this->analyzeAirspace();
        this->watchdogCheckTimeouts();
    }

    void CAirspaceAnalyzer::clear()
    {
        m_aircraftCallsignTimestamps.clear();
        m_atcCallsignTimestamps.clear();

        QWriteLocker l(&m_lockSnapshot);
        m_latestAircraftSnapshot = CAirspaceAircraftSnapshot();
    }

    void CAirspaceAnalyzer::ps_watchdogRemoveAircraftCallsign(const CCallsign &callsign)
    {
        m_aircraftCallsignTimestamps.remove(callsign);
    }

    void CAirspaceAnalyzer::ps_watchdogRemoveAtcCallsign(const CCallsign &callsign)
    {
        m_atcCallsignTimestamps.remove(callsign);
    }

    void CAirspaceAnalyzer::watchdogCheckTimeouts()
    {
        qint64 currentTimeMsEpoch = QDateTime::currentMSecsSinceEpoch();

        qint64 callDiffMs = currentTimeMsEpoch - m_lastWatchdogCallMsSinceEpoch;
        qint64 callThresholdMs = static_cast<int>(m_timer.interval() * 1.5);
        m_lastWatchdogCallMsSinceEpoch = currentTimeMsEpoch;

        // this is a trick to not remove everything while debugging
        if (callDiffMs > callThresholdMs) { return; }

        qint64 aircraftTimeoutMs = m_timeoutAircraft.valueInteger(CTimeUnit::ms());
        qint64 atcTimeoutMs = m_timeoutAtc.valueInteger(CTimeUnit::ms());
        qint64 timeoutAircraftEpochMs = currentTimeMsEpoch - aircraftTimeoutMs;
        qint64 timeoutAtcEpochMs = currentTimeMsEpoch - atcTimeoutMs;

        for (const CCallsign &callsign : m_aircraftCallsignTimestamps.keys()) // clazy:exclude=container-anti-pattern,range-loop
        {
            if (m_aircraftCallsignTimestamps.value(callsign) > timeoutAircraftEpochMs) { continue; }
            CLogMessage(this).debug() << "Aircraft " << callsign.toQString() << "timed out!";
            m_aircraftCallsignTimestamps.remove(callsign);
            emit timeoutAircraft(callsign);
        }

        for (const CCallsign &callsign : m_atcCallsignTimestamps.keys()) // clazy:exclude=container-anti-pattern,range-loop
        {
            if (m_atcCallsignTimestamps.value(callsign) > timeoutAtcEpochMs) { continue; }
            CLogMessage(this).debug() << "ATC " << callsign.toQString() << "timed out!";
            m_atcCallsignTimestamps.remove(callsign);
            emit timeoutAtc(callsign);
        }
    }

    void CAirspaceAnalyzer::analyzeAirspace()
    {
        Q_ASSERT_X(!CThreadUtils::isCurrentThreadApplicationThread(), Q_FUNC_INFO, "Expect to run in background thread");
        Q_ASSERT_X(!CThreadUtils::isApplicationThreadObjectThread(this), Q_FUNC_INFO, "Expect to run in background thread affinity");

        bool restricted, enabled;
        int maxAircraft;
        CLength maxRenderedDistance;
        {
            QReadLocker l(&m_lockRestrictions);
            restricted = this->m_simulatorRenderedAircraftRestricted;
            enabled = this->m_simulatorRenderingEnabled,
            maxAircraft = this->m_simulatorMaxRenderedAircraft;
            maxRenderedDistance = this->m_simulatorMaxRenderedDistance;
        }

        // remark for simulation snapshot is used when there are restrictions
        // nevertheless we calculate all the time as the snapshot could be used in other scenarios

        CSimulatedAircraftList aircraftInRange(getAircraftInRange()); // thread safe copy from provider
        CAirspaceAircraftSnapshot snapshot(
            aircraftInRange,
            restricted, enabled,
            maxAircraft, maxRenderedDistance
        );

        // lock block
        {
            QWriteLocker l(&m_lockSnapshot);
            bool wasValid = m_latestAircraftSnapshot.isValidSnapshot();
            if (wasValid)
            {
                snapshot.setRestrictionChanged(m_latestAircraftSnapshot);
            }
            m_latestAircraftSnapshot = snapshot;
            if (!wasValid) { return; } // ignore the 1st snapshot
        }

        emit airspaceAircraftSnapshot(snapshot);
    }

} // ns
