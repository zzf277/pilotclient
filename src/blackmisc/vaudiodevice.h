/* Copyright (C) 2013 VATSIM Community / authors
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BLACKMISC_AUDIODEVICE_H
#define BLACKMISC_AUDIODEVICE_H

/*!
    \file
*/

#include "valueobject.h"
#include <QString>

#ifdef Q_OS_WIN
typedef short int16_t;
#endif

namespace BlackMisc
{
    namespace Voice
    {
        /*!
         * Value object encapsulating information of a audio device.
         * If you want to safe this object, use the name instead of the index, since the index can change after
         * a restart.
         */
        class CAudioDevice : public BlackMisc::CValueObject
        {
        public:

            /*!
             * \brief Type
             */
            enum DeviceType
            {
                InputDevice,
                OutputDevice,
                Unknown
            };

            /*!
             * Default constructor.
             * If m_deviceIndex is -1, default should be used. However on Windows this doesnt work. Needs
             * to be checked in Vatlib.
             */
            CAudioDevice();

            /*!
             * Constructor.
             */
            CAudioDevice(DeviceType type, const int16_t index, const QString &getName);

            /*!
             * \brief QVariant, required for DBus QVariant lists
             * \return
             */
            virtual QVariant toQVariant() const
            {
                return QVariant::fromValue(*this);
            }

            /*!
             * Get the device index
             * \return
             */
            int16_t getIndex() const { return m_deviceIndex; }

            /*!
             * Get the device name
             * \return
             */
            const QString &getName() const { return m_deviceName; }

            /*!
             * \brief Type
             * \return
             */
            DeviceType getType() const { return m_type; }

            /*!
             * \brief Valid audio device object?
             * \return
             */
            bool isValid() const { return m_deviceIndex >= -1 && !m_deviceName.isEmpty(); }

            /*!
             * \brief Equal operator ==
             * \param other
             * @return
             */
            bool operator ==(const CAudioDevice &other) const;

            /*!
             * \brief Unequal operator ==
             * \param other
             * @return
             */
            bool operator !=(const CAudioDevice &other) const;

            /*!
             * \brief Value hash
             */
            virtual uint getValueHash() const;

            /*!
             * \brief Register metadata
             */
            static void registerMetadata();

            /*!
             * \brief Device type
             * \return
             */
            static int16_t defaultDevice() {return -1;}

            /*!
             * \brief Device type
             * \return
             */
            static int16_t invalidDevice() {return -2;}

        protected:

            /*!
             * \brief Rounded value as string
             * \param i18n
             * \return
             */
            virtual QString convertToQString(bool i18n = false) const;

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

        protected:
            /*!
             * deviceIndex is the number is the reference for the VVL. The device is selected by this index.
             * The managing class needs to take care, that indexes are valid.
             */
            DeviceType m_type;
            int16_t m_deviceIndex;
            QString m_deviceName;
            QString m_hostName;

        private:
            /*!
             * \brief Own host name
             * \return
             */
            static QString hostName();
        };

    } // Voice
} // BlackMisc

Q_DECLARE_METATYPE(BlackMisc::Voice::CAudioDevice)

#endif // guard
