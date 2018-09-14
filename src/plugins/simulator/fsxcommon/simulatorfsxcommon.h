/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKSIMPLUGIN_FSXCOMMON_SIMULATORFSXCOMMON_H
#define BLACKSIMPLUGIN_FSXCOMMON_SIMULATORFSXCOMMON_H

#include "simconnectdatadefinition.h"
#include "simconnectobject.h"
#include "../fsxcommon/simconnectwindows.h"
#include "../fscommon/simulatorfscommon.h"
#include "blackcore/simulator.h"
#include "blackmisc/simulation/interpolatorlinear.h"
#include "blackmisc/simulation/simulatorplugininfo.h"
#include "blackmisc/simulation/settings/simulatorsettings.h"
#include "blackmisc/simulation/aircraftmodel.h"
#include "blackmisc/simulation/simulatedaircraft.h"
#include "blackmisc/aviation/airportlist.h"
#include "blackmisc/aviation/altitude.h"
#include "blackmisc/statusmessage.h"
#include "blackmisc/network/client.h"
#include "blackmisc/pixmap.h"

#include <QObject>
#include <QtPlugin>
#include <QHash>
#include <QList>
#include <QFutureWatcher>

namespace BlackSimPlugin
{
    namespace FsxCommon
    {
        //! SimConnect Event IDs
        enum EventIds
        {
            SystemEventSimStatus,
            SystemEventObjectAdded,
            SystemEventObjectRemoved,
            SystemEventSlewToggle,
            SystemEventFrame,
            SystemEventPause,
            SystemEventFlightLoaded,
            EventPauseToggle,
            EventFreezeLat,
            EventFreezeAlt,
            EventFreezeAtt,
            EventSetCom1Active,
            EventSetCom2Active,
            EventSetCom1Standby,
            EventSetCom2Standby,
            EventSetTransponderCode,
            EventTextMessage,
            EventSetTimeZuluYear,
            EventSetTimeZuluDay,
            EventSetTimeZuluHours,
            EventSetTimeZuluMinutes,
            // ------------ lights -------------
            EventLandingLightsOff,
            EventLandinglightsOn,
            EventLandingLightsSet,
            EventLandingLightsToggle,
            EventPanelLightsOff,
            EventPanelLightsOn,
            EventPanelLightsSet,
            EventStrobesOff,
            EventStrobesOn,
            EventStrobesSet,
            EventStrobesToggle,
            EventToggleBeaconLights,
            EventToggleCabinLights,
            EventToggleLogoLights,
            EventToggleNavLights,
            EventToggleRecognitionLights,
            EventToggleTaxiLights,
            EventToggleWingLights,
            EventFSXEndMarker
        };

        //! Struct to trace send ids
        struct TraceFsxSendId
        {
            //! Ctor
            TraceFsxSendId(DWORD sendId, const CSimConnectObject &simObject, const QString &comment) :
                sendId(sendId), simObject(simObject), comment(comment)
            { }

            // DWORD is unsigned
            DWORD sendId = 0;            //!< the send id
            CSimConnectObject simObject; //!< CSimConnectObject at the time of the trace
            QString comment;             //!< where sent

            //! For probe
            bool isForProbe() const { return simObject.getType() == CSimConnectObject::TerrainProbe; }

            //! For aircraft
            bool isForAircraft() const { return simObject.getType() == CSimConnectObject::Aircraft; }

            //! Invalid trace?
            bool isInvalid() const { return sendId == 0 && simObject.isInvalid() == 0 && comment.isEmpty(); }

            //! Valid trace?
            bool isValid() const { return !this->isInvalid(); }

            //! Invalid object
            static const TraceFsxSendId &invalid() { static const TraceFsxSendId i(0, CSimConnectObject(), ""); return i; }
        };

        //! FSX Simulator Implementation
        class CSimulatorFsxCommon : public FsCommon::CSimulatorFsCommon
        {
            Q_OBJECT
            Q_INTERFACES(BlackCore::ISimulator)

