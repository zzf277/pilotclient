/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "ui_rawfsdmessagescomponent.h"
#include "blackgui/components/rawfsdmessagescomponent.h"
#include "blackgui/guiapplication.h"
#include "blackcore/context/contextnetwork.h"
#include "blackmisc/directoryutils.h"

#include <QFileDialog>
#include <QDir>
#include <QStringList>
#include <QtGlobal>

using namespace BlackMisc;
using namespace BlackMisc::Network;
using namespace BlackCore;
using namespace BlackCore::Vatsim;

namespace BlackGui
{
    namespace Components
    {
        CRawFsdMessagesComponent::CRawFsdMessagesComponent(QWidget *parent) :
            QFrame(parent), ui(new Ui::CRawFsdMessagesComponent)
        {
            ui->setupUi(this);
            ui->cb_FileWritingMode->addItem(QApplication::translate("CRawFsdMessagesComponent", "None", nullptr), QVariant::fromValue(CRawFsdMessageSettings::None));
            ui->cb_FileWritingMode->addItem(QApplication::translate("CRawFsdMessagesComponent", "Truncate", nullptr), QVariant::fromValue(CRawFsdMessageSettings::Truncate));
            ui->cb_FileWritingMode->addItem(QApplication::translate("CRawFsdMessagesComponent", "Append", nullptr), QVariant::fromValue(CRawFsdMessageSettings::Append));
            ui->cb_FileWritingMode->addItem(QApplication::translate("CRawFsdMessagesComponent", "Timestamped", nullptr), QVariant::fromValue(CRawFsdMessageSettings::Timestamped));

            ui->cb_FilterPacketType->addItem("");
            ui->cb_FilterPacketType->addItems(CRawFsdMessage::getAllPacketTypes());
            ui->pte_RawFsdMessages->setMaximumBlockCount(m_maxDisplayedMessages);
            ui->le_MaxDisplayedMessages->setText(QString::number(m_maxDisplayedMessages));

            setupConnections();
            expandFilters(false);
            expandWritingToFile(false);
        }

        CRawFsdMessagesComponent::~CRawFsdMessagesComponent()
        { }

        void CRawFsdMessagesComponent::setupConnections()
        {

            connect(ui->le_FilterText, &QLineEdit::returnPressed, this, &CRawFsdMessagesComponent::changeStringFilter);
            connect(ui->cb_FilterPacketType, &QComboBox::currentTextChanged, this, &CRawFsdMessagesComponent::changePacketTypeFilter);
            connect(ui->gb_Filter, &QGroupBox::toggled, this, &CRawFsdMessagesComponent::expandFilters);
            connect(ui->gb_WriteToFile , &QGroupBox::toggled, this, &CRawFsdMessagesComponent::expandWritingToFile);
            connect(ui->pb_SelectFileDir, &QPushButton::clicked, this, &CRawFsdMessagesComponent::selectFileDir);
            connect(ui->le_MaxDisplayedMessages, &QLineEdit::returnPressed, this, &CRawFsdMessagesComponent::changeMaxDisplayedMessages);

            QValidator *validator = new QIntValidator(10, 200, this);
            ui->le_MaxDisplayedMessages->setValidator(validator);

            using namespace std::placeholders;
            QMetaObject::Connection c = sGui->getIContextNetwork()->connectRawFsdMessageSignal(this, std::bind(&CRawFsdMessagesComponent::addFsdMessage, this, _1));
            if (!c)
            {
                ui->pte_RawFsdMessages->appendPlainText(QStringLiteral("Could not connect to raw FSD message."));
                ui->pte_RawFsdMessages->appendPlainText(QStringLiteral("This is most likely because core is not running in this process."));
                ui->pte_RawFsdMessages->appendPlainText(QStringLiteral("Open this component again from the process running core."));
                return;
            }
            m_signalConnections.append(c);

            readSettings();
            // Connect them after settings are read. Otherwise they get called.
            connect(ui->cb_FileWritingMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CRawFsdMessagesComponent::changeFileWritingMode);
            connect(ui->cb_EnableRawFsdMessages, &QCheckBox::toggled, this, &CRawFsdMessagesComponent::enableRawFsdMessages);
        }

        void CRawFsdMessagesComponent::enableRawFsdMessages(bool enable)
        {
            ui->cb_FilterPacketType->setEnabled(enable);
            ui->le_FilterText->setEnabled(enable);
            ui->le_MaxDisplayedMessages->setEnabled(enable);
            ui->le_FileDir->setEnabled(enable);
            ui->pb_SelectFileDir->setEnabled(enable);
            ui->cb_FileWritingMode->setEnabled(enable);
            m_setting.setProperty(Vatsim::CRawFsdMessageSettings::IndexRawFsdMessagesEnabled, CVariant::fromValue(enable));
        }

        void CRawFsdMessagesComponent::expandFilters(bool expand)
        {
            if (expand)
            {
                ui->gl_Filters->setHorizontalSpacing(7);
                ui->gl_Filters->setVerticalSpacing(7);
                ui->gl_Filters->setContentsMargins(11, 11, 11, 11);
            }
            else
            {
                ui->gl_Filters->setHorizontalSpacing(0);
                ui->gl_Filters->setVerticalSpacing(0);
                ui->gl_Filters->setContentsMargins(0, 0, 0, 0);
            }

            for (int idx = 0; idx < ui->gl_Filters->count(); idx++)
            {
                QLayoutItem *const item = ui->gl_Filters->itemAt(idx);
                if (item->widget()) { item->widget()->setVisible(expand); }
            }
        }

