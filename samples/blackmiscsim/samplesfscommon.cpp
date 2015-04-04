/* Copyright (C) 2014
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "samplesfscommon.h"
#include "blackmisc/sampleutils.h"
#include "blackmisc/simulation/fscommon/aircraftcfgentrieslist.h"
#include "blackmisc/simulation/fscommon/aircraftmapper.h"

#include <QDebug>
#include <QFuture>
#include <QTime>
#include <QTextStream>
#include <QTemporaryFile>

using namespace BlackMisc;
using namespace BlackMisc::Simulation::FsCommon;

namespace BlackSimTest
{

    /*
     * Samples
     */
    int CSamplesFsCommon::samples(QTextStream &streamOut, QTextStream &streamIn)
    {
        QString fsxDir = CSampleUtils::selectDirectory({"P:/FlightSimulatorX (MSI)/SimObjects", "P:/Temp/SimObjects"}, streamOut, streamIn);
        CAircraftMapper mapper;
        if (!mapper.changeCAircraftCfgEntriesDirectory(fsxDir))
        {
            streamOut << "Wrong or empty directoy " << fsxDir << endl;
            return 0;
        }

        streamOut << "start reading" << endl;
        QString input = streamIn.readLine();
        Q_UNUSED(input);

        streamOut << "reading directly" << endl;
        QTime time;
        time.start();
        streamOut << "reading " << mapper.getAircraftCfgEntriesList().getRootDirectory() << endl;
        mapper.readSimObjects();
        streamOut << "read entries: " << mapper.getAircraftCfgEntriesList().size() << " in " << time.restart() << "ms" << endl;

        CAircraftCfgEntriesList entriesList = mapper.getAircraftCfgEntriesList();
        QJsonDocument doc(entriesList.toJson());
        QByteArray jsonArray(doc.toJson());
        streamOut << "write JSON array with size " << jsonArray.size() << endl;
        QTemporaryFile tempFile;
        tempFile.open();
        tempFile.write(jsonArray);
        tempFile.close();
        streamOut << "written to " << tempFile.fileName() << " in " << time.restart() << "ms" <<  endl;

        // re-read
        tempFile.open();
        jsonArray = tempFile.readAll();
        doc = QJsonDocument::fromJson(jsonArray);
        entriesList.clear();
        entriesList.convertFromJson(doc.object());
        streamOut << "read JSON array with size " << jsonArray.size() << endl;
        streamOut << "read entries from disk: " << entriesList.size() << " in " << time.restart() << "ms" << endl;
        tempFile.close();
        return 0;
    }

} // namespace