        public:
            //! Constructor, parameters as in \sa BlackCore::ISimulatorFactory::create
            CSimulatorFsxCommon(const BlackMisc::Simulation::CSimulatorPluginInfo &info,
                                BlackMisc::Simulation::IOwnAircraftProvider *ownAircraftProvider,
                                BlackMisc::Simulation::IRemoteAircraftProvider *remoteAircraftProvider,
                                BlackMisc::Weather::IWeatherGridProvider *weatherGridProvider,
                                BlackMisc::Network::IClientProvider *clientProvider,
                                QObject *parent = nullptr);

            //! Destructor
            virtual ~CSimulatorFsxCommon() override;

            //! \name ISimulator implementations
            //! @{
            virtual bool connectTo() override;
            virtual bool disconnectFrom() override;
            virtual bool physicallyAddRemoteAircraft(const BlackMisc::Simulation::CSimulatedAircraft &newRemoteAircraft) override;
            virtual bool physicallyRemoveRemoteAircraft(const BlackMisc::Aviation::CCallsign &callsign) override;
            virtual int  physicallyRemoveAllRemoteAircraft() override;
            virtual bool updateOwnSimulatorCockpit(const BlackMisc::Simulation::CSimulatedAircraft &ownAircraft, const BlackMisc::CIdentifier &originator) override;
            virtual bool updateOwnSimulatorSelcal(const BlackMisc::Aviation::CSelcal &selcal, const BlackMisc::CIdentifier &originator) override;
            virtual void displayStatusMessage(const BlackMisc::CStatusMessage &message) const override;
            virtual void displayTextMessage(const BlackMisc::Network::CTextMessage &message) const override;
            virtual bool isPhysicallyRenderedAircraft(const BlackMisc::Aviation::CCallsign &callsign) const override;
            virtual BlackMisc::Aviation::CCallsignSet physicallyRenderedAircraft() const override;
            virtual void clearAllRemoteAircraftData() override;
            virtual BlackMisc::CStatusMessageList debugVerifyStateAfterAllAircraftRemoved() const override;
            virtual QString getStatisticsSimulatorSpecific() const override;
            virtual void resetAircraftStatistics() override;
            virtual void setFlightNetworkConnected(bool connected) override;
            //! @}

            //! \copydoc BlackMisc::Simulation::ISimulationEnvironmentProvider::requestElevation
            //! \remark x86 FSX version, x64 version is overridden
            //! \sa CSimulatorFsxCommon::is
            virtual bool requestElevation(const BlackMisc::Geo::ICoordinateGeodetic &reference, const BlackMisc::Aviation::CCallsign &aircraftCallsign) override;

            //! Tracing?
            bool isTracingSendId() const;

            //! Set tracing on/off
            void setTractingSendId(bool trace);

            //! FSX Terrain probe
            //! \remark must be off at P3D v4.2 drivers or later
            bool isUsingFsxTerrainProbe() const { return m_useFsxTerrainProbe; }

            //! FSX terrain probe
            void setUsingFsxTerrainProbe(bool use) { m_useFsxTerrainProbe = use; }

            //! Request for sim data (request in range of sim data)?
            static bool isRequestForSimObjAircraft(DWORD requestId) { return requestId >= RequestSimObjAircraftStart && requestId <= RequestSimObjAircraftRangeEnd; }

            //! Request for probe (elevation)?
            static bool isRequestForSimObjTerrainProbe(DWORD requestId) { return requestId >= RequestSimObjTerrainProbeStart && requestId <= RequestSimObjTerrainProbeRangeEnd; }

            //! Request for any CSimConnectObject?
            static bool isRequestForSimConnectObject(DWORD requestId)
            {
                return isRequestForSimObjAircraft(requestId) || isRequestForSimObjTerrainProbe(requestId);
            }

            //! Sub request type
            static CSimConnectDefinitions::SimObjectRequest requestToSimObjectRequest(DWORD requestId);

            //! Random unit text request id
            //! \private
            static DWORD unitTestRequestId(CSimConnectObject::SimObjectType type);

            //! Encapsulates creating QString from FSX string data
            static QString fsxCharToQString(const char *fsxChar, int size = -1);

        protected:
            //! SimConnect callback
            //! \note all tasks called in this function (i.e, all called functions) must perform fast or shall be called asynchronously
            static void CALLBACK SimConnectProc(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext);

            //! \name Interface implementations
            //! @{
            virtual bool isConnected() const override;
            virtual bool isSimulating() const override;
            //! @}

