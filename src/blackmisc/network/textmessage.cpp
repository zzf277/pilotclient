/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "blackmisc/network/textmessage.h"
#include "blackmisc/aviation/callsign.h"
#include "blackmisc/aviation/comsystem.h"
#include "blackmisc/aviation/selcal.h"
#include "blackmisc/pq/constants.h"
#include "blackmisc/pq/physicalquantity.h"
#include "blackmisc/stringutils.h"

#include <Qt>
#include <QtGlobal>
#include <QStringBuilder>

using namespace BlackMisc::Aviation;
using namespace BlackMisc::PhysicalQuantities;

namespace BlackMisc
{
    namespace Network
    {
        CTextMessage::CTextMessage(const QString &message, const CFrequency &frequency, const CCallsign &senderCallsign)
            : m_senderCallsign(senderCallsign), m_frequency(frequency)
        {
            this->setMessage(message); // single place to modify message
            m_frequency.switchUnit(PhysicalQuantities::CFrequencyUnit::MHz());
        }

        CTextMessage::CTextMessage(const QString &message, const CCallsign &senderCallsign, const CCallsign &recipientCallsign)
            : m_senderCallsign(senderCallsign), m_recipientCallsign(recipientCallsign), m_frequency(0, nullptr)
        {
            this->setMessage(message); // single place to modify message
        }

        QString CTextMessage::convertToQString(bool i18n) const
        {
            if (this->isPrivateMessage())
            {
                return m_message %
                       u' ' % m_senderCallsign.toQString(i18n) %
                       u' ' % m_recipientCallsign.toQString(i18n);
            }
            return m_message % u' ' % m_frequency.toQString(i18n);
        }

        const QString &CTextMessage::swiftRelayMessage()
        {
            static const QString s("swift relayed: ");
            return s;
        }

        bool CTextMessage::isPrivateMessage() const
        {
            return !m_senderCallsign.isEmpty() && !m_recipientCallsign.isEmpty();
        }

        bool CTextMessage::isSupervisorMessage() const
        {
            return m_senderCallsign.isSupervisorCallsign();
        }

        void CTextMessage::markAsSent()
        {
            m_wasSent = true;
            if (!this->hasValidTimestamp())
            {
                this->setCurrentUtcTime();
            }
        }

        bool CTextMessage::isRelayedMessage() const
        {
            return m_relayedMessage || this->getMessage().startsWith(CTextMessage::swiftRelayMessage());
        }

        void CTextMessage::makeRelayedMessage(const CCallsign &partnerCallsign)
        {
            if (this->getMessage().startsWith(CTextMessage::swiftRelayMessage())) { return; }
            this->markAsRelayedMessage();
            this->setRecipientCallsign(partnerCallsign);
            m_recipientCallsign.setTypeHint(CCallsign::Aircraft);
            const QString sender = this->getSenderCallsign().asString();
            const QString newMessage = CTextMessage::swiftRelayMessage() % sender % u";" % this->getMessage();
            m_message = newMessage;
        }

        bool CTextMessage::relayedMessageToPrivateMessage()
        {
            if (!this->isRelayedMessage()) { return false; }
            const int index = m_message.indexOf(';');
            if (index < CTextMessage::swiftRelayMessage().length()) { return false; }
            if (m_message.length() <= index + 1) { return false; } // no next line
            const QString originalSender = m_message.left(index).remove(CTextMessage::swiftRelayMessage()).trimmed();
            this->setSenderCallsign(CCallsign(originalSender, CCallsign::Aircraft));
            m_message = m_message.mid(index + 1);
            return true;
        }

        bool CTextMessage::canBeAppended(const CTextMessage &textMessage) const
        {
            if (textMessage.isEmpty()) { return false; }
            if (this->getSenderCallsign() != textMessage.getSenderCallsign()) { return false; }
            if (this->isRadioMessage() && textMessage.isRadioMessage())
            {
                if (this->getFrequency() != textMessage.getFrequency()) { return false; }
                return true;
            }
            else if (this->isPrivateMessage() && textMessage.isPrivateMessage())
            {
                if (this->getRecipientCallsign() != textMessage.getRecipientCallsign()) { return false; }
                return true;
            }
            return false;
        }

        bool CTextMessage::appendIfPossible(const CTextMessage &textMessage)
        {
            if (textMessage.isEmpty()) { return false; }
            if (!this->canBeAppended(textMessage)) { return false; }
            m_message += u' ' % textMessage.getMessage();
            return true;
        }

        QString CTextMessage::getRecipientCallsignOrFrequency() const
        {
            if (!m_recipientCallsign.isEmpty()) { return m_recipientCallsign.asString(); }
            if (m_frequency.isNull()) { return {}; }
            return m_frequency.valueRoundedWithUnit(CFrequencyUnit::MHz(), 3);
        }

        bool CTextMessage::isSendToFrequency(const CFrequency &frequency) const
        {
            if (!this->isRadioMessage()) { return false; }
            return m_frequency == frequency;
        }

        bool CTextMessage::isSendToUnicom() const
        {
            return this->isSendToFrequency(CPhysicalQuantitiesConstants::FrequencyUnicom());
        }

        bool CTextMessage::hasValidRecipient() const
        {
            if (!m_recipientCallsign.isEmpty()) { return true; }
            return CComSystem::isValidCivilAviationFrequency(m_frequency);
        }

        bool CTextMessage::mentionsCallsign(const CCallsign &callsign) const
        {
            if (callsign.isEmpty()) { return false; }
            if (m_message.length() < callsign.asString().length()) { return false; }
            return m_message.contains(callsign.asString(), Qt::CaseInsensitive);
        }

