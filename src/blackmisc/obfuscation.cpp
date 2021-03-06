/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "blackmisc/obfuscation.h"
#include "blackmisc/thirdparty/simplecrypt.h"
#include <QStringBuilder>

using namespace BlackMisc::ThirdParty;

namespace BlackMisc
{
    QString CObfuscation::decode(const QString &inString, bool trimmed)
    {
        if (!inString.startsWith(prefix())) { return trimmed ? inString.trimmed() : inString; }
        if (inString.length() == prefix().length()) { return QString(); }
        SimpleCrypt simpleCrypt(Key);
        const QString decoded = simpleCrypt.decryptToString(inString.mid(prefix().length()));
        return trimmed ? decoded.trimmed() : decoded;
    }

    QString CObfuscation::encode(const QString &inString, bool trimmed)
    {
        SimpleCrypt simpleCrypt(Key);
        const QString encrypted = simpleCrypt.encryptToString(trimmed ? inString.trimmed() : inString);
        return prefix() % encrypted;
    }

    const QString &CObfuscation::prefix()
    {
        static const QString obfuscated("OBF:");
        return obfuscated;
    }
} // ns
