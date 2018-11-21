/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "xswiftbustrafficproxy.h"
#include <QLatin1String>
#include <QDBusConnection>

#define XSWIFTBUS_SERVICENAME "org.swift-project.xswiftbus"

using namespace BlackMisc::Aviation;
using namespace BlackMisc::Geo;
using namespace BlackMisc::PhysicalQuantities;

namespace BlackSimPlugin
{
    namespace XPlane
    {
        CXSwiftBusTrafficProxy::CXSwiftBusTrafficProxy(QDBusConnection &connection, QObject *parent, bool dummy) : QObject(parent)
        {
            m_dbusInterface = new BlackMisc::CGenericDBusInterface(XSWIFTBUS_SERVICENAME, ObjectPath(), InterfaceName(), connection, this);
            if (!dummy)
            {
                bool s;
                s = connection.connect(QString(), "/xswiftbus/traffic", "org.swift_project.xswiftbus.traffic",
                                       "simFrame", this, SIGNAL(simFrame()));
                Q_ASSERT(s);

                s = connection.connect(QString(), "/xswiftbus/traffic", "org.swift_project.xswiftbus.traffic",
                                       "remoteAircraftAdded", this, SIGNAL(remoteAircraftAdded(QString)));
                Q_ASSERT(s);

                s = connection.connect(QString(), "/xswiftbus/traffic", "org.swift_project.xswiftbus.traffic",
                                       "remoteAircraftAddingFailed", this, SIGNAL(remoteAircraftAddingFailed(QString)));
                Q_ASSERT(s);
            }
        }

        bool CXSwiftBusTrafficProxy::initialize()
        {
            return m_dbusInterface->callDBusRet<bool>(QLatin1String("initialize"));
        }

        void CXSwiftBusTrafficProxy::cleanup()
        {
            m_dbusInterface->callDBus(QLatin1String("cleanup"));
        }

        bool CXSwiftBusTrafficProxy::loadPlanesPackage(const QString &path)
        {
            return m_dbusInterface->callDBusRet<bool>(QLatin1String("loadPlanesPackage"), path);
        }

        void CXSwiftBusTrafficProxy::setDefaultIcao(const QString &defaultIcao)
        {
            m_dbusInterface->callDBus(QLatin1String("setDefaultIcao"), defaultIcao);
        }

        void CXSwiftBusTrafficProxy::setDrawingLabels(bool drawing)
        {
            m_dbusInterface->callDBus(QLatin1String("setDrawingLabels"), drawing);
        }

        bool CXSwiftBusTrafficProxy::isDrawingLabels() const
        {
            return m_dbusInterface->callDBusRet<bool>(QLatin1String("isDrawingLabels"));
        }

        void CXSwiftBusTrafficProxy::setMaxPlanes(int planes)
        {
            m_dbusInterface->callDBus(QLatin1String("setMaxPlanes"), planes);
        }

        void CXSwiftBusTrafficProxy::setMaxDrawDistance(double nauticalMiles)
        {
            m_dbusInterface->callDBus(QLatin1String("setMaxDrawDistance"), nauticalMiles);
        }

        void CXSwiftBusTrafficProxy::addPlane(const QString &callsign, const QString &modelName, const QString &aircraftIcao, const QString &airlineIcao, const QString &livery)
        {
            m_dbusInterface->callDBus(QLatin1String("addPlane"), callsign, modelName, aircraftIcao, airlineIcao, livery);
        }

        void CXSwiftBusTrafficProxy::removePlane(const QString &callsign)
        {
            m_dbusInterface->callDBus(QLatin1String("removePlane"), callsign);
        }

        void CXSwiftBusTrafficProxy::removeAllPlanes()
        {
            m_dbusInterface->callDBus(QLatin1String("removeAllPlanes"));
        }

        void CXSwiftBusTrafficProxy::setPlanesPositions(const PlanesPositions &planesPositions)
        {
            m_dbusInterface->callDBus(QLatin1String("setPlanesPositions"),
                                      planesPositions.callsigns, planesPositions.latitudesDeg, planesPositions.longitudesDeg,
                                      planesPositions.altitudesFt, planesPositions.pitchesDeg, planesPositions.rollsDeg,
                                      planesPositions.headingsDeg, planesPositions.onGrounds);
        }

