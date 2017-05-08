/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackmisc/aviation/livery.h"
#include "blackmisc/compare.h"
#include "blackmisc/comparefunctions.h"
#include "blackmisc/db/datastoreutility.h"
#include "blackmisc/logcategory.h"
#include "blackmisc/logcategorylist.h"
#include "blackmisc/propertyindex.h"
#include "blackmisc/statusmessage.h"
#include "blackmisc/stringutils.h"
#include "blackmisc/variant.h"
#include "blackmisc/verify.h"

#include <QCoreApplication>
#include <QJsonValue>
#include <Qt>
#include <QtGlobal>
#include <tuple>
#include <QStringBuilder>

using namespace BlackMisc;
using namespace BlackMisc::Db;
using namespace BlackMisc::PhysicalQuantities;

namespace BlackMisc
{
    namespace Aviation
    {
        CLivery::CLivery()
        { }

        CLivery::CLivery(const QString &combinedCode, const CAirlineIcaoCode &airline, const QString &description) :
            CLivery(-1, combinedCode, airline, description, "", "", false)
        { }

        CLivery::CLivery(const QString &combinedCode, const CAirlineIcaoCode &airline, const QString &description, const QString &colorFuselage, const QString &colorTail, bool isMilitary) :
            CLivery(-1, combinedCode, airline, description, colorFuselage, colorTail, isMilitary)
        { }

        CLivery::CLivery(const QString &combinedCode, const CAirlineIcaoCode &airline, const QString &description, const CRgbColor &colorFuselage, const CRgbColor &colorTail, bool isMilitary) :
            CLivery(-1, combinedCode, airline, description, colorFuselage, colorTail, isMilitary)
        { }

        CLivery::CLivery(int dbKey, const QString &combinedCode, const CAirlineIcaoCode &airline, const QString &description, const QString &colorFuselage, const QString &colorTail, bool isMilitary) :
            IDatastoreObjectWithIntegerKey(dbKey),
            m_airline(airline),
            m_combinedCode(combinedCode.trimmed().toUpper()), m_description(description.trimmed()),
            m_colorFuselage(CRgbColor(colorFuselage)), m_colorTail(CRgbColor(colorTail)),
            m_military(isMilitary)
        { }

        CLivery::CLivery(int dbKey, const QString &combinedCode, const CAirlineIcaoCode &airline, const QString &description, const CRgbColor &colorFuselage, const CRgbColor &colorTail, bool isMilitary) :
            IDatastoreObjectWithIntegerKey(dbKey),
            m_airline(airline),
            m_combinedCode(combinedCode.trimmed().toUpper()), m_description(description.trimmed()),
            m_colorFuselage(colorFuselage), m_colorTail(colorTail),
            m_military(isMilitary)
        { }

        QString CLivery::getCombinedCodePlusInfo() const
        {
            QString s = this->getCombinedCode();
            if (!this->getDescription().isEmpty())
            {
                s += " ";
                s += this->getDescription();
            }
            return s;
        }

        QString CLivery::getCombinedCodePlusInfoAndId() const
        {
            QString s = this->getCombinedCodePlusInfo();
            s += this->getDbKeyAsStringInParentheses(" ");
            return s;
        }

        bool CLivery::isContainedInSimplifiedAirlineName(const QString &candidate) const
        {
            return this->getAirlineIcaoCode().isContainedInSimplifiedName(candidate);
        }

        bool CLivery::setAirlineIcaoCode(const CAirlineIcaoCode &airlineIcao)
        {
            if (m_airline == airlineIcao) { return false; }
            m_airline = airlineIcao;
            return true;
        }

        bool CLivery::hasColorFuselage() const
        {
            return m_colorFuselage.isValid();
        }

        bool CLivery::hasColorTail() const
        {
            return m_colorTail.isValid();
        }

        bool CLivery::hasValidColors() const
        {
            return this->hasColorFuselage() && this->hasColorTail();
        }

        bool CLivery::matchesCombinedCode(const QString &candidate) const
        {
            if (candidate.isEmpty() || !this->hasCombinedCode()) { return false; }
            QString c(candidate.trimmed().toUpper());
            return c == this->m_combinedCode;
        }

