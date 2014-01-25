/* Copyright (C) 2013 VATSIM Community / authors
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*!
    \file
*/

#ifndef BLACKMISC_SETTINGS_SERVER_H
#define BLACKMISC_SETTINGS_SERVER_H

#include "nwserverlist.h"
#include "valueobject.h"
#include "statusmessagelist.h"
#include "settingutilities.h"


namespace BlackMisc
{
    namespace Settings
    {
        /*!
         * Value object encapsulating information of a server
         */
        class CSettingsNetwork : public BlackMisc::CValueObject
        {
        public:
            /*!
             * Default constructor.
             */
            CSettingsNetwork();

            /*!
             * Destructor.
             */
            virtual ~CSettingsNetwork() {}

            /*!
             * \brief Update
             * \return
             */
            static const QString &CmdSetCurrentServer()
            {
                static const QString cmd("currenttrafficserver");
                return cmd;
            }

            /*!
             * \brief Path
             * \return
             */
            static const QString &PathTrafficServer()
            {
                static const QString cmd("trafficserver");
                return cmd;
            }

            /*!
             * \copydoc CValueObject::toQVariant()
             */
            virtual QVariant toQVariant() const
            {
                return QVariant::fromValue(*this);
            }

            /*!
             * Traffic network server objects
             * \return
             */
            BlackMisc::Network::CServerList getTrafficNetworkServers() const { return m_trafficNetworkServers; }

            /*!
             * Selected traffic network server
             */
            BlackMisc::Network::CServer getCurrentNetworkServer() const { return m_trafficNetworkServerCurrent; }

            /*!
             * \brief URL of booking service
             * \return
             */
            QString getBookingServiceUrl() const { return "http://vatbook.euroutepro.com/xml2.php"; }

            /*!
             * Selected traffic network server
             * \param currentServer
             * \return
             */
            bool setCurrentNetworkServer(const BlackMisc::Network::CServer &currentServer);

            /*!
             * Traffic network server objects
             */
            void addTrafficNetworkServer(const BlackMisc::Network::CServer &server) { m_trafficNetworkServers.push_back(server); }

            /*!
             * \brief Equal operator ==
             */
            bool operator ==(const CSettingsNetwork &other) const;

            /*!
             * \brief Unequal operator !=
             */
            bool operator !=(const CSettingsNetwork &other) const;

            /*!
             * \copydoc BlackCore::IContextSettings
             */
            virtual BlackMisc::CStatusMessageList value(const QString &path, const QString &command, const QVariant &value, bool &changedFlag);

            /*!
             * \brief Value hash
             */
            virtual uint getValueHash() const;

            /*!
             * \brief Register metadata
             */
            static void registerMetadata();

        protected:
            /*!
             * \brief Rounded value as string
             * \param i18n
             * \return
             */
            virtual QString convertToQString(bool i18n = false) const;

            /*!
             * \copydoc CValueObject::getMetaTypeId
             */
            virtual int getMetaTypeId() const;

            /*!
             * \copydoc CValueObject::isA
             */
            virtual bool isA(int metaTypeId) const;

            /*!
             * \copydoc CValueObject::compareImpl
             */
            virtual int compareImpl(const CValueObject &other) const;

            /*!
             * \brief Stream to DBus <<
             * \param argument
             */
            virtual void marshallToDbus(QDBusArgument &argument) const;

            /*!
             * \brief Stream from DBus >>
             * \param argument
             */
            virtual void unmarshallFromDbus(const QDBusArgument &argument);

        private:
            BlackMisc::Network::CServerList m_trafficNetworkServers;
            BlackMisc::Network::CServer m_trafficNetworkServerCurrent;
        };

    } // namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::Settings::CSettingsNetwork)

#endif // guard
