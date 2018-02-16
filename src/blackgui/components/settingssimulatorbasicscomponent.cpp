/* Copyright (C) 2016
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "settingssimulatorbasicscomponent.h"
#include "ui_settingssimulatorbasicscomponent.h"
#include "blackmisc/simulation/xplane/xplaneutil.h"
#include "blackmisc/fileutils.h"
#include "blackmisc/logmessage.h"
#include "blackconfig/buildconfig.h"

#include <QRegularExpression>

using namespace BlackMisc;
using namespace BlackMisc::Simulation;
using namespace BlackMisc::Simulation::FsCommon;
using namespace BlackMisc::Simulation::Settings;
using namespace BlackMisc::Simulation::XPlane;
using namespace BlackConfig;

namespace BlackGui
{
    namespace Components
    {
        const CLogCategoryList &CSettingsSimulatorBasicsComponent::getLogCategories()
        {
            static const CLogCategoryList cats({ CLogCategory::guiComponent(), CLogCategory::wizard() });
            return cats;
        }

        CSettingsSimulatorBasicsComponent::CSettingsSimulatorBasicsComponent(QWidget *parent) :
            COverlayMessagesFrame(parent),
            ui(new Ui::CSettingsSimulatorBasicsComponent)
        {
            ui->setupUi(this);
            this->setSmallLayout(true); // no disadvantage, so I always set it
            ui->comp_SimulatorSelector->setMode(CSimulatorSelector::RadioButtons);
            ui->comp_SimulatorSelector->setRememberSelection(true);
            ui->comp_SimulatorSelector->setToLastSelection();
            connect(ui->pb_ExcludeFileDialog, &QPushButton::clicked, this, &CSettingsSimulatorBasicsComponent::excludeFileDialog);
            connect(ui->pb_ModelFileDialog, &QPushButton::clicked, this, &CSettingsSimulatorBasicsComponent::modelFileDialog);
            connect(ui->pb_SimulatorFileDialog, &QPushButton::clicked, this, &CSettingsSimulatorBasicsComponent::simulatorFileDialog);
            connect(ui->pb_Save, &QPushButton::clicked, this, &CSettingsSimulatorBasicsComponent::save);
            connect(ui->pb_Reset, &QPushButton::clicked, this, &CSettingsSimulatorBasicsComponent::reset);
            connect(ui->pb_CopyDefaults, &QPushButton::clicked, this, &CSettingsSimulatorBasicsComponent::copyDefaults);
            connect(ui->pb_AdjustModelDirectory, &QPushButton::clicked, this, &CSettingsSimulatorBasicsComponent::adjustModelDirectory);
            connect(ui->le_SimulatorDirectory, &QLineEdit::returnPressed, this, &CSettingsSimulatorBasicsComponent::simulatorDirectoryEntered);
            connect(ui->comp_SimulatorSelector, &CSimulatorSelector::changed, this, &CSettingsSimulatorBasicsComponent::onSimulatorChanged);

            this->onSimulatorChanged();
        }

        CSettingsSimulatorBasicsComponent::~CSettingsSimulatorBasicsComponent()
        { }

        void CSettingsSimulatorBasicsComponent::hideSelector(bool show)
        {
            ui->comp_SimulatorSelector->setVisible(show);
        }

        void CSettingsSimulatorBasicsComponent::setSimulator(const CSimulatorInfo &simulator)
        {
            Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "Need single simulator");
            ui->comp_SimulatorSelector->setValue(simulator);
        }

        void CSettingsSimulatorBasicsComponent::setSmallLayout(bool small)
        {
            ui->lbl_ExcludeDirectories->setWordWrap(small);
            ui->lbl_ModelDirectory->setWordWrap(small);
            ui->lbl_SimulatorDirectory->setWordWrap(small);
        }

        void CSettingsSimulatorBasicsComponent::simulatorFileDialog()
        {
            const QString startDirectory = CFileUtils::fixWindowsUncPath(this->getFileBrowserSimulatorDirectory());
            const QString dir = QFileDialog::getExistingDirectory(this, tr("Simulator directory"), startDirectory,
                                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            if (dir.isEmpty()) { return; }
            ui->le_SimulatorDirectory->setText(CFileUtils::normalizeFilePathToQtStandard(dir));
            this->adjustModelDirectory();
        }

        void CSettingsSimulatorBasicsComponent::modelFileDialog()
        {
            const QString startDirectory = CFileUtils::fixWindowsUncPath(this->getFileBrowserModelDirectory());
            const QString dir = QFileDialog::getExistingDirectory(this, tr("Model directory"), startDirectory,
                                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            if (dir.isEmpty()) { return; }
            const QStringList newDirs = this->addDirectory(dir, this->parseModelDirectories());
            this->displayModelDirectories(newDirs);
        }

        void CSettingsSimulatorBasicsComponent::excludeFileDialog()
        {
            const QString startDirectory = this->getFileBrowserModelDirectory();
            const QString dir = QFileDialog::getExistingDirectory(this, tr("Exclude directory"), startDirectory,
                                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            if (dir.isEmpty()) { return; }
            const QStringList newDirs = this->addDirectory(dir, this->parseModelDirectories());
            this->displayExcludeDirectoryPatterns(newDirs);
        }

        void CSettingsSimulatorBasicsComponent::simulatorDirectoryEntered()
        {
            const CSimulatorInfo simulator(ui->comp_SimulatorSelector->getValue());
            const QString simDir = CFileUtils::normalizeFilePathToQtStandard(ui->le_SimulatorDirectory->text().trimmed());
            ui->le_SimulatorDirectory->setText(simDir);
            this->displayDefaultValuesAsPlaceholder(simulator);
        }

        void CSettingsSimulatorBasicsComponent::save()
        {
            const CSimulatorInfo simulator(ui->comp_SimulatorSelector->getValue());
            CSimulatorSettings s = this->getSettings(simulator).getGenericSettings();
            const QString simulatorDir(ui->le_SimulatorDirectory->text().trimmed());
            const QStringList modelDirs(this->parseModelDirectories());
            const QStringList excludeDirs(this->parseDirectories(ui->pte_ExcludeDirectories->toPlainText()));

            const QStringList relativeDirs = CFileUtils::makeDirectoriesRelative(excludeDirs, this->getFileBrowserModelDirectory(), m_fileCaseSensitivity);
            s.setSimulatorDirectory(simulatorDir);
            s.setModelDirectories(modelDirs);
            s.setModelExcludeDirectories(relativeDirs);
            const CStatusMessage m = m_settings.setAndSaveSettings(s, simulator);
            if (!m.isEmpty()) { CLogMessage::preformatted(m); }

            // display as saved
            this->displaySettings(simulator);
        }

        void CSettingsSimulatorBasicsComponent::copyDefaults()
        {
            const CSimulatorInfo simulator(ui->comp_SimulatorSelector->getValue());
            const QString sd(m_settings.defaultSimulatorDirectory(simulator));
            ui->le_SimulatorDirectory->setText(CFileUtils::normalizeFilePathToQtStandard(sd));
            const QStringList md(m_settings.defaultModelDirectories(simulator));
            this->displayModelDirectories(md);
            const QStringList excludes(m_settings.defaultModelExcludeDirectoryPatterns(simulator));
            this->displayExcludeDirectoryPatterns(excludes);
        }

        void CSettingsSimulatorBasicsComponent::adjustModelDirectory()
        {
            const CSimulatorInfo simulator(ui->comp_SimulatorSelector->getValue());
            const QString simDir = this->getFileBrowserSimulatorDirectory();
            CSpecializedSimulatorSettings s = m_settings.getSpecializedSettings(simulator);
            s.setSimulatorDirectory(simDir);

            // There is not really a fixed place in the X-Plane install directory where models are put.
            // We just treat the whole X-Plane directory as model directory and search for models in all subdirectories recursively.

            const QStringList parsedDirectories = this->parseModelDirectories();
            const QStringList newDirs = parsedDirectories.size() > 1 ? parsedDirectories : this->removeDirectories(s.getModelDirectoriesFromSimulatorDirectoryOrDefault(), parsedDirectories);
            this->displayModelDirectories(newDirs);
        }

        void CSettingsSimulatorBasicsComponent::reset()
        {
            const CSimulatorInfo simulator(ui->comp_SimulatorSelector->getValue());
            m_settings.resetToDefaults(simulator);
            ui->le_SimulatorDirectory->clear();
            ui->pte_ModelDirectories->clear();
            ui->pte_ExcludeDirectories->clear();
            this->displayDefaultValuesAsPlaceholder(simulator);

            CLogMessage(this).info("Reset values for settings of %1") << simulator.toQString(true);
        }

        void CSettingsSimulatorBasicsComponent::onSimulatorChanged()
        {
            const CSimulatorInfo simulator(ui->comp_SimulatorSelector->getValue());
            this->displaySettings(simulator);
            this->displayDefaultValuesAsPlaceholder(simulator);
        }

        QStringList CSettingsSimulatorBasicsComponent::parseModelDirectories() const
        {
            return this->parseDirectories(ui->pte_ModelDirectories->toPlainText());
        }

        QStringList CSettingsSimulatorBasicsComponent::parseDirectories(const QString &rawString) const
        {
            const QString raw = rawString.trimmed();
            if (raw.isEmpty()) { return QStringList(); }
            QStringList dirs;
            thread_local const QRegularExpression regExp("\n|\r\n|\r");
            const QStringList rawLines = raw.split(regExp);
            for (const QString &l : rawLines)
            {
                const QString normalized = CFileUtils::normalizeFilePathToQtStandard(l);
                if (normalized.isEmpty()) { continue; }
                dirs.push_back(normalized);
            }
            dirs = CFileUtils::removeSubDirectories(dirs);
            return dirs;
        }

        QStringList CSettingsSimulatorBasicsComponent::addDirectory(const QString &directory, const QStringList &existingDirs)
        {
            const QString d(CFileUtils::normalizeFilePathToQtStandard(directory));
            QStringList dirs(existingDirs);
            if (d.isEmpty()) { return existingDirs; }
            if (!dirs.contains(d, m_fileCaseSensitivity))
            {
                dirs.push_back(d);
            }
            dirs.removeDuplicates();
            dirs.sort(m_fileCaseSensitivity);
            return dirs;
        }

        QStringList CSettingsSimulatorBasicsComponent::removeDirectory(const QString &directory, const QStringList &existingDirs)
        {
            const QString d(CFileUtils::normalizeFilePathToQtStandard(directory));
            return this->removeDirectories(QStringList({d}), existingDirs);
        }

        QStringList CSettingsSimulatorBasicsComponent::removeDirectories(const QStringList &removeDirectories, const QStringList &existingDirs)
        {
            if (existingDirs.isEmpty() || removeDirectories.isEmpty()) { return existingDirs; }
            const QStringList rDirs = CFileUtils::fixWindowsUncPaths(removeDirectories);
            QStringList dirs;
            for (const QString &dir : existingDirs)
            {
                if (rDirs.contains(dir, m_fileCaseSensitivity)) { continue; }
                dirs.push_back(dir);
            }
            dirs.removeDuplicates();
            dirs.sort(m_fileCaseSensitivity);
            return dirs;
        }

        void CSettingsSimulatorBasicsComponent::displayExcludeDirectoryPatterns(const QStringList &dirs)
        {
            QStringList cleanedDirs(dirs);
            cleanedDirs.removeDuplicates();
            cleanedDirs.sort(m_fileCaseSensitivity);
            const QString d = cleanedDirs.join("\n");
            ui->pte_ExcludeDirectories->setPlainText(d);
        }

        void CSettingsSimulatorBasicsComponent::displayModelDirectories(const QStringList &dirs)
        {
            QStringList cleanedDirs(dirs);
            cleanedDirs.removeDuplicates();
            cleanedDirs.sort(m_fileCaseSensitivity);
            const QString d = cleanedDirs.join("\n");
            ui->pte_ModelDirectories->setPlainText(d);
            this->displayDefaultValuesAsPlaceholder(ui->comp_SimulatorSelector->getValue());
        }

        CSpecializedSimulatorSettings CSettingsSimulatorBasicsComponent::getSettings(const CSimulatorInfo &simulator) const
        {
            const CSpecializedSimulatorSettings s = m_settings.getSpecializedSettings(simulator);
            return s;
        }

        void CSettingsSimulatorBasicsComponent::displaySettings(const CSimulatorInfo &simulator)
        {
            this->displayExcludeDirectoryPatterns(m_settings.getModelExcludeDirectoryPatternsIfNotDefault(simulator));
            this->displayModelDirectories(m_settings.getModelDirectoriesIfNotDefault(simulator));
            ui->le_SimulatorDirectory->setText(m_settings.getSimulatorDirectoryIfNotDefault(simulator));
        }

        void CSettingsSimulatorBasicsComponent::displayDefaultValuesAsPlaceholder(const CSimulatorInfo &simulator)
        {
            const QString simDir = this->getFileBrowserSimulatorDirectory();
            ui->le_SimulatorDirectory->setPlaceholderText(simDir.isEmpty() ? "Simulator directory" : simDir);

            //! \fixme correct version when Qt multiline placeholder is fixed https://bugreports.qt.io/browse/QTBUG-43817
            constexpr bool BrokenMultilinePlaceholder = true; // last checked with Qt 5.10

            // we take the settings and update to latest sim.directory
            CSpecializedSimulatorSettings settings = m_settings.getSpecializedSettings(simulator);
            settings.setSimulatorDirectory(simDir);

            const QStringList m = settings.getModelDirectoriesFromSimulatorDirectoryOrDefault();
            if (m.isEmpty())
            {
                ui->pte_ModelDirectories->setPlaceholderText("Model directories");
            }
            else
            {
                const QString ms(m.join("<br>"));
                if (BrokenMultilinePlaceholder && m.size() > 1)
                {
                    ui->pte_ModelDirectories->setToolTip(ms);
                    ui->pte_ModelDirectories->setPlaceholderText("See tooltip for defaults");
                }
                else
                {
                    ui->pte_ModelDirectories->setPlaceholderText(ms);
                }
            }

            const QStringList e = settings.getDefaultModelExcludeDirectoryPatterns();
            if (e.isEmpty())
            {
                ui->pte_ExcludeDirectories->setPlaceholderText("Exclude directories");
            }
            else
            {
                const QString es(e.join("<br>"));
                if (BrokenMultilinePlaceholder && e.size() > 1)
                {
                    //! \fixme correct version when Qt multiline placeholder is fixed https://bugreports.qt.io/browse/QTBUG-43817
                    ui->pte_ExcludeDirectories->setToolTip(es);
                    ui->pte_ExcludeDirectories->setPlaceholderText("See tooltip for defaults");
                }
                else
                {
                    ui->pte_ExcludeDirectories->setPlaceholderText(es);
                }
            }
        }

        QString CSettingsSimulatorBasicsComponent::getFileBrowserModelDirectory() const
        {
            const CSimulatorInfo simulator(ui->comp_SimulatorSelector->getValue());
            const QStringList modelDirs(this->parseDirectories(ui->pte_ModelDirectories->toPlainText()));
            QString md = modelDirs.isEmpty() ? "" : modelDirs.first();
            if (md.isEmpty())
            {
                md = m_settings.getFirstModelDirectoryOrDefault(simulator);
            }
            if (md.isEmpty())
            {
                md = this->getFileBrowserSimulatorDirectory();
            }
            return CFileUtils::normalizeFilePathToQtStandard(md);
        }

        QString CSettingsSimulatorBasicsComponent::getFileBrowserSimulatorDirectory() const
        {
            const CSimulatorInfo simulator(ui->comp_SimulatorSelector->getValue());
            QString sd(ui->le_SimulatorDirectory->text().trimmed());
            if (sd.isEmpty())
            {
                sd = m_settings.getSimulatorDirectoryOrDefault(simulator);
            }
            return CFileUtils::normalizeFilePathToQtStandard(sd);
        }
    } // ns
} // ns