        QString CTextMessage::getAsciiOnlyMessage() const
        {
            if (m_message.isEmpty()) { return {}; }
            return asciiOnlyString(simplifyAccents(m_message));
        }

        void CTextMessage::setMessage(const QString &message)
        {
            m_message = message.simplified().trimmed();
        }

        bool CTextMessage::isRadioMessage() const
        {
            return (CComSystem::isValidCivilAviationFrequency(m_frequency));
        }

        bool CTextMessage::isServerMessage() const
        {
            if (!this->isPrivateMessage()) { return false; }
            const CCallsign cs = this->getSenderCallsign();
            return (cs.asString().startsWith("SERVER", Qt::CaseInsensitive));
        }

        QString CTextMessage::asString(bool withSender, bool withRecipient, const QString &separator) const
        {
            QString s(this->getFormattedUtcTimestampHms());
            if (withSender)
            {
                if (!m_senderCallsign.isEmpty())
                {
                    if (!s.isEmpty()) { s += separator % m_senderCallsign.getStringAsSet(); }
                }
            }

            if (withRecipient)
            {
                if (!m_recipientCallsign.isEmpty())
                {
                    if (!s.isEmpty()) { s += separator % m_recipientCallsign.getStringAsSet(); }
                }
                else
                {
                    if (CComSystem::isValidCivilAviationFrequency(m_frequency))
                    {
                        if (!s.isEmpty()) { s += separator % m_frequency.valueRoundedWithUnit(3, true); }
                    }
                }
            } // to

            if (m_message.isEmpty()) { return s; }
            if (!s.isEmpty()) { s += separator % m_message; }
            return s;
        }

        CStatusMessage CTextMessage::asStatusMessage(bool withSender, bool withRecipient, const QString &separator) const
        {
            const QString m = this->asString(withSender, withRecipient, separator);
            return { this, CStatusMessage::SeverityInfo, m };
        }

        QString CTextMessage::asHtmlSummary(const QString &separator) const
        {
            return this->asString(true, true, separator);
        }

        void CTextMessage::toggleSenderRecipient()
        {
            qSwap(m_senderCallsign, m_recipientCallsign);
        }

        bool CTextMessage::isSelcalMessage() const
        {
            // some first level checks, before really parsing the message
            if (this->isEmpty()) { return false; }
            if (this->isPrivateMessage()) { return false; }
            if (m_message.length() > 15 || m_message.length() < 10) { return false; } // SELCAL AB-CD -> 12, I allow some more characters as I do not know wheter in real life it exactly matches
            return this->getSelcalCode().length() == 4;
        }

        bool CTextMessage::isSelcalMessageFor(const QString &selcal) const
        {
            if (!CSelcal::isValidCode(selcal)) return false;
            return selcal.toUpper() ==  this->getSelcalCode();
        }

        QString CTextMessage::getSelcalCode() const
        {
            // http://forums.vatsim.net/viewtopic.php?f=8&t=63467#p458062
            // When the ATC client sends a selcal, it goes out as a text message
            // on their primary frequency, formatted like so:
            // SELCAL AB-CD

            if (this->isEmpty()) return {};
            if (this->isPrivateMessage()) return {};
            if (!m_message.startsWith(QLatin1String("SELCAL"), Qt::CaseInsensitive)) return {};
            if (m_message.length() > 15 || m_message.length() < 10) return {}; // SELCAL AB-CD -> 12, I allow some more characters as I do not know wheter in real life it exactly matches
            QString candidate = removeChars(m_message, [](QChar c) { return !c.isLetter(); }); // SELCALABCD
            if (candidate.length() != 10) return {};
            return std::move(candidate).right(4).toUpper();
        }

        CIcon CTextMessage::toIcon() const
        {
            return m_senderCallsign.toIcon();
        }

        QPixmap CTextMessage::toPixmap() const
        {
            return m_senderCallsign.toPixmap();
        }

        CVariant CTextMessage::propertyByIndex(const BlackMisc::CPropertyIndex &index) const
        {
            if (index.isMyself()) { return CVariant::from(*this); }
            if (ITimestampBased::canHandleIndex(index)) { return ITimestampBased::propertyByIndex(index); }

            const ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexSenderCallsign: return m_senderCallsign.propertyByIndex(index.copyFrontRemoved());
            case IndexRecipientCallsign: return m_recipientCallsign.propertyByIndex(index.copyFrontRemoved());
            case IndexRecipientCallsignOrFrequency: return CVariant::fromValue(this->getRecipientCallsignOrFrequency());
            case IndexMessage: return CVariant::fromValue(m_message);
            default: return CValueObject::propertyByIndex(index);
            }
        }

        void CTextMessage::setPropertyByIndex(const CPropertyIndex &index, const CVariant &variant)
        {
            if (index.isMyself()) { (*this) = variant.to<CTextMessage>(); return; }
            if (ITimestampBased::canHandleIndex(index)) { ITimestampBased::setPropertyByIndex(index, variant); return; }

            const ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexSenderCallsign: m_senderCallsign.setPropertyByIndex(index.copyFrontRemoved(), variant); break;
            case IndexRecipientCallsign: m_recipientCallsign.setPropertyByIndex(index.copyFrontRemoved(), variant); break;
            case IndexMessage: m_message = variant.value<QString>(); break;
            default: CValueObject::setPropertyByIndex(index, variant); break;
            }
        }
    } // namespace
} // namespace
