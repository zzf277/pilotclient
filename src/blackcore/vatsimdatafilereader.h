/* Copyright (C) 2013 VATSIM Community / authors
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BLACKCORE_VATSIMDATAFILEREADER_H
#define BLACKCORE_VATSIMDATAFILEREADER_H

#include "blackmisc/avatcstationlist.h"
#include "blackmisc/avaircraftlist.h"
#include "blackmisc/nwserverlist.h"
#include <QObject>
#include <QTimer>
#include <QNetworkReply>

namespace BlackCore
{
    /*!
     * \brief Read bookings from VATSIM
     */
    class CVatsimDataFileReader : public QObject
    {
        Q_OBJECT

    public:
        //! \brief Constructor
        explicit CVatsimDataFileReader(const QStringList &urls, QObject *parent = nullptr);

        //! \brief Update timestamp
        QDateTime getUpdateTimestamp() const { return this->m_updateTimestamp; }

        //! \brief Read / re-read bookings
        void read();

        /*!
         * \brief Set the update time
         * \param updatePeriodMs 0 stops the timer
         */
        void setInterval(int updatePeriodMs);

        //! \brief Get the timer interval (ms)
        int interval() const { return this->m_updateTimer->interval();}

        //! \brief Get aircrafts
        const BlackMisc::Aviation::CAircraftList &getAircrafts() { return this->m_aircrafts; }

        //! \brief Get aircrafts
        const BlackMisc::Aviation::CAtcStationList &getAtcStations() { return this->m_atcStations; }

        //! \brief Get all voice servers
        const BlackMisc::Network::CServerList &getVoiceServers() { return this->m_voiceServers; }


    private slots:
        //! \brief Data have been read
        void loadFinished(QNetworkReply *nwReply);

    private:
        QStringList m_serviceUrls; /*!< URL of the service */
        int m_currentUrlIndex;
        QNetworkAccessManager *m_networkManager;
        QDateTime m_updateTimestamp;
        QTimer *m_updateTimer;
        BlackMisc::Network::CServerList m_voiceServers;
        BlackMisc::Aviation::CAtcStationList m_atcStations;
        BlackMisc::Aviation::CAircraftList m_aircrafts;
        static const QMap<QString, QString> clientPartsToMap(const QString &currentLine, const QStringList &clientSectionAttributes);

        //! \brief Section in file
        enum Section
        {
            SectionNone,
            SectionVoiceServer,
            SectionClients,
            SectionGeneral
        };

    signals:
        //! \brief Data have been read
        void dataRead();
    };
}

#endif // guard
