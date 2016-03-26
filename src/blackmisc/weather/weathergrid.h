/* Copyright (C) 2016
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_WEATHER_WEATHERGRID_H
#define BLACKMISC_WEATHER_WEATHERGRID_H

#include "gridpoint.h"
#include "blackmisc/blackmiscexport.h"
#include "blackmisc/sequence.h"
#include "blackmisc/geo/geoobjectlist.h"

namespace BlackMisc
{
    namespace Weather
    {
        /*!
         * Value object a list of weather grid points
         */
        class BLACKMISC_EXPORT CWeatherGrid :
            public CSequence<CGridPoint>,
            public BlackMisc::Mixin::MetaType<CWeatherGrid>
        {
        public:
            BLACKMISC_DECLARE_USING_MIXIN_METATYPE(CWeatherGrid)

            //! Default constructor.
            CWeatherGrid() = default;

            //! Initializer list constructor.
            CWeatherGrid(std::initializer_list<CGridPoint> il) : CSequence<CGridPoint>(il) {}

            //! Construct from a base class object.
            CWeatherGrid(const CSequence<CGridPoint> &other);

            //! \copydoc Geo::IGeoObjectList::findWithinRange
            CWeatherGrid findWithinRange(const BlackMisc::Geo::ICoordinateGeodetic &coordinate, const BlackMisc::PhysicalQuantities::CLength &range) const;

            //! \copydoc Geo::IGeoObjectList::findClosest
            CWeatherGrid findClosest(int number, const BlackMisc::Geo::ICoordinateGeodetic &coordinate) const;

            //! Clear weather grid
            static const BlackMisc::Weather::CWeatherGrid &getClearWeatherGrid();

            //! Thunderstorm grid
            static const BlackMisc::Weather::CWeatherGrid &getThunderStormGrid();
        };

    } //namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::Weather::CWeatherGrid)
Q_DECLARE_METATYPE(BlackMisc::CSequence<BlackMisc::Weather::CGridPoint>)

#endif //guard