        bool CLivery::matchesColors(const CRgbColor &fuselage, const CRgbColor &tail) const
        {
            return this->getColorFuselage() == fuselage && this->getColorTail() == tail;
        }

        QString CLivery::convertToQString(bool i18n) const
        {
            static const QString livery("Livery");
            static const QString liveryI18n(QCoreApplication::translate("Aviation", "Livery"));

            return (i18n ? liveryI18n : livery) %
                   QLatin1String(" cc: '") % m_combinedCode %
                   QLatin1String("' airline: '") % m_airline.toQString(i18n) %
                   QLatin1String("' desc.: '") % m_description %
                   QLatin1String("' F: '") % m_colorFuselage.hex() %
                   QLatin1String("' T: '") % m_colorTail.hex() %
                   QLatin1String("' Mil: ") % boolToYesNo(this->isMilitary());

            // force strings for translation in resource files
            (void)QT_TRANSLATE_NOOP("Aviation", "Livery");
        }

        bool CLivery::hasCompleteData() const
        {
            return !m_description.isEmpty() && !m_combinedCode.isEmpty();
        }

        CStatusMessageList CLivery::validate() const
        {
            static const CLogCategoryList cats(CLogCategoryList(this).join({ CLogCategory::validation() }));
            CStatusMessageList msg;
            if (!hasCombinedCode()) { msg.push_back(CStatusMessage(cats, CStatusMessage::SeverityError, "Livery: missing livery code")); }
            if (!hasColorFuselage()) { msg.push_back(CStatusMessage(cats, CStatusMessage::SeverityWarning, "Livery: no fuselage color")); }
            if (!hasColorTail()) { msg.push_back(CStatusMessage(cats, CStatusMessage::SeverityWarning, "Livery: no tail color")); }
            if (this->isColorLivery())
            {
                if (!this->getAirlineIcaoCodeDesignator().isEmpty())
                {
                    // color livery, supposed to have empty airline
                    msg.push_back(CStatusMessage(cats, CStatusMessage::SeverityWarning, "Livery: color livery, but airline looks odd"));
                }
            }
            else
            {
                msg.push_back(m_airline.validate());
            }
            return msg;
        }

        bool CLivery::hasValidAirlineDesignator() const
        {
            return m_airline.hasValidDesignator();
        }

        bool CLivery::hasCombinedCode() const
        {
            Q_ASSERT_X(!m_combinedCode.startsWith("." + standardLiveryMarker()), Q_FUNC_INFO, "illegal combined code");
            return !m_combinedCode.isEmpty();
        }

        bool CLivery::isAirlineLivery() const
        {
            return (this->m_airline.hasValidDesignator());
        }

        bool CLivery::isAirlineStandardLivery() const
        {
            if (isColorLivery()) { return false; }
            return (this->m_airline.hasValidDesignator() && this->m_combinedCode.endsWith(standardLiveryMarker()));
        }

        bool CLivery::isColorLivery() const
        {
            return m_combinedCode.startsWith(colorLiveryMarker());
        }

        double CLivery::getColorDistance(const CLivery &otherLivery) const
        {
            return this->getColorDistance(otherLivery.getColorFuselage(), otherLivery.getColorTail());
        }

        double CLivery::getColorDistance(const CRgbColor &fuselage, const CRgbColor &tail) const
        {
            if (!fuselage.isValid() || !tail.isValid()) { return 1.0; }
            if (this->getColorFuselage().isValid() && this->getColorTail().isValid())
            {
                if (this->matchesColors(fuselage, tail)) { return 0.0; } // avoid rounding
                const double xDist = this->getColorFuselage().colorDistance(fuselage);
                const double yDist = this->getColorTail().colorDistance(tail);
                const double d = xDist * xDist + yDist * yDist;
                return d / 2.0; // normalize to 0..1
            }
            else
            {
                return 1.0;
            }
        }