            //! \name Base class overrides
            //! @{
            virtual void reset() override;
            virtual void initSimulatorInternals() override;
            virtual void injectWeatherGrid(const BlackMisc::Weather::CWeatherGrid &weatherGrid) override;
            //! @}

            //! Timer event (our SimConnect event loop), runs dispatch
            //! \sa m_timerId
            //! \sa CSimulatorFsxCommon::dispatch
            virtual void timerEvent(QTimerEvent *event) override;

            //! Specific P3D events
            virtual HRESULT initEventsP3D();

            //! \addtogroup swiftdotcommands
            //! @{
            //! <pre>
            //! .drv sendid  on|off      tracing simCOnnect sendId on/off
            //! </pre>
            //! @}
            virtual bool parseDetails(const BlackMisc::CSimpleCommandParser &parser) override;

            //! Trigger tracing ids for some while
            //! \sa CSimulatorFsxCommon::isTracingSendId
            bool triggerAutoTraceSendId(qint64 traceTimeMs = AutoTraceOffsetMs);

            //! Callsign for pending request
            BlackMisc::Aviation::CCallsign getCallsignForPendingProbeRequests(DWORD requestId, bool remove);

            //! Get new request id, overflow safe
            SIMCONNECT_DATA_REQUEST_ID obtainRequestIdForSimObjAircraft();

            //! Get new request id, overflow safe
            SIMCONNECT_DATA_REQUEST_ID obtainRequestIdForSimObjTerrainProbe();

            //! Release AI control
            HRESULT releaseAIControl(SIMCONNECT_OBJECT_ID objectId, SIMCONNECT_DATA_REQUEST_ID requestId);

            //! Valid CSimConnectObject which is NOT pendig removed
            bool isValidSimObjectNotPendingRemoved(const CSimConnectObject &simObject) const;

            //! CSimConnectObject for trace
            CSimConnectObject getSimObjectForTrace(const TraceFsxSendId &trace) const;

            //! Remove the CSimConnectObject linked in the trace
            bool removeSimObjectForTrace(const TraceFsxSendId &trace);

            //! Register help
            static void registerHelp();

            static constexpr qint64 AutoTraceOffsetMs = 10 * 1000;              //!< how long do we trace?
            HANDLE m_hSimConnect = nullptr;                                     //!< handle to SimConnect object
            DispatchProc m_dispatchProc = &CSimulatorFsxCommon::SimConnectProc; //!< called function for dispatch, can be overriden by specialized P3D function
            CSimConnectObjects m_simConnectObjects;                             //!< AI objects and their object and request ids

            // probes
            bool m_useFsxTerrainProbe = true;   //!< Use FSX Terrain probe?
            bool m_initFsxTerrainProbes = false; //!< initialized terrain probes
            int  m_addedProbes = 0; //!< added probes
            QMap<DWORD, BlackMisc::Aviation::CCallsign> m_pendingProbeRequests; //!< pending elevation requests: requestId/aircraft callsign

        private:
            //! Reason for adding an aircraft
            enum AircraftAddMode
            {
                ExternalCall,     //!< normal external request to add aircraft
                AddByTimer,       //!< add pending aircraft by timer
                AddAfterAdded,    //!< add pending because object successfully added
                AddedAfterRemoved //!< added again after removed
            };

            //! Mode as string
            static const QString &modeToString(AircraftAddMode mode);

            //! Dispatch SimConnect messages
            //! \remark very frequently called
            void dispatch();

            //! Implementation of add remote aircraft, which also handles FSX specific adding one by one
            //! \remark main purpose of this function is to only add one aircraft at a time, and only if simulator is not paused/stopped
            bool physicallyAddRemoteAircraftImpl(const BlackMisc::Simulation::CSimulatedAircraft &newRemoteAircraft, AircraftAddMode addMode);

            //! Add AI object for terrain probe
            //! \remark experimental
            bool physicallyAddAITerrainProbe(const BlackMisc::Geo::ICoordinateGeodetic &coordinate, int number);

            //! Add number probes (inits the probe objects)
            //! \remark experimental
            int physicallyInitAITerrainProbes(const BlackMisc::Geo::ICoordinateGeodetic &coordinate, int number);

