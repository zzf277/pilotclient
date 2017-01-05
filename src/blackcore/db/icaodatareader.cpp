/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackcore/application.h"
#include "blackcore/data/globalsetup.h"
#include "blackcore/db/icaodatareader.h"
#include "blackmisc/fileutils.h"
#include "blackmisc/json.h"
#include "blackmisc/logmessage.h"
#include "blackmisc/statusmessage.h"

#include <QDateTime>
#include <QDir>
#include <QFlags>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QReadLocker>
#include <QScopedPointer>
#include <QScopedPointerDeleteLater>
#include <QTimer>
#include <QUrl>
#include <QWriteLocker>
#include <Qt>
#include <QtGlobal>

using namespace BlackMisc;
using namespace BlackMisc::Db;
using namespace BlackMisc::Aviation;
using namespace BlackMisc::Network;
using namespace BlackCore::Data;

namespace BlackCore
{
    namespace Db
    {
        CIcaoDataReader::CIcaoDataReader(QObject *owner, const CDatabaseReaderConfigList &confg) :
            CDatabaseReader(owner, confg, "CIcaoDataReader")
        {
            // init to avoid threading issues
            getBaseUrl(CDbFlags::DbReading);
        }

        CAircraftIcaoCodeList CIcaoDataReader::getAircraftIcaoCodes() const
        {
            return m_aircraftIcaoCache.get();
        }

        CAircraftIcaoCode CIcaoDataReader::getAircraftIcaoCodeForDesignator(const QString &designator) const
        {
            return getAircraftIcaoCodes().findFirstByDesignatorAndRank(designator);
        }

        CAircraftIcaoCodeList CIcaoDataReader::getAircraftIcaoCodesForDesignator(const QString &designator) const
        {
            return getAircraftIcaoCodes().findByDesignator(designator);
        }

        CAircraftIcaoCode CIcaoDataReader::getAircraftIcaoCodeForDbKey(int key) const
        {
            return getAircraftIcaoCodes().findByKey(key);
        }

        CAirlineIcaoCodeList CIcaoDataReader::getAirlineIcaoCodes() const
        {
            return m_airlineIcaoCache.get();
        }

        CAircraftIcaoCode CIcaoDataReader::smartAircraftIcaoSelector(const CAircraftIcaoCode &icaoPattern) const
        {
            CAircraftIcaoCodeList codes(getAircraftIcaoCodes()); // thread safe copy
            return codes.smartAircraftIcaoSelector(icaoPattern); // sorted by rank
        }

        CCountryList CIcaoDataReader::getCountries() const
        {
            return m_countryCache.get();
        }

        CCountry CIcaoDataReader::getCountryForIsoCode(const QString &isoCode) const
        {
            return this->getCountries().findByIsoCode(isoCode);
        }

        CCountry CIcaoDataReader::getCountryForName(const QString &name) const
        {
            return this->getCountries().findBestMatchByCountryName(name);
        }

        CAirlineIcaoCodeList CIcaoDataReader::getAirlineIcaoCodesForDesignator(const QString &designator) const
        {
            return this->getAirlineIcaoCodes().findByVDesignator(designator);
        }

        CAirlineIcaoCode CIcaoDataReader::getAirlineIcaoCodeForDbKey(int key) const
        {
            return this->getAirlineIcaoCodes().findByKey(key);
        }

        CAirlineIcaoCode CIcaoDataReader::smartAirlineIcaoSelector(const CAirlineIcaoCode &icaoPattern, const CCallsign &callsign) const
        {
            CAirlineIcaoCodeList codes(this->getAirlineIcaoCodes()); // thread safe copy
            return codes.smartAirlineIcaoSelector(icaoPattern, callsign);
        }

        int CIcaoDataReader::getAircraftIcaoCodesCount() const
        {
            return this->getAircraftIcaoCodes().size();
        }

        int CIcaoDataReader::getAirlineIcaoCodesCount() const
        {
            return this->getAirlineIcaoCodes().size();
        }

        bool CIcaoDataReader::areAllDataRead() const
        {
            return getCountriesCount() > 0 && getAirlineIcaoCodesCount() > 0 && getAircraftIcaoCodesCount() > 0;
        }

        int CIcaoDataReader::getCountriesCount() const
        {
            return this->getCountries().size();
        }

