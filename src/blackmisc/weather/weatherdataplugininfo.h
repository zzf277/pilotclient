/* Copyright (C) 2016
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_WEATHER_WEATHERDATAPLUGININFO_H
#define BLACKMISC_WEATHER_WEATHERDATAPLUGININFO_H

#include "blackmisc/blackmiscexport.h"
#include "blackmisc/valueobject.h"

namespace BlackMisc
{
    namespace Weather
    {
        //! Describing a weather data plugin
        class BLACKMISC_EXPORT CWeatherDataPluginInfo : public BlackMisc::CValueObject<CWeatherDataPluginInfo>
        {
        public:
            //! Default constructor
            CWeatherDataPluginInfo() = default;

            //! Constructor (used with unit tests)
            CWeatherDataPluginInfo(const QString &identifier, const QString &name,
                                   const QString &description, bool valid);

            //! \copydoc BlackMisc::CValueObject::convertFromJson
            void convertFromJson(const QJsonObject &json);

            //! Check if the provided plugin metadata is valid.
            //! Weather data plugin has to meet the following requirements:
            //!  * implements org.swift-project.blackcore.weatherdata;
            //!  * provides plugin name;
            bool isValid() const { return m_valid; }

            //! Identifier
            const QString &getIdentifier() const { return m_identifier; }

            //! Name
            const QString &getName() const { return m_name; }

            //! Description
            const QString &getDescription() const { return m_description; }

            //! \copydoc CValueObject::convertToQString
            QString convertToQString(bool i18n = false) const;

        private:
            BLACK_ENABLE_TUPLE_CONVERSION(CWeatherDataPluginInfo)
            QString m_identifier;
            QString m_name;
            QString m_description;
            bool m_valid { false };
        };
    } // ns
} // ns

BLACK_DECLARE_TUPLE_CONVERSION(BlackMisc::Weather::CWeatherDataPluginInfo, (
                                   attr(o.m_identifier, flags <CaseInsensitiveComparison> ()),
                                   attr(o.m_name, flags < DisabledForComparison | DisabledForHashing > ()),
                                   attr(o.m_description, flags < DisabledForComparison | DisabledForHashing > ()),
                                   attr(o.m_valid, flags < DisabledForComparison | DisabledForHashing > ())
                               ))
Q_DECLARE_METATYPE(BlackMisc::Weather::CWeatherDataPluginInfo)

#endif // guard