        void CXSwiftBusTrafficProxy::setPlanesSurfaces(const PlanesSurfaces &planesSurfaces)
        {
            m_dbusInterface->callDBus(QLatin1String("setPlanesSurfaces"),
                                      planesSurfaces.callsigns, planesSurfaces.gears, planesSurfaces.flaps,
                                      planesSurfaces.spoilers, planesSurfaces.speedBrakes, planesSurfaces.slats,
                                      planesSurfaces.wingSweeps, planesSurfaces.thrusts, planesSurfaces.elevators,
                                      planesSurfaces.rudders, planesSurfaces.ailerons,
                                      planesSurfaces.landLights, planesSurfaces.beaconLights, planesSurfaces.strobeLights,
                                      planesSurfaces.navLights, planesSurfaces.lightPatterns);
        }

        void CXSwiftBusTrafficProxy::setPlanesTransponders(const PlanesTransponders &planesTransponders)
        {
            m_dbusInterface->callDBus(QLatin1String("setPlanesTransponders"),
                                      planesTransponders.callsigns, planesTransponders.codes,
                                      planesTransponders.modeCs, planesTransponders.idents);
        }

        void CXSwiftBusTrafficProxy::setInterpolatorMode(const QString &callsign, bool spline)
        {
            m_dbusInterface->callDBus(QLatin1String("setInterpolatorMode"), callsign, spline);
        }

        void CXSwiftBusTrafficProxy::getRemoteAircraftData(const QStringList &callsigns, const RemoteAircraftDataCallback &setter) const
        {
            std::function<void(QDBusPendingCallWatcher *)> callback = [ = ](QDBusPendingCallWatcher * watcher)
            {
                QDBusPendingReply<QStringList, QList<double>, QList<double>, QList<double>, QList<double>> reply = *watcher;
                if (!reply.isError())
                {
                    const QStringList callsigns = reply.argumentAt<0>();
                    const QList<double> latitudesDeg = reply.argumentAt<1>();
                    const QList<double> longitudesDeg = reply.argumentAt<2>();
                    const QList<double> elevationsM = reply.argumentAt<3>();
                    const QList<double> verticalOffsets = reply.argumentAt<4>();
                    setter(callsigns, latitudesDeg, longitudesDeg, elevationsM, verticalOffsets);
                }
                watcher->deleteLater();
            };
            m_dbusInterface->callDBusAsync(QLatin1String("getRemoteAircraftData"), callback, callsigns);
        }

        void CXSwiftBusTrafficProxy::getElevationAtPosition(const CCallsign &callsign, double latitudeDeg, double longitudeDeg, double altitudeMeters,
                const ElevationCallback &setter) const
        {
            std::function<void(QDBusPendingCallWatcher *)> callback = [ = ](QDBusPendingCallWatcher * watcher)
            {
                QDBusPendingReply<QString, double> reply = *watcher;
                if (!reply.isError())
                {
                    const CCallsign cs(reply.argumentAt<0>());
                    const double elevationMeters = reply.argumentAt<1>();
                    const CAltitude elevationAlt(elevationMeters, CLengthUnit::m(), CLengthUnit::ft());
                    const CElevationPlane elevation(CLatitude(latitudeDeg, CAngleUnit::deg()),
                                                    CLongitude(longitudeDeg, CAngleUnit::deg()),
                                                    elevationAlt, CElevationPlane::singlePointRadius());
                    setter(elevation, cs);
                }
                watcher->deleteLater();
            };
            m_dbusInterface->callDBusAsync(QLatin1String("getElevationAtPosition"), callback, callsign.asString(), latitudeDeg, longitudeDeg, altitudeMeters);
        }

        void CXSwiftBusTrafficProxy::setFollowedAircraft(const QString &callsign)
        {
            m_dbusInterface->callDBus(QLatin1String("setFollowedAircraft"), callsign);
        }
    }
}
