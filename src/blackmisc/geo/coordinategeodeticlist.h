/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_GEO_COORDINATEGEODETICLIST_H
#define BLACKMISC_GEO_COORDINATEGEODETICLIST_H

#include "coordinategeodetic.h"
#include "geoobjectlist.h"
#include "blackmisc/blackmiscexport.h"
#include "blackmisc/collection.h"
#include "blackmisc/json.h"
#include "blackmisc/sequence.h"
#include "blackmisc/variant.h"

#include <QJsonObject>
#include <QMetaType>
#include <initializer_list>

namespace BlackMisc
{
    namespace Geo
    {
        //! Value object encapsulating a list of coordinates.
        class BLACKMISC_EXPORT CCoordinateGeodeticList :
            public CSequence<CCoordinateGeodetic>,
            public IGeoObjectList<CCoordinateGeodetic, CCoordinateGeodeticList>,
            public Mixin::MetaType<CCoordinateGeodeticList>,
            public Mixin::JsonOperators<CCoordinateGeodeticList>
        {
        public:
            BLACKMISC_DECLARE_USING_MIXIN_METATYPE(CCoordinateGeodeticList)

            //! Default constructor.
            CCoordinateGeodeticList();

            //! Construct by coordinates
            CCoordinateGeodeticList(std::initializer_list<CCoordinateGeodetic> coordinates);

            //! Construct from a base class object.
            CCoordinateGeodeticList(const CSequence<CCoordinateGeodetic> &other);
        };
    } //namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::Geo::CCoordinateGeodeticList)
Q_DECLARE_METATYPE(BlackMisc::CCollection<BlackMisc::Geo::CCoordinateGeodetic>)
Q_DECLARE_METATYPE(BlackMisc::CSequence<BlackMisc::Geo::CCoordinateGeodetic>)

#endif //guard