        CLivery CLivery::fromDatabaseJson(const QJsonObject &json, const QString &prefix)
        {
            if (!existsKey(json,  prefix))
            {
                // when using relationship, this can be null
                return CLivery();
            }

            const QString combinedCode(json.value(prefix + "combinedcode").toString());
            if (combinedCode.isEmpty())
            {
                CLivery liveryStub; // only consists of id, maybe key and timestamp
                liveryStub.setKeyAndTimestampFromDatabaseJson(json, prefix);
                return liveryStub;
            }

            const bool isColorLivery = combinedCode.startsWith(colorLiveryMarker());
            const QString description(json.value(prefix + "description").toString());
            const CRgbColor colorFuselage(json.value(prefix + "colorfuselage").toString());
            const CRgbColor colorTail(json.value(prefix + "colortail").toString());
            const bool military = CDatastoreUtility::dbBoolStringToBool(json.value(prefix + "military").toString());
            CAirlineIcaoCode airline;
            if (!isColorLivery) { airline = CAirlineIcaoCode::fromDatabaseJson(json, "al_"); }
            CLivery livery(combinedCode, airline, description, colorFuselage, colorTail, military);
            livery.setKeyAndTimestampFromDatabaseJson(json, prefix);

            // color liveries must have default ICAO, but airline liveries must have DB airline
            BLACK_VERIFY_X((livery.isColorLivery() && !livery.getAirlineIcaoCode().hasValidDbKey()) || (livery.isAirlineLivery() && livery.getAirlineIcaoCode().hasValidDbKey()), Q_FUNC_INFO, "inconsistent data");

            return livery;
        }

        bool CLivery::isValidCombinedCode(const QString &candidate)
        {
            if (candidate.isEmpty()) { return false; }
            if (candidate.startsWith(colorLiveryMarker()))
            {
                return candidate.length() > colorLiveryMarker().length() + 1;
            }
            else
            {
                if (candidate.count('.') != 1) { return false; }
                return candidate.length() > 2;
            }
        }

        const QString &CLivery::standardLiveryMarker()
        {
            static const QString s("_STD");
            return s;
        }

        QString CLivery::getStandardCode(const CAirlineIcaoCode &airline)
        {
            QString code(airline.getVDesignator());
            return code.isEmpty() ? "" : code.append('.').append(standardLiveryMarker());
        }

        const QString &CLivery::colorLiveryMarker()
        {
            static const QString s("_CC");
            return s;
        }

        const QString &CLivery::tempLiveryCode()
        {
            static const QString temp("_CC_NOCOLOR");
            return temp;
        }

        CVariant CLivery::propertyByIndex(const BlackMisc::CPropertyIndex &index) const
        {
            if (index.isMyself()) { return CVariant::from(*this); }
            if (IDatastoreObjectWithIntegerKey::canHandleIndex(index)) { return IDatastoreObjectWithIntegerKey::propertyByIndex(index); }
            ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexDescription:
                return CVariant::fromValue(m_description);
            case IndexAirlineIcaoCode:
                return m_airline.propertyByIndex(index.copyFrontRemoved());
            case IndexColorFuselage:
                return this->m_colorFuselage.propertyByIndex(index.copyFrontRemoved());;
            case IndexColorTail:
                return this->m_colorTail.propertyByIndex(index.copyFrontRemoved());
            case IndexCombinedCode:
                return CVariant::fromValue(this->m_combinedCode);
            case IndexIsMilitary:
                return CVariant::fromValue(this->m_military);
            default:
                return CValueObject::propertyByIndex(index);
            }
        }