            //! Remove aircraft no longer in provider
            //! \remark kind of cleanup function, in an ideal this should never need to cleanup something
            BlackMisc::Aviation::CCallsignSet physicallyRemoveAircraftNotInProvider();

            //! Verify that an object has been added in simulator
            //! \remark checks if the object was really added after an "add request" and not directly removed again
            //! \remark requests further data on remote aircraft (lights, ..) when correctly added
            void verifyAddedRemoteAircraft(const BlackMisc::Simulation::CSimulatedAircraft &remoteAircraftIn);

            //! Adding an aircraft failed
            void addingAircraftFailed(const CSimConnectObject &simObject);

            //! Create a detailed info about the failed aircraft
            bool verifyFailedAircraftInfo(const CSimConnectObject &simObject, BlackMisc::CStatusMessage &details) const;

            //! Logging version of verifyFailedAircraftInfo
            bool logVerifyFailedAircraftInfo(const CSimConnectObject &simObject) const;

            //! Verify the probe
            void verifyAddedTerrainProbe(const BlackMisc::Simulation::CSimulatedAircraft &remoteAircraftIn);

            //! Add next aircraft based on timer
            void addPendingAircraftByTimer();

            //! Add next aircraft after another has been confirmed
            void addPendingAircraftAfterAdded();

            //! Try to add the next aircraft (one by one)
            void addPendingAircraft(AircraftAddMode mode);

            //! Remove as m_addPendingAircraft and m_aircraftToAddAgainWhenRemoved
            CSimConnectObject removeFromAddPendingAndAddAgainAircraft(const BlackMisc::Aviation::CCallsign &callsign);

            //! Call this method to declare the simulator connected
            void setSimConnected();

            //! Called when simulator has started
            void onSimRunning();

            //! Deferred version of onSimRunning to avoid jitter
            void onSimRunningDeferred(qint64 referenceTs);

            //! Called every visual frame
            void onSimFrame();

            //! Called when simulator has stopped, e.g. by selecting the "select aircraft screen"
            void onSimStopped();

            //! Simulator is going down
            void onSimExit();

            //! Init when connected
            HRESULT initWhenConnected();

            //! Initialize SimConnect system events
            HRESULT initEvents();

            //! Initialize SimConnect data definitions
            HRESULT initDataDefinitionsWhenConnected();

            //! Update remote aircraft
            //! \remark this is where the interpolated data are sent
            void updateRemoteAircraft();

            //! Update remote aircraft parts (send to FSX)
            bool updateRemoteAircraftParts(const CSimConnectObject &simObject, const BlackMisc::Simulation::CInterpolationResult &result);

            //! Calling CSimulatorFsxCommon::updateAirports
            void triggerUpdateAirports(const BlackMisc::Aviation::CAirportList &airports);

            //! Update airports from simulator
            void updateAirports(const BlackMisc::Aviation::CAirportList &airports);

            //! Send parts to simulator
            //! \remark does not send if there is no change
            bool sendRemoteAircraftPartsToSimulator(const CSimConnectObject &simObject, DataDefinitionRemoteAircraftPartsWithoutLights &ddRemoteAircraftParts, const BlackMisc::Aviation::CAircraftLights &lights);

            //! Send lights to simulator (those which have to be toggled)
            //! \remark challenge here is that I can only sent those value if I have already obtained the current light state from simulator
            //! \param force send lights even if they appear to be the same
            void sendToggledLightsToSimulator(const CSimConnectObject &simObject, const BlackMisc::Aviation::CAircraftLights &lightsWanted, bool force = false);

            //! Called when data about our own aircraft are received
            void updateOwnAircraftFromSimulator(const DataDefinitionOwnAircraft &simulatorOwnAircraft);

            //! Call CSimulatorFsxCommon::updateRemoteAircraftFromSimulator asynchronously
            //! \remark do not to send SimConnect data in event loop
            void triggerUpdateRemoteAircraftFromSimulator(const CSimConnectObject &simObject, const DataDefinitionPosData &remoteAircraftData);

            //! Call CSimulatorFsxCommon::updateRemoteAircraftFromSimulator asynchronously
            //! \remark do not to send SimConnect data in event loop
            void triggerUpdateRemoteAircraftFromSimulator(const CSimConnectObject &simObject, const DataDefinitionRemoteAircraftModel &remoteAircraftModel);

