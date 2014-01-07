/* Copyright (C) 2013 VATSIM Community / authors
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "avatcstationlist.h"
#include "predicates.h"
using namespace BlackMisc::PhysicalQuantities;

namespace BlackMisc
{
    namespace Aviation
    {
        /*
         * Empty constructor
         */
        CAtcStationList::CAtcStationList() { }

        /*
         * Construct from base class object
         */
        CAtcStationList::CAtcStationList(const CSequence<CAtcStation> &other) :
            CSequence<CAtcStation>(other)
        { }

        /*
         * Register metadata
         */
        void CAtcStationList::registerMetadata()
        {
            qRegisterMetaType<BlackMisc::CSequence<CAtcStation>>();
            qDBusRegisterMetaType<BlackMisc::CSequence<CAtcStation>>();
            qRegisterMetaType<BlackMisc::CCollection<CAtcStation>>();
            qDBusRegisterMetaType<BlackMisc::CCollection<CAtcStation>>();
            qRegisterMetaType<CAtcStationList>();
            qDBusRegisterMetaType<CAtcStationList>();
        }

        /*
         * Find by callsign
         */
        CAtcStationList CAtcStationList::findByCallsign(const CCallsign &callsign) const
        {
            return this->findBy(&CAtcStation::getCallsign, callsign);
        }

        /*
         * Stations within range
         */
        CAtcStationList CAtcStationList::findWithinRange(const BlackMisc::Geo::ICoordinateGeodetic &coordinate, const PhysicalQuantities::CLength &range) const
        {
            return this->findBy([&](const CAtcStation & atcStation)
            {
                return greatCircleDistance(atcStation, coordinate) <= range;
            });
        }

        /*
         * Distances to own plane
         */
        void CAtcStationList::calculateDistancesToPlane(const Geo::CCoordinateGeodetic &position)
        {
            std::for_each(this->begin(), this->end(), [ & ](CAtcStation &station)
            {
                station.calculcateDistanceToPlane(position);
            });
        }

        /*
         * Merge with booking
         */
        int  CAtcStationList::mergeWithBooking(CAtcStation &bookedAtcStation)
        {
            int c = 0;
            bookedAtcStation.setOnline(false); // reset

            for (auto i = this->begin(); i != this->end(); ++i)
            {
                CAtcStation onlineAtcStation = *i;
                if (onlineAtcStation.getCallsign() != bookedAtcStation.getCallsign()) continue;

                // from online to booking
                bookedAtcStation.setOnline(true);
                bookedAtcStation.setFrequency(onlineAtcStation.getFrequency());

                // Logoff Zulu Time set?
                // comes directly from the online controller and is most likely more accurate
                if (!onlineAtcStation.getBookedUntilUtc().isNull())
                    bookedAtcStation.setBookedUntilUtc(onlineAtcStation.getBookedUntilUtc());

                // from booking to online
                if (!onlineAtcStation.isBookedNow() && bookedAtcStation.hasValidBookingTimes())
                {
                    if (onlineAtcStation.hasValidBookingTimes())
                    {
                        if (bookedAtcStation.isBookedNow())
                        {
                            // can't get any better
                            onlineAtcStation.setBookedFromUntil(bookedAtcStation);
                        }
                        else
                        {
                            // we already have some booking dates
                            CTime timeDiffBooking = bookedAtcStation.bookedWhen();
                            CTime timeDiffOnline = onlineAtcStation.bookedWhen();
                            if (timeDiffBooking.isNegativeWithEpsilonConsidered() && timeDiffOnline.isNegativeWithEpsilonConsidered())
                            {
                                // both in past
                                if (timeDiffBooking > timeDiffOnline)
                                    onlineAtcStation.setBookedFromUntil(bookedAtcStation);
                            }
                            else if (timeDiffBooking.isPositiveWithEpsilonConsidered() && timeDiffOnline.isPositiveWithEpsilonConsidered())
                            {
                                // both in future
                                if (timeDiffBooking < timeDiffOnline)
                                    onlineAtcStation.setBookedFromUntil(bookedAtcStation);
                            }
                            else if (timeDiffBooking.isPositiveWithEpsilonConsidered() && timeDiffOnline.isNegativeWithEpsilonConsidered())
                            {
                                // future booking is better than past booking
                                onlineAtcStation.setBookedFromUntil(bookedAtcStation);
                            }
                        }
                    }
                    else
                    {
                        // no booking info so far
                        onlineAtcStation.setBookedFromUntil(bookedAtcStation);
                    }
                }

                // both ways
                onlineAtcStation.syncronizeControllerData(bookedAtcStation);
                if (onlineAtcStation.hasValidDistance())
                    bookedAtcStation.setDistanceToPlane(onlineAtcStation.getDistanceToPlane());
                else if (bookedAtcStation.hasValidDistance())
                    onlineAtcStation.setDistanceToPlane(bookedAtcStation.getDistanceToPlane());

                // update
                *i = onlineAtcStation;
                c++;
            }

            // normally 1 expected, as I should find one online station
            // for this booking
            return c;
        }

    } // namespace
} // namespace
