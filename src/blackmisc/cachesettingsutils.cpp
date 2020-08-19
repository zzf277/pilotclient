/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "blackmisc/cachesettingsutils.h"
#include "blackmisc/settingscache.h"
#include "blackmisc/datacache.h"
#include <QRegularExpression>

namespace BlackMisc
{
    bool CCacheSettingsUtils::isSetting(const QString &fileName)
    {
        if (!fileName.contains(CSettingsCache::relativeFilePath())) { return false; }
        return fileName.contains(binSettings(), Qt::CaseInsensitive);
    }

    bool CCacheSettingsUtils::isCache(const QString &fileName)
    {
        if (!fileName.contains(CDataCache::relativeFilePath())) { return false; }
        return fileName.contains(binData(), Qt::CaseInsensitive);
    }

    const QString &CCacheSettingsUtils::binSettings()
    {
        static const QString s("bin/settings/");
        return s;
    }

    const QString &CCacheSettingsUtils::binData()
    {
        static const QString s("bin/data/");
        return s;
    }

    QString CCacheSettingsUtils::relativeSettingsPath(const QString &fileName)
    {
        if (!isSetting(fileName)) { return QString(); }
        const int index = fileName.lastIndexOf(binSettings());
        if (index < 0) { return QString(); }
        return fileName.mid(index);
    }

    QString CCacheSettingsUtils::relativeCachePath(const QString &fileName)
    {
        if (!isCache(fileName)) { return QString(); }
        const int index = fileName.lastIndexOf(binData());
        if (index < 0) { return QString(); }
        return fileName.mid(index);
    }

    QString CCacheSettingsUtils::otherVersionSettingsFileName(const CApplicationInfo &info, const QString &mySettingFile)
    {
        const QString relativeMySetting = relativeSettingsPath(mySettingFile);
        return otherVersionFileName(info, relativeMySetting);
    }

    QString CCacheSettingsUtils::otherVersionCacheFileName(const CApplicationInfo &info, const QString &myCacheFile)
    {
        const QString relativeMyCache = relativeCachePath(myCacheFile);
        return otherVersionFileName(info, relativeMyCache);
    }

    bool CCacheSettingsUtils::hasOtherVersionSettingsFile(const CApplicationInfo &info, const QString &mySettingFile)
    {
        return !otherVersionSettingsFileName(info, mySettingFile).isEmpty();
    }

    bool CCacheSettingsUtils::hasOtherVersionCacheFile(const CApplicationInfo &info, const QString &myCacheFile)
    {
        return !otherVersionCacheFileName(info, myCacheFile).isEmpty();
    }

    QString CCacheSettingsUtils::otherVersionFileName(const CApplicationInfo &info, const QString &relativeFileName)
    {
        thread_local const QRegularExpression re("bin$");
        if (relativeFileName.isEmpty()) { return {}; }
        QString otherFile = info.getApplicationDataDirectory();
        otherFile.replace(re, relativeFileName);
        const QFileInfo fi(otherFile);
        if (!fi.isFile()) { return {}; }
        if (fi.exists()) { return fi.absoluteFilePath(); }
        return {};
    }

    QString CCacheSettingsUtils::otherVersionSettingsFileContent(const CApplicationInfo &info, const QString &mySettingFile)
    {
        const QString file = otherVersionSettingsFileName(info, mySettingFile);
        if (file.isEmpty()) { return {}; }
        const QString jsonStr = CFileUtils::readFileToString(file);
        return jsonStr;
    }

    QString CCacheSettingsUtils::otherVersionCacheFileContent(const CApplicationInfo &info, const QString &myCacheFile)
    {
        const QString file = otherVersionCacheFileName(info, myCacheFile);
        if (file.isEmpty()) { return {}; }
        const QString jsonStr = CFileUtils::readFileToString(file);
        return jsonStr;
    }
} // namespace