            //! Remote aircraft data sent from simulator
            void updateRemoteAircraftFromSimulator(const CSimConnectObject &simObject, const DataDefinitionPosData &remoteAircraftData);

            //! Remote aircraft data sent from simulator
            void updateRemoteAircraftFromSimulator(const CSimConnectObject &simObject, const DataDefinitionRemoteAircraftModel &remoteAircraftModel);

            //! Probe data sent from simulator
            void updateProbeFromSimulator(const BlackMisc::Aviation::CCallsign &callsign, const DataDefinitionPosData &remoteAircraftData);

            //! Update from SB client area
            //! \threadsafe
            void updateOwnAircraftFromSimulator(const DataDefinitionClientAreaSb &sbDataArea);

            //! An AI aircraft was added in the simulator
            bool simulatorReportedObjectAdded(DWORD objectId);

            //! Simulator reported that AI aircraft was removed
            bool simulatorReportedObjectRemoved(DWORD objectID);

            //! Set ID of a SimConnect object, so far we only have an request id in the object
            bool setSimConnectObjectId(DWORD requestId, DWORD objectId);

            //! Remember current lights
            bool setCurrentLights(const BlackMisc::Aviation::CCallsign &callsign, const BlackMisc::Aviation::CAircraftLights &lights);

            //! Remember lights sent
            bool setLightsAsSent(const BlackMisc::Aviation::CCallsign &callsign, const BlackMisc::Aviation::CAircraftLights &lights);

            //! Display receive exceptions?
            bool stillDisplayReceiveExceptions();

            //! The SimConnect related objects
            const CSimConnectObjects &getSimConnectObjects() const { return m_simConnectObjects; }

            //! The SimConnect object for idxs
            CSimConnectObject getSimObjectForObjectId(DWORD objectId) const;

            //! Format conversion
            //! \note must be valid situation
            SIMCONNECT_DATA_INITPOSITION aircraftSituationToFsxPosition(const BlackMisc::Aviation::CAircraftSituation &situation, bool sendGnd = true);

            //! Format conversion
            SIMCONNECT_DATA_INITPOSITION coordinateToFsxPosition(const BlackMisc::Geo::ICoordinateGeodetic &coordinate);

            //! Sync time with user's computer
            void synchronizeTime(const BlackMisc::PhysicalQuantities::CTime &zuluTimeSim, const BlackMisc::PhysicalQuantities::CTime &localTimeSim);

            //! Request data for a CSimConnectObject (aka remote aircraft)
            bool requestPositionDataForSimObject(const CSimConnectObject &simObject, SIMCONNECT_PERIOD period = SIMCONNECT_PERIOD_SECOND);

            //! Request data for the terrain probe
            bool requestTerrainProbeData(const CSimConnectObject &simObject, const BlackMisc::Aviation::CCallsign &aircraftCallsign);

            //! Request lights for a CSimConnectObject
            bool requestLightsForSimObject(const CSimConnectObject &simObject);

            //! Model info for a CSimConnectObject
            bool requestModelInfoForSimObject(const CSimConnectObject &simObject);

            //! Stop requesting data for CSimConnectObject
            bool stopRequestingDataForSimObject(const CSimConnectObject &simObject);

            //! FSX position as string
            static QString fsxPositionToString(const SIMCONNECT_DATA_INITPOSITION &position);

            //! Get the callsigns which are no longer in the provider, but still in m_simConnectObjects
            BlackMisc::Aviation::CCallsignSet getCallsignsMissingInProvider() const;

            //! Set tracing on/off
            void setTraceSendId(bool traceSendId) { m_traceSendId = traceSendId; }

            //! Trace the send id
            void traceSendId(const CSimConnectObject &simObject, const QString &functionName, const QString &details = {}, bool forceTrace = false);

            //! Trace if required, log errors
            HRESULT logAndTraceSendId(HRESULT hr, const QString &warningMsg, const QString &functionName, const QString &functionDetails = {});

            //! Trace if required, log errors
            HRESULT logAndTraceSendId(HRESULT hr, const CSimConnectObject &simObject, const QString &warningMsg, const QString &functionName, const QString &functionDetails = {});