        void CIcaoDataReader::ps_read(BlackMisc::Network::CEntityFlags::Entity entities, BlackMisc::Db::CDbFlags::DataRetrievalModeFlag mode, const QDateTime &newerThan)
        {
            this->threadAssertCheck(); // runs in background thread
            if (this->isShuttingDown()) { return; }

            CEntityFlags::Entity entitiesTriggered = CEntityFlags::NoEntity;
            if (entities.testFlag(CEntityFlags::AircraftIcaoEntity))
            {
                CUrl url(getAircraftIcaoUrl(mode));
                if (!url.isEmpty())
                {
                    if (!newerThan.isNull()) { url.appendQuery("newer=" + newerThan.toString(Qt::ISODate)); }
                    sApp->getFromNetwork(url, { this, &CIcaoDataReader::ps_parseAircraftIcaoData });
                    entitiesTriggered |= CEntityFlags::AircraftIcaoEntity;
                }
                else
                {
                    CLogMessage(this).error("No URL for %1") << CEntityFlags::flagToString(CEntityFlags::AircraftIcaoEntity);
                }
            }

            if (entities.testFlag(CEntityFlags::AirlineIcaoEntity))
            {
                CUrl url(getAirlineIcaoUrl(mode));
                if (!url.isEmpty())
                {
                    if (!newerThan.isNull()) { url.appendQuery("newer=" + newerThan.toString(Qt::ISODate)); }
                    sApp->getFromNetwork(url, { this, &CIcaoDataReader::ps_parseAirlineIcaoData });
                    entitiesTriggered |= CEntityFlags::AirlineIcaoEntity;
                }
                else
                {
                    CLogMessage(this).error("No URL for %1") << CEntityFlags::flagToString(CEntityFlags::AirlineIcaoEntity);
                }
            }

            if (entities.testFlag(CEntityFlags::CountryEntity))
            {
                CUrl url(getCountryUrl(mode));
                if (!url.isEmpty())
                {
                    if (!newerThan.isNull()) { url.appendQuery("newer=" + newerThan.toString(Qt::ISODate)); }
                    sApp->getFromNetwork(url, { this, &CIcaoDataReader::ps_parseCountryData });
                    entitiesTriggered |= CEntityFlags::CountryEntity;
                }
                else
                {
                    CLogMessage(this).error("No URL for %1") << CEntityFlags::flagToString(CEntityFlags::CountryEntity);
                }
            }

            if (entitiesTriggered != CEntityFlags::NoEntity)
            {
                emit dataRead(entitiesTriggered, CEntityFlags::StartRead, 0);
            }
        }

        void CIcaoDataReader::ps_aircraftIcaoCacheChanged()
        {
            this->cacheHasChanged(CEntityFlags::AircraftIcaoEntity);
        }

        void CIcaoDataReader::ps_airlineIcaoCacheChanged()
        {
            this->cacheHasChanged(CEntityFlags::AirlineIcaoEntity);
        }

        void CIcaoDataReader::ps_countryCacheChanged()
        {
            this->cacheHasChanged(CEntityFlags::CountryEntity);
        }

        void CIcaoDataReader::ps_baseUrlCacheChanged()
        {
            // void
        }

        void CIcaoDataReader::updateReaderUrl(const CUrl &url)
        {
            const CUrl current = this->m_readerUrlCache.get();
            if (current == url) { return; }
            const CStatusMessage m = this->m_readerUrlCache.set(url);
            if (m.isFailure())
            {
                CLogMessage::preformatted(m);
            }
        }

        void CIcaoDataReader::ps_parseAircraftIcaoData(QNetworkReply *nwReplyPtr)
        {
            // wrap pointer, make sure any exit cleans up reply
            // required to use delete later as object is created in a different thread
            QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> nwReply(nwReplyPtr);
            if (this->isShuttingDown()) { return; }

            const QString urlString(nwReply->url().toString());
            CDatabaseReader::JsonDatastoreResponse res = this->setStatusAndTransformReplyIntoDatastoreResponse(nwReply.data());
            if (res.hasErrorMessage())
            {
                CLogMessage::preformatted(res.lastWarningOrAbove());
                emit dataRead(CEntityFlags::AircraftIcaoEntity, CEntityFlags::ReadFailed, 0);
                return;
            }

            // normally read from special view which already filter incomplete
            const CAircraftIcaoCodeList codes = CAircraftIcaoCodeList::fromDatabaseJson(res, false);
            const int n = codes.size();
            qint64 latestTimestamp = codes.latestTimestampMsecsSinceEpoch();
            if (n > 0 && latestTimestamp < 0)
            {
                CLogMessage(this).error("No timestamp in aircraft ICAO list, setting to last modified value");
                latestTimestamp = lastModifiedMsSinceEpoch(nwReply.data());
            }

            this->m_aircraftIcaoCache.set(codes, latestTimestamp);
            this->updateReaderUrl(getBaseUrl(CDbFlags::DbReading));
            emit dataRead(CEntityFlags::AircraftIcaoEntity, CEntityFlags::ReadFinished, n);
            CLogMessage(this).info("Read %1 %2 from %3") << n << CEntityFlags::flagToString(CEntityFlags::AircraftIcaoEntity) << urlString;
        }