        void CRawFsdMessagesComponent::expandWritingToFile(bool expand)
        {
            if (expand)
            {
                ui->gl_WriteToFile->setHorizontalSpacing(7);
                ui->gl_WriteToFile->setVerticalSpacing(7);
                ui->gl_WriteToFile->setContentsMargins(11, 11, 11, 11);
            }
            else
            {
                ui->gl_WriteToFile->setHorizontalSpacing(0);
                ui->gl_WriteToFile->setVerticalSpacing(0);
                ui->gl_WriteToFile->setContentsMargins(0, 0, 0, 0);
            }

            for (int idx = 0; idx < ui->gl_WriteToFile->count(); idx++)
            {
                QLayoutItem *const item = ui->gl_WriteToFile->itemAt(idx);
                if (item->widget()) { item->widget()->setVisible(expand); }
            }
        }

        void CRawFsdMessagesComponent::changeStringFilter()
        {
            m_filterString = ui->le_FilterText->text();
            filterDisplayedMessages();
        }

        void CRawFsdMessagesComponent::changePacketTypeFilter(const QString &type)
        {
            m_filterPacketType = type;
            filterDisplayedMessages();
        }

        void CRawFsdMessagesComponent::changeMaxDisplayedMessages()
        {
            int maxDisplayedMessages = 0;
            bool ok = false;
            maxDisplayedMessages = ui->le_MaxDisplayedMessages->text().toInt(&ok);
            if (ok)
            {
                m_maxDisplayedMessages = maxDisplayedMessages;
                filterDisplayedMessages();
            }
        }

        void CRawFsdMessagesComponent::filterDisplayedMessages()
        {
            CRawFsdMessageList filtered = m_buffer;
            if (! m_filterString.isEmpty()) { filtered = filtered.findByContainsString(m_filterString); }
            if (! m_filterPacketType.isEmpty()) { filtered = filtered.findByPacketType(m_filterPacketType); }
            ui->pte_RawFsdMessages->clear();
            // Append only the last messages up to maximum display size. Erase everything before.
            filtered.erase(filtered.begin(), filtered.end() - std::min(filtered.size(), m_maxDisplayedMessages));
            for (const CRawFsdMessage &rawFsdMessage : filtered)
            {
                ui->pte_RawFsdMessages->appendPlainText(rawFsdMessageToString(rawFsdMessage));
            }
        }

        void CRawFsdMessagesComponent::selectFileDir()
        {
            QString fileDir = ui->le_FileDir->text();
            fileDir = QFileDialog::getExistingDirectory(this, tr("Select File Directory"), fileDir);
            if (fileDir.isEmpty()) { return; }
            ui->le_FileDir->setText(fileDir);
            m_setting.setProperty(Vatsim::CRawFsdMessageSettings::IndexFileDir, CVariant::fromValue(fileDir));
        }

        void CRawFsdMessagesComponent::changeFileWritingMode()
        {
            const CRawFsdMessageSettings::FileWriteMode mode = ui->cb_FileWritingMode->currentData().value<CRawFsdMessageSettings::FileWriteMode>();
            m_setting.setProperty(Vatsim::CRawFsdMessageSettings::IndexFileWriteMode, CVariant::fromValue(mode));
        }

        void CRawFsdMessagesComponent::addFsdMessage(const CRawFsdMessage &rawFsdMessage)
        {
            if (m_buffer.size() == m_maxDisplayedMessages) { m_buffer.pop_front(); }
            m_buffer.push_backMaxElements(rawFsdMessage, m_maxDisplayedMessages);

            if (! m_filterPacketType.isEmpty() && !rawFsdMessage.isPacketType(m_filterPacketType)) { return; }
            if (! m_filterString.isEmpty() && !rawFsdMessage.containsString(m_filterString)) { return; }
            ui->pte_RawFsdMessages->appendPlainText(rawFsdMessageToString(rawFsdMessage));
        }

        void CRawFsdMessagesComponent::readSettings()
        {
            const Vatsim::CRawFsdMessageSettings setting = m_setting.get();
            const bool enable = setting.areRawFsdMessagesEnabled();
            ui->cb_EnableRawFsdMessages->setChecked(enable);
            ui->cb_FilterPacketType->setEnabled(enable);
            ui->le_FilterText->setEnabled(enable);
            ui->le_MaxDisplayedMessages->setEnabled(enable);
            ui->le_FileDir->setEnabled(enable);
            ui->le_FileDir->setText(setting.getFileDir());
            ui->pb_SelectFileDir->setEnabled(enable);
            ui->cb_FileWritingMode->setEnabled(enable);
            const CRawFsdMessageSettings::FileWriteMode mode = setting.getFileWriteMode();
            ui->cb_FileWritingMode->setCurrentIndex(static_cast<int>(mode));
        }

        QString CRawFsdMessagesComponent::rawFsdMessageToString(const CRawFsdMessage &rawFsdMessage)
        {
            static const QString s("%1 %2");
            return s.arg(rawFsdMessage.getFormattedUtcTimestampHmsz(), rawFsdMessage.getRawMessage());
        }
    }
} // namespace
