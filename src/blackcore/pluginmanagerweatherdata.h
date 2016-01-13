/* Copyright (C) 2016
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKCORE_PLUGINMANAGERWEATHERDATA_H
#define BLACKCORE_PLUGINMANAGERWEATHERDATA_H

#include "blackcoreexport.h"
#include "pluginmanager.h"
#include "blackmisc/weather/weatherdataplugininfolist.h"
#include <QObject>

namespace BlackCore
{
    class IWeatherDataFactory;
    class IWeatherData;

    /*!
     * Manages plugins of type WeatherData.
     */
    class BLACKCORE_EXPORT CPluginManagerWeatherData :
            public BlackCore::IPluginManager
    {
        Q_OBJECT

    public:
        //! Ctor
        CPluginManagerWeatherData(QObject *parent = nullptr);

        //! Get weatherdata factory from the plugin
        IWeatherDataFactory *getFactory(const QString &pluginId);

        //! Get all weather data plugins
        BlackMisc::Weather::CWeatherDataPluginInfoList getAvailableWeatherDataPlugins() const;

        //! \copydoc BlackCore::IPluginManager::collectPlugins()
        virtual void collectPlugins() override;

    protected:
        //! \copydoc BlackCore::IPluginManager::acceptedIids()
        virtual BlackMisc::CSequence<QString> acceptedIids() const override;

        //! \copydoc BlackCore::IPluginManager::pluginDirectory()
        virtual QString pluginDirectory() const override;

    private:
        //! Extended data for plugin
        struct PluginExtended
        {
            BlackMisc::Weather::CWeatherDataPluginInfo info;
            QHash<QString, BlackMisc::CVariant> storage; //!< Permanent plugin storage - data stored here will be kept even when plugin is unloaded
        };

        QMap<QString, PluginExtended> m_plugins; //!< Id <-> extended data pairs
    };

} // namespace

#endif // guard
