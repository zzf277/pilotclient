/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_NETWORK_SERVERLIST_H
#define BLACKMISC_NETWORK_SERVERLIST_H

#include "blackmisc/blackmiscexport.h"
#include "blackmisc/network/server.h"
#include "blackmisc/collection.h"
#include "blackmisc/sequence.h"

namespace BlackMisc
{
    namespace Network
    {
        //! Value object encapsulating a list of servers.
        class BLACKMISC_EXPORT CServerList :
            public CSequence<CServer>,
            public BlackMisc::Mixin::MetaType<CServerList>
        {
        public:
            BLACKMISC_DECLARE_USING_MIXIN_METATYPE(CServerList)

            //! Default constructor.
            CServerList();

            //! Construct from a base class object.
            CServerList(const CSequence<CServer> &other);

            //! Contains name
            bool containsName(const QString &name) const;

            //! Contains server with same address/port
            bool containsAddressPort(const CServer &server);

            //! Add if address not already exists
            void addIfAddressNotExists(const CServer &server);

            //! Add if address not already exists
            void addIfAddressNotExists(const CServerList &servers);
        };
    } //namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::Network::CServerList)
Q_DECLARE_METATYPE(BlackMisc::CCollection<BlackMisc::Network::CServer>)
Q_DECLARE_METATYPE(BlackMisc::CSequence<BlackMisc::Network::CServer>)

#endif //guard