        void CIcaoDataReader::ps_parseAirlineIcaoData(QNetworkReply *nwReplyPtr)
        {
            QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> nwReply(nwReplyPtr);
            if (this->isShuttingDown()) { return; }

            QString urlString(nwReply->url().toString());
            CDatabaseReader::JsonDatastoreResponse res = this->setStatusAndTransformReplyIntoDatastoreResponse(nwReply.data());
            if (res.hasErrorMessage())
            {
                CLogMessage::preformatted(res.lastWarningOrAbove());
                emit dataRead(CEntityFlags::AirlineIcaoEntity, CEntityFlags::ReadFailed, 0);
                return;
            }
            const CAirlineIcaoCodeList codes = CAirlineIcaoCodeList::fromDatabaseJson(res, false);
            const int n = codes.size();
            qint64 latestTimestamp = codes.latestTimestampMsecsSinceEpoch();
            if (n > 0 && latestTimestamp < 0)
            {
                CLogMessage(this).error("No timestamp in airline ICAO list, setting to last modified value");
                latestTimestamp = lastModifiedMsSinceEpoch(nwReply.data());
            }

            this->m_airlineIcaoCache.set(codes, latestTimestamp);
            this->updateReaderUrl(getBaseUrl(CDbFlags::DbReading));
            emit dataRead(CEntityFlags::AirlineIcaoEntity, CEntityFlags::ReadFinished, n);
            CLogMessage(this).info("Read %1 %2 from %3") << n << CEntityFlags::flagToString(CEntityFlags::AirlineIcaoEntity) << urlString;
        }

        void CIcaoDataReader::ps_parseCountryData(QNetworkReply *nwReplyPtr)
        {
            QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> nwReply(nwReplyPtr);
            QString urlString(nwReply->url().toString());
            CDatabaseReader::JsonDatastoreResponse res = this->setStatusAndTransformReplyIntoDatastoreResponse(nwReply.data());
            if (res.hasErrorMessage())
            {
                CLogMessage::preformatted(res.lastWarningOrAbove());
                emit dataRead(CEntityFlags::CountryEntity, CEntityFlags::ReadFailed, 0);
                return;
            }
            const CCountryList countries = CCountryList::fromDatabaseJson(res);
            const int n = countries.size();
            qint64 latestTimestamp = countries.latestTimestampMsecsSinceEpoch();
            if (n > 0 && latestTimestamp < 0)
            {
                CLogMessage(this).error("No timestamp in country list, setting to last modified value");
                latestTimestamp = lastModifiedMsSinceEpoch(nwReply.data());
            }

            this->m_countryCache.set(countries, latestTimestamp);
            this->updateReaderUrl(getBaseUrl(CDbFlags::DbReading));
            emit dataRead(CEntityFlags::CountryEntity, CEntityFlags::ReadFinished, n);
            CLogMessage(this).info("Read %1 %2 from %3") << n << CEntityFlags::flagToString(CEntityFlags::CountryEntity) << urlString;
        }