            //! Trace if required, log errors
            HRESULT logAndTraceSendId(HRESULT hr, bool traceSendId, const CSimConnectObject &simObject, const QString &warningMsg, const QString &functionName, const QString &functionDetails = {});

            //! Send id trace or given send id
            TraceFsxSendId getSendIdTrace(DWORD sendId) const;

            //! Get the trace details, otherwise empty string
            QString getSendIdTraceDetails(const TraceFsxSendId &trace) const;

            //! Get the trace details, otherwise empty string
            QString getSendIdTraceDetails(DWORD sendId) const;

            //! Remove all probes
            int removeAllProbes();

            //! Insert a new SimConnect object
            CSimConnectObject insertNewSimConnectObject(const BlackMisc::Simulation::CSimulatedAircraft &aircraft, DWORD requestId, const CSimConnectObject &removedPendingObject = {});

            //! Used for terrain probes
            static const BlackMisc::Aviation::CAltitude &terrainProbeAltitude();

            static constexpr int GuessRemoteAircraftPartsCycle = 20; //!< guess every n-th cycle
            static constexpr int SkipUpdateCyclesForCockpit    = 10; //!< skip x cycles before updating cockpit again
            static constexpr int IgnoreReceiveExceptions       = 10; //!< skip exceptions when displayed more than x times
            static constexpr int MaxSendIdTraces     = 10000;        //!< max.traces of send id
            static constexpr DWORD MaxSimObjAircraft = 10000;        //!< max.number of SimObjects at the same time
            static constexpr DWORD MaxSimObjProbes   = 100;          //!< max. probes

            // -- second chance tresholds --
            static constexpr int ThresholdAddException = 1; //!< one failure allowed
            static constexpr int ThresholdAddedAndDirectlyRemoved = 2; //!< two failures allowed

            // -- range for sim data, each sim object will get its own request id and use the offset ranges
            static constexpr int RequestSimObjAircraftStart    = static_cast<int>(CSimConnectDefinitions::RequestEndMarker);
            static constexpr int RequestSimObjAircraftEnd      = RequestSimObjAircraftStart - 1 + MaxSimObjAircraft;
            static constexpr int RequestSimObjAircraftRangeEnd = RequestSimObjAircraftStart - 1 + static_cast<int>(CSimConnectDefinitions::SimObjectEndMarker) * MaxSimObjAircraft;

            // -- range for probe data, each probe object will get its own request id and use the offset ranges
            static constexpr int RequestSimObjTerrainProbeStart    = RequestSimObjAircraftRangeEnd + 1 ;
            static constexpr int RequestSimObjTerrainProbeEnd      = RequestSimObjTerrainProbeStart - 1 + MaxSimObjProbes;
            static constexpr int RequestSimObjTerrainProbeRangeEnd = RequestSimObjTerrainProbeStart - 1 + static_cast<int>(CSimConnectDefinitions::SimObjectEndMarker) * MaxSimObjProbes;

            // times
            static constexpr int AddPendingAircraftIntervalMs = 20 * 1000;
            static constexpr int DispatchIntervalMs    = 10;   //!< how often with run the FSX event queue
            static constexpr int DeferSimulatingFlagMs = 1500; //!< simulating can jitter at startup (simulating->stopped->simulating, multiple start events), so we defer detection
            static constexpr int DeferResendingLights  = 2500; //!< Resend light state when aircraft light state was not yet available

            QString m_simConnectVersion;         //!< SimConnect version
            bool m_simConnected  = false;        //!< Is simulator connected?
            bool m_simSimulating = false;        //!< Simulator running?
            bool m_useSbOffsets  = true;         //!< with SB offsets
            bool m_traceSendId   = false;        //!< trace the send ids, meant for debugging
            qint64 m_traceAutoUntilTs = -1;      //!< allows to automatically trace for some time
            qint64 m_simulatingChangedTs = -1;   //!< timestamp, when simulating changed (used to avoid jitter)
            int m_syncDeferredCounter =  0;      //!< Set when synchronized, used to wait some time
            int m_skipCockpitUpdateCycles = 0;   //!< skip some update cycles to allow changes in simulator cockpit to be set
            int m_ownAircraftUpdate = 0;         //!< own aircraft update

