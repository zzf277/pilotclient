/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_SIMULATION_FSCOMMONUTIL_H
#define BLACKMISC_SIMULATION_FSCOMMONUTIL_H

#include "blackmisc/simulation/aircraftmodellist.h"
#include "blackmisc/logcategorylist.h"
#include "blackmisc/blackmiscexport.h"

#include <QSet>
#include <QStringList>

namespace BlackMisc
{
    namespace Simulation
    {
        namespace FsCommon
        {
            //! FS9/FSX/P3D utils
            class BLACKMISC_EXPORT CFsCommonUtil
            {
            public:
                //! Log categories
                static const BlackMisc::CLogCategoryList &getLogCategories();

                //! Constructor
                CFsCommonUtil() = delete;

                //! FSX directory obtained from registry
                static const QString &fsxDirFromRegistry();

                //! FSX directory from different sources
                static const QString &fsxDir();

                //! FSX's simObject directory from registry
                static const QString &fsxSimObjectsDirFromRegistry();

                //! FSX's simobject dir, resolved from multiple sources
                static const QString &fsxSimObjectsDir();

                //! FSX aircraft dir, relative to simulator directory
                static QString fsxSimObjectsDirFromSimDir(const QString &simDir);

                //! Exclude directories for simObjects
                static const QStringList &fsxSimObjectsExcludeDirectoryPatterns();

                //! P3D directory obtained from registry
                static const QString &p3dDirFromRegistry();

                //! P3D directory from different sources
                static const QString &p3dDir();

                //! P3D's simObject directory from registry
                static const QString &p3dSimObjectsDirFromRegistry();

                //! P3D's simObject dir, resolved from multiple sources
                static const QString &p3dSimObjectsDir();

                //! P3D's simObject dir and the add on dirs
                static QStringList p3dSimObjectsDirPlusAddOnSimObjectsDirs(const QString &simObjectsDir = "", const QString &versionHint = "v4");

                //! P3D aircraft dir, relative to simulator directory
                static QString p3dSimObjectsDirFromSimDir(const QString &simDir);

                //! Exclude directories for simObjects
                static const QStringList &p3dSimObjectsExcludeDirectoryPatterns();

                //! FS9 directory obtained from registry
                static const QString &fs9DirFromRegistry();

                //! FS9 directory obtained from multiple sources
                static const QString &fs9Dir();

                //! FS9's aircraft directory from registry
                static const QString &fs9AircraftDirFromRegistry();

                //! FS9's aircraft directory
                static const QString &fs9AircraftDir();

                //! FS9 aircraft dir, relative to simulator directory
                static QString fs9AircraftDirFromSimDir(const QString &simDir);

                //! Exclude directories for aircraft objects
                static const QStringList &fs9AircraftObjectsExcludeDirectoryPatterns();

                //! Adjust file directory
                static bool adjustFileDirectory(CAircraftModel &model, const QString &simObjectsDirectory);

                //! Adjust file directory
                static bool adjustFileDirectory(CAircraftModel &model, const QStringList &simObjectsDirectories);

                //! Copy the terrain probe
                static int copyFsxTerrainProbeFiles(const QString &simObjectDir, CStatusMessageList &messages);

                //! Find the config files (add-ons.cfg)
                //! \note "C:/Users/Joe Doe/AppData/Roaming/Lockheed Martin/Prepar3D v4"
                //! \param versionHint like "v4"
                static QSet<QString> findP3dAddOnConfigFiles(const QString &versionHint = "v4");

                //! Find the config files (simobjects.cfg)
                //! \note "C:/Users/Joe Doe/AppData/Roaming/Lockheed Martin/Prepar3D v4"
                //! \param versionHint like "v4"
                static QSet<QString> findP3dSimObjectsConfigFiles(const QString &versionHint = "v4");

                //! All PATH values from the config files
                static QSet<QString> allConfigFilesPathValues(const QStringList &configFiles, bool checked, const QString &pathPrefix);

                //! All add-on paths from the config files
                static QSet<QString> allP3dAddOnSimObjectPaths(const QStringList &addOnPaths, bool checked);

                //! All add-on paths from the config files
                static QSet<QString> allP3dAddOnSimObjectPaths(const QString &versionHint = "v4");

                //! All paths from the simobjects.cfg files
                static QSet<QString> allP3dSimObjectsConfigPaths(const QString &simulatorDir, const QString &versionHint = "v4");

                //! All add-on paths from the config files and the simobjects.cfg files
                static QSet<QString> allP3dSimObjectPaths(const QString &simulatorDir, const QString &versionHint = "v4");

                //! Find the config files (fsx.cfg)
                // C:/Users/Joe Doe/AppData/Roaming/Lockheed Martin/Prepar3D v4
                static QStringList findFsxConfigFiles();

                //! Get all the SimObjects paths from fsx.cfg
                // SimObjectPaths.0=SimObjects\Airplanes
                static QSet<QString> fsxSimObjectsPaths(const QStringList &fsxFiles, bool checked);

                //! Get all the SimObjects files from fsx.cfg
                // SimObjectPaths.0=SimObjects\Airplanes
                static QSet<QString> fsxSimObjectsPaths(const QString &fsxFile, bool checked);

                //! Validate aircraft.cfg entries
                //! \remark only for FSX/P3D/FS9 models
                static CStatusMessageList validateConfigFiles(const CAircraftModelList &models, CAircraftModelList &validModels, CAircraftModelList &invalidModels, bool ignoreEmptyFileNames, int stopAtFailedFiles, bool &stopped);

                //! Validate if known SimObjects path are used
                //! \remark only for P3D
                static CStatusMessageList validateP3DSimObjectsPath(const CAircraftModelList &models, CAircraftModelList &validModels, CAircraftModelList &invalidModels, bool ignoreEmptyFileNames, int stopAtFailedFiles, bool &stopped, const QString &simulatorDir);

                //! Validate if known SimObjects path are used
                //! \remark only for FSX
                static CStatusMessageList validateFSXSimObjectsPath(const CAircraftModelList &models, CAircraftModelList &validModels, CAircraftModelList &invalidModels, bool ignoreEmptyFileNames, int stopAtFailedFiles, bool &stopped);

            private:
                //! Utility functions @{
                static QSet<QString> findP3dConfigFiles(const QString &configFile, const QString &versionHint = "v4");
                //! @}

                //! Validate if known SimObjects path are used
                //! \remark only for P3D
                static CStatusMessageList validateSimObjectsPath(const QSet<QString> &simObjectDirs, const CAircraftModelList &models, CAircraftModelList &validModels, CAircraftModelList &invalidModels, bool ignoreEmptyFileNames, int stopAtFailedFiles, bool &stopped);

                //! Log the reading of config files
                static bool logConfigPathReading();
            };
        } // namespace
    } // namespace
} // namespace

#endif // guard