        CStatusMessageList CIcaoDataReader::readFromJsonFiles(const QString &dir, CEntityFlags::Entity whatToRead)
        {
            QDir directory(dir);
            if (!directory.exists())
            {
                return CStatusMessage(this).error("Missing directory '%1'") << dir;
            }

            // Hint: Do not emit while locked -> deadlock
            CStatusMessageList msgs;
            whatToRead &= CEntityFlags::AllIcaoAndCountries;
            CEntityFlags::Entity reallyRead = CEntityFlags::NoEntity;
            if (whatToRead.testFlag(CEntityFlags::CountryEntity))
            {
                const QString fileName = CFileUtils::appendFilePaths(directory.absolutePath(), "countries.json");
                const QString countriesJson(CFileUtils::readFileToString(fileName));
                if (countriesJson.isEmpty())
                {
                    msgs.push_back(CStatusMessage(this).error("Failed to read from file/empty file '%1'") << fileName);
                }
                else
                {
                    try
                    {
                        CCountryList countries;
                        countries.convertFromJson(Json::jsonObjectFromString(countriesJson));
                        const int c = countries.size();
                        this->m_countryCache.set(countries);
                        reallyRead |= CEntityFlags::CountryEntity;
                        emit dataRead(CEntityFlags::CountryEntity, CEntityFlags::ReadFinished, c);
                    }
                    catch (const CJsonException &ex)
                    {
                        emit dataRead(CEntityFlags::CountryEntity, CEntityFlags::ReadFailed, 0);
                        msgs.push_back(ex.toStatusMessage(this, QString("Reading countries from '%1'").arg(fileName)));
                    }
                }
            }

            if (whatToRead.testFlag(CEntityFlags::AircraftIcaoEntity))
            {
                const QString fileName = CFileUtils::appendFilePaths(directory.absolutePath(), "aircrafticao.json");
                const QString aircraftJson(fileName);
                if (aircraftJson.isEmpty())
                {
                    msgs.push_back(CStatusMessage(this).error("Failed to read from file/empty file '%1'") << fileName);
                }
                else
                {
                    try
                    {
                        CAircraftIcaoCodeList aircraftIcaos;
                        aircraftIcaos.convertFromJson(Json::jsonObjectFromString(aircraftJson));
                        const int c = aircraftIcaos.size();
                        this->m_aircraftIcaoCache.set(aircraftIcaos);
                        reallyRead |= CEntityFlags::AircraftIcaoEntity;
                        emit dataRead(CEntityFlags::AircraftIcaoEntity, CEntityFlags::ReadFinished, c);
                    }
                    catch (const CJsonException &ex)
                    {
                        emit dataRead(CEntityFlags::AircraftIcaoEntity, CEntityFlags::ReadFailed, 0);
                        msgs.push_back(ex.toStatusMessage(this, QString("Reading aircraft ICAOs from '%1'").arg(fileName)));
                    }
                }
            }

            if (whatToRead.testFlag(CEntityFlags::AirlineIcaoEntity))
            {
                const QString fileName = CFileUtils::appendFilePaths(directory.absolutePath(), "airlineicao.json");
                const QString airlineJson(fileName);
                if (airlineJson.isEmpty())
                {
                    msgs.push_back(CStatusMessage(this).error("Failed to read from file/empty file '%1'") << fileName);
                }
                else
                {
                    try
                    {
                        CAirlineIcaoCodeList airlineIcaos;
                        airlineIcaos.convertFromJson(Json::jsonObjectFromString(airlineJson));
                        const int c = airlineIcaos.size();
                        this->m_airlineIcaoCache.set(airlineIcaos);
                        reallyRead |= CEntityFlags::AirlineIcaoEntity;
                        emit dataRead(CEntityFlags::AirlineIcaoEntity, CEntityFlags::ReadFinished, c);
                    }
                    catch (const CJsonException &ex)
                    {
                        emit dataRead(CEntityFlags::AirlineIcaoEntity, CEntityFlags::ReadFailed, 0);
                        msgs.push_back(ex.toStatusMessage(this, QString("Reading airline ICAOs from '%1'").arg(fileName)));
                    }
                }
            }

            if (msgs.isSuccess() && (reallyRead & CEntityFlags::DistributorLiveryModel) == whatToRead)
            {
                return CStatusMessage(this).info("Updated caches for '%1' from '%2'") << CEntityFlags::flagToString(reallyRead) << dir;
            }
            else
            {
                return msgs;
            }
        }

        bool CIcaoDataReader::readFromJsonFilesInBackground(const QString &dir, CEntityFlags::Entity whatToRead)
        {
            if (dir.isEmpty() || whatToRead == CEntityFlags::NoEntity) { return false; }
            QTimer::singleShot(0, this, [this, dir, whatToRead]()
            {
                const CStatusMessageList msgs = this->readFromJsonFiles(dir, whatToRead);
                if (msgs.isFailure())
                {
                    CLogMessage::preformatted(msgs);
                }
            });
            return true;
        }

        bool CIcaoDataReader::writeToJsonFiles(const QString &dir) const
        {
            QDir directory(dir);
            if (!directory.exists()) { return false; }
            if (this->getCountriesCount() > 0)
            {
                QString json(QJsonDocument(this->getCountries().toJson()).toJson());
                bool s = CFileUtils::writeStringToFileInBackground(json, CFileUtils::appendFilePaths(directory.absolutePath(), "countries.json"));
                if (!s) { return false; }
            }

            if (this->getAircraftIcaoCodesCount() > 0)
            {
                QString json(QJsonDocument(this->getAircraftIcaoCodes().toJson()).toJson());
                bool s = CFileUtils::writeStringToFileInBackground(json, CFileUtils::appendFilePaths(directory.absolutePath(), "aircrafticao.json"));
                if (!s) { return false; }
            }

            if (this->getAirlineIcaoCodesCount() > 0)
            {
                QString json(QJsonDocument(this->getAirlineIcaoCodes().toJson()).toJson());
                bool s = CFileUtils::writeStringToFileInBackground(json, CFileUtils::appendFilePaths(directory.absolutePath(), "airlineicao.json"));
                if (!s) { return false; }
            }
            return true;
        }

