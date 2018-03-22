/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackmisc/aviation/aircraftsituationlist.h"
#include "blackmisc/aviation/aircraftsituation.h"
#include "blackmisc/geo/elevationplane.h"
#include <tuple>

using namespace BlackMisc::PhysicalQuantities;
using namespace BlackMisc::Geo;

namespace BlackMisc
{
    namespace Aviation
    {
        CAircraftSituationList::CAircraftSituationList()
        { }

        CAircraftSituationList::CAircraftSituationList(const CSequence<CAircraftSituation> &other) :
            CSequence<CAircraftSituation>(other)
        { }

        CAircraftSituationList::CAircraftSituationList(std::initializer_list<CAircraftSituation> il) :
            CSequence<CAircraftSituation>(il)
        { }

        int CAircraftSituationList::setGroundElevationChecked(const CElevationPlane &elevationPlane, qint64 newerThanAdjustedMs)
        {
            if (elevationPlane.isNull()) { return 0; }
            int c = 0;
            for (CAircraftSituation &s : *this)
            {
                if (newerThanAdjustedMs >= 0 && s.getAdjustedMSecsSinceEpoch() <= newerThanAdjustedMs) { continue; }
                const bool set = s.setGroundElevationChecked(elevationPlane);
                if (set) { c++; }
            }
            return c;
        }

        int CAircraftSituationList::adjustGroundFlag(const CAircraftParts &parts, double timeDeviationFactor)
        {
            int c = 0;
            for (CAircraftSituation &situation : *this)
            {
                if (situation.adjustGroundFlag(parts, timeDeviationFactor)) { c++; };
            }
            return c;
        }

        int CAircraftSituationList::extrapolateGroundFlag()
        {
            if (this->isEmpty()) { return 0; }
            CAircraftSituationList withInfo = this->findByInboundGroundInformation(true);
            withInfo.sortLatestFirst();
            if (withInfo.isEmpty()) { return 0; }
            const CAircraftSituation latest = withInfo.front();

            int c = 0;
            for (CAircraftSituation &situation : *this)
            {
                if (situation.isNewerThanAdjusted(latest))
                {
                    situation.setOnGround(latest.getOnGround(), latest.getOnGroundDetails());
                    c++;
                }
            }
            return c;
        }

        CAircraftSituationList CAircraftSituationList::findByInboundGroundInformation(bool hasGroundInfo) const
        {
            return this->findBy(&CAircraftSituation::hasInboundGroundInformation, hasGroundInfo);
        }
    } // namespace
} // namespace