        void CLivery::setPropertyByIndex(const CPropertyIndex &index, const CVariant &variant)
        {
            if (index.isMyself()) { (*this) = variant.to<CLivery>(); return; }
            if (IDatastoreObjectWithIntegerKey::canHandleIndex(index)) { IDatastoreObjectWithIntegerKey::setPropertyByIndex(index, variant); return; }
            ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexDescription:
                this->m_description = variant.toQString(false);
                break;
            case IndexAirlineIcaoCode:
                this->m_airline.setPropertyByIndex(index.copyFrontRemoved(), variant);
                break;
            case IndexColorFuselage:
                this->m_colorFuselage.setPropertyByIndex(index.copyFrontRemoved(), variant);
                break;
            case IndexColorTail:
                this->m_colorTail.setPropertyByIndex(index.copyFrontRemoved(), variant);
                break;
            case IndexCombinedCode:
                this->setCombinedCode(variant.toQString(false));
                break;
            case IndexIsMilitary:
                this->setMilitary(variant.toBool());
                break;
            default:
                CValueObject::setPropertyByIndex(index, variant);
                break;
            }
        }

        int CLivery::comparePropertyByIndex(const CPropertyIndex &index, const CLivery &compareValue) const
        {
            if (index.isMyself()) { return this->getCombinedCode().compare(compareValue.getCombinedCode()); }
            if (IDatastoreObjectWithIntegerKey::canHandleIndex(index)) { return IDatastoreObjectWithIntegerKey::comparePropertyByIndex(index, compareValue);}
            ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexDescription:
                return this->m_description.compare(compareValue.getDescription(), Qt::CaseInsensitive);
            case IndexAirlineIcaoCode:
                return this->m_airline.comparePropertyByIndex(index.copyFrontRemoved(), compareValue.getAirlineIcaoCode());
            case IndexColorFuselage:
                return this->m_colorFuselage.comparePropertyByIndex(index.copyFrontRemoved(), compareValue.getColorFuselage());
            case IndexColorTail:
                return this->m_colorTail.comparePropertyByIndex(index.copyFrontRemoved(), compareValue.getColorTail());
            case IndexCombinedCode:
                return this->getCombinedCode().compare(compareValue.getCombinedCode());
            case IndexIsMilitary:
                return Compare::compare(this->isMilitary(), compareValue.isMilitary());
            default:
                break;
            }
            Q_ASSERT_X(false, Q_FUNC_INFO, "No compare function");
            return 0;
        }

        void CLivery::updateMissingParts(const CLivery &otherLivery)
        {
            if (!this->hasValidDbKey() && otherLivery.hasValidDbKey())
            {
                // we have no DB data, but the other one has
                // so we change roles. We take the DB object as base, and update our parts
                CLivery copy(otherLivery);
                copy.updateMissingParts(*this);
                *this = copy;
                return;
            }

            if (!this->m_colorFuselage.isValid()) { this->setColorFuselage(otherLivery.getColorFuselage()); }
            if (!this->m_colorTail.isValid()) { this->setColorTail(otherLivery.getColorTail()); }
            if (this->m_combinedCode.isEmpty()) { this->setCombinedCode(otherLivery.getCombinedCode());}
            if (this->m_description.isEmpty()) { this->setDescription(otherLivery.getDescription());}

            this->m_airline.updateMissingParts(otherLivery.getAirlineIcaoCode());
            if (!this->hasValidDbKey())
            {
                this->setDbKey(otherLivery.getDbKey());
                this->setUtcTimestamp(otherLivery.getUtcTimestamp());
            }
        }

        QString CLivery::asHtmlSummary(const QString &separator) const
        {
            static const QString html = "%1%2Airline: %3";
            return html.arg(
                       this->getCombinedCodePlusInfoAndId(), separator,
                       this->getAirlineIcaoCode().getDesignator().isEmpty() ? "No airline" : this->getAirlineIcaoCode().getCombinedStringWithKey()
                   ).replace(" ", "&nbsp;");
        }

        int CLivery::calculateScore(const CLivery &otherLivery, bool preferColorLiveries) const
        {
            int score = 0;
            if (this->isLoadedFromDb() && otherLivery.isLoadedFromDb() && (this->getCombinedCode() == otherLivery.getCombinedCode()))
            {
                return 100; // exact
            }
            else
            {
                const double multiplier = preferColorLiveries ? 0.10 : 0.35;
                score += multiplier * this->getAirlineIcaoCode().calculateScore(otherLivery.getAirlineIcaoCode());
            }

            // max. 0..35 so far
            if (score == 0) { return 0; }
            if (this->isAirlineStandardLivery() && !preferColorLiveries) { score += 10; }

            // overrate non airline liveries / color liveries
            if (preferColorLiveries)
            {
                if (this->isColorLivery() || !this->hasValidAirlineDesignator())
                {
                    score += 20;
                }
            }

            // 0..45 so far
            if (!this->hasValidColors()) { return score; }
            const double cd = this->getColorDistance(otherLivery); // 0..1
            if (cd == 0)
            {
                score += 40;
            }
            else
            {
                score += (1.0 - cd) * 25; // 0..25
            }
            return score;
        }
    } // namespace
} // namespace