        CEntityFlags::Entity CIcaoDataReader::getSupportedEntities() const
        {
            return CEntityFlags::AllIcaoAndCountries;
        }

        void CIcaoDataReader::synchronizeCaches(CEntityFlags::Entity entities)
        {
            if (entities.testFlag(CEntityFlags::AircraftIcaoEntity)) { this->m_aircraftIcaoCache.synchronize(); }
            if (entities.testFlag(CEntityFlags::AirlineIcaoEntity)) { this->m_airlineIcaoCache.synchronize(); }
            if (entities.testFlag(CEntityFlags::CountryEntity)) { this->m_countryCache.synchronize(); }
        }

        void CIcaoDataReader::admitCaches(CEntityFlags::Entity entities)
        {
            if (entities.testFlag(CEntityFlags::AircraftIcaoEntity)) { this->m_aircraftIcaoCache.admit(); }
            if (entities.testFlag(CEntityFlags::AirlineIcaoEntity)) { this->m_airlineIcaoCache.admit(); }
            if (entities.testFlag(CEntityFlags::CountryEntity)) { this->m_countryCache.admit(); }
        }

        void CIcaoDataReader::invalidateCaches(CEntityFlags::Entity entities)
        {
            if (entities.testFlag(CEntityFlags::AircraftIcaoEntity)) { CDataCache::instance()->clearAllValues(this->m_aircraftIcaoCache.getKey()); }
            if (entities.testFlag(CEntityFlags::AirlineIcaoEntity)) {CDataCache::instance()->clearAllValues(this->m_airlineIcaoCache.getKey()); }
            if (entities.testFlag(CEntityFlags::CountryEntity)) {CDataCache::instance()->clearAllValues(this->m_countryCache.getKey()); }
        }

        QDateTime CIcaoDataReader::getCacheTimestamp(CEntityFlags::Entity entity) const
        {
            switch (entity)
            {
            case CEntityFlags::AircraftIcaoEntity: return this->m_aircraftIcaoCache.getAvailableTimestamp();
            case CEntityFlags::AirlineIcaoEntity:  return this->m_airlineIcaoCache.getAvailableTimestamp();
            case CEntityFlags::CountryEntity:      return this->m_countryCache.getAvailableTimestamp();
            default: return QDateTime();
            }
        }

        int CIcaoDataReader::getCacheCount(CEntityFlags::Entity entity) const
        {
            switch (entity)
            {
            case CEntityFlags::AircraftIcaoEntity: return this->m_aircraftIcaoCache.get().size();
            case CEntityFlags::AirlineIcaoEntity:  return this->m_airlineIcaoCache.get().size();
            case CEntityFlags::CountryEntity:      return this->m_countryCache.get().size();
            default: return 0;
            }
        }

        bool CIcaoDataReader::hasChangedUrl(CEntityFlags::Entity entity) const
        {
            Q_UNUSED(entity);
            return CDatabaseReader::isChangedUrl(this->m_readerUrlCache.get(), getBaseUrl(CDbFlags::DbReading));
        }

        CUrl CIcaoDataReader::getDbServiceBaseUrl() const
        {
            return sApp->getGlobalSetup().getDbIcaoReaderUrl();
        }

        CUrl CIcaoDataReader::getAircraftIcaoUrl(CDbFlags::DataRetrievalModeFlag mode) const
        {
            return getBaseUrl(mode).withAppendedPath(fileNameForMode(CEntityFlags::AircraftIcaoEntity, mode));
        }

        CUrl CIcaoDataReader::getAirlineIcaoUrl(CDbFlags::DataRetrievalModeFlag mode) const
        {
            return getBaseUrl(mode).withAppendedPath(fileNameForMode(CEntityFlags::AirlineIcaoEntity, mode));
        }

        CUrl CIcaoDataReader::getCountryUrl(CDbFlags::DataRetrievalModeFlag mode) const
        {
            return getBaseUrl(mode).withAppendedPath(fileNameForMode(CEntityFlags::CountryEntity, mode));
        }
    } // ns
} // ns