            // tracing dispatch performance
            int m_dispatchErrors         = 0;    //!< number of dispatched failed, \sa dispatch
            int m_dispatchProcCount      = 0;    //!< number of dispatchProc counts
            int m_dispatchProcEmptyCount = 0;    //!< number dispatchProc doing nothing
            qint64 m_dispatchTimeMs        = -1;
            qint64 m_dispatchMaxTimeMs     = -1;
            qint64 m_dispatchProcTimeMs    = -1;
            qint64 m_dispatchProcMaxTimeMs = -1;

            SIMCONNECT_RECV_ID m_dispatchReceiveIdLast    = SIMCONNECT_RECV_ID_NULL;     //!< last receive id from dispatching
            SIMCONNECT_RECV_ID m_dispatchReceiveIdMaxTime = SIMCONNECT_RECV_ID_NULL;     //!< receive id corresponding to max.time
            DWORD m_dispatchRequestIdLast    = CSimConnectDefinitions::RequestEndMarker; //!< request id if any for last request
            DWORD m_dispatchRequestIdMaxTime = CSimConnectDefinitions::RequestEndMarker; //!< request id corresponding to max.time

            // sending via SimConnect
            QList<TraceFsxSendId> m_sendIdTraces; //!< Send id traces for debugging, latest first
            int m_receiveExceptionCount = 0;      //!< exceptions
            int m_requestSimObjectDataCount  = 0; //!< requested SimObjects

            // objects
            CSimConnectObjects m_simConnectObjectsPositionAndPartsTraces; //!< position/parts received, but object not yet added, excluded, disabled etc.
            CSimConnectObjects m_addPendingAircraft; //!< aircraft/probes awaiting to be added;
            SIMCONNECT_DATA_REQUEST_ID m_requestIdSimObjAircraft     = static_cast<SIMCONNECT_DATA_REQUEST_ID>(RequestSimObjAircraftStart);     //!< request id, use obtainRequestIdForSimObjAircraft to get id
            SIMCONNECT_DATA_REQUEST_ID m_requestIdSimObjTerrainProbe = static_cast<SIMCONNECT_DATA_REQUEST_ID>(RequestSimObjTerrainProbeStart); //!< request id, use obtainRequestIdForSimObjTerrainProbe to get id
            QTimer m_addPendingSimObjTimer; //!< updating of SimObjects awaiting to be added

            //! Request id to string
            static QString requestIdToString(DWORD requestId);

        public:
            //! Offsets @{
            static DWORD offsetSimObjAircraft(CSimConnectDefinitions::SimObjectRequest req) { return MaxSimObjAircraft * static_cast<DWORD>(req); }
            static DWORD offsetSimObjTerrainProbe(CSimConnectDefinitions::SimObjectRequest req) { return MaxSimObjProbes * static_cast<DWORD>(req); }
            //! @}
        };

        //! Listener for FSX
        class CSimulatorFsxCommonListener : public BlackCore::ISimulatorListener
        {
            Q_OBJECT

        public:
            //! Constructor
            CSimulatorFsxCommonListener(const BlackMisc::Simulation::CSimulatorPluginInfo &info);

            //! \copydoc BlackCore::ISimulatorListener::backendInfo
            virtual QString backendInfo() const override;

        protected:
            //! \copydoc BlackCore::ISimulatorListener::startImpl
            virtual void startImpl() override;

            //! \copydoc BlackCore::ISimulatorListener::stopImpl
            virtual void stopImpl() override;

            //! \copydoc BlackCore::ISimulatorListener::checkImpl
            virtual void checkImpl() override;

            //! Test if connection can be established
            void checkConnection();

            //! Check simulator version and type
            bool checkVersionAndSimulator() const;

            //! Check the simconnect.dll
            bool checkSimConnectDll() const;

        private:
            QTimer  m_timer { this }; //!< timer, "this" is needed otherwise I get warnings when move to new thread
            QString m_simulatorVersion;
            QString m_simConnectVersion;
            QString m_simulatorName;
            QString m_simulatorDetails;
            BlackMisc::CStatusMessage m_lastMessage; //!< last listener message

            //! SimConnect Callback (simplified version for listener)
            //! \sa CSimConnectObjects::SimConnectProc
            static void CALLBACK SimConnectProc(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext);
        };
    } // namespace
} // namespace

#endif // guard
