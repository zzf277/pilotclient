/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */


#include "valueobject.h"
#include "propertyindexvariantmap.h"
#include "propertyindexlist.h"
#include "propertyindex.h"
#include "iconlist.h"
#include "blackmiscfreefunctions.h"

namespace BlackMisc
{

    /*
     * Stringify
     */
    QString CValueObject::toQString(bool i18n) const
    {
        return this->convertToQString(i18n);
    }

    /*
     * Stringify
     */
    QString CValueObject::toFormattedQString(bool i18n) const
    {
        return this->toQString(i18n);
    }

    /*
     * Stringify
     */
    std::string CValueObject::toStdString(bool i18n) const
    {
        return this->convertToQString(i18n).toStdString();
    }

    /*
     * Streaming
     */
    QString CValueObject::stringForStreaming() const
    {
        // simplest default implementation requires only one method
        return this->convertToQString();
    }

    /*
     * Setter for property by index
     */
    void CValueObject::setPropertyByIndex(const CVariant &variant, const CPropertyIndex &index)
    {
        if (index.isMyself())
        {
            this->convertFromCVariant(variant);
            return;
        }

        // not all classes have implemented nesting
        const QString m = QString("Property by index not found (setter), index: ").append(index.toQString());
        qFatal("%s", qPrintable(m));
    }

    /*
     * By index
     */
    CVariant CValueObject::propertyByIndex(const BlackMisc::CPropertyIndex &index) const
    {
        if (index.isMyself()) { return this->toCVariant(); }
        ColumnIndex i = index.frontCasted<ColumnIndex>();
        switch (i)
        {
        case IndexIcon:
            return this->toIcon().toCVariant();
        case IndexPixmap:
            return CVariant::from(this->toPixmap());
        case IndexString:
            return CVariant(this->toQString());
        default:
            break;
        }

        // not all classes have implemented nesting
        const QString m = QString("Property by index not found, index: ").append(index.toQString());
        qFatal("%s", qPrintable(m));
        return CVariant(m); // avoid compiler warning
    }

    /*
     * By index as string
     */
    QString CValueObject::propertyByIndexAsString(const CPropertyIndex &index, bool i18n) const
    {
        // default implementation, requires propertyByIndex
        return this->propertyByIndex(index).toQString(i18n);
    }

    /*
     * Variant equal property index?
     */
    bool CValueObject::equalsPropertyByIndex(const CVariant &compareValue, const CPropertyIndex &index) const
    {
        return this->propertyByIndex(index) == compareValue;
    }

    /*
     * Compare
     */
    int compare(const CValueObject &v1, const CValueObject &v2)
    {
        if (v1.isA(v2.getMetaTypeId()))
        {
            return v2.compareImpl(v1) * -1;
        }
        else if (v2.isA(v1.getMetaTypeId()))
        {
            return v1.compareImpl(v2);
        }
        else
        {
            Q_ASSERT_X(false, Q_FUNC_INFO, "Attempt to compare between instances of unrelated classes");
            return 0;
        }
    }

    /*
     * apply, return changed indexes
     */
    CPropertyIndexList CValueObject::apply(const BlackMisc::CPropertyIndexVariantMap &indexMap, bool skipEqualValues)
    {
        CPropertyIndexList changed;
        if (indexMap.isEmpty()) return changed;

        const auto &map = indexMap.map();
        for (auto it = map.begin(); it != map.end(); ++it)
        {
            const CVariant value = it.value().toCVariant();
            const CPropertyIndex index = it.key();
            if (skipEqualValues)
            {
                bool equal = this->equalsPropertyByIndex(value, index);
                if (equal) { continue; }
            }
            this->setPropertyByIndex(value, index);
            changed.push_back(index);
        }
        return changed;
    }

    /*
     * Icon
     */
    CIcon CValueObject::toIcon() const
    {
        return CIconList::iconByIndex(CIcons::StandardIconUnknown16);
    }

    /*
     * Pixmap
     */
    QPixmap CValueObject::toPixmap() const
    {
        return this->toIcon().toPixmap();
    }

    /*
     * from DBus
     */
    const QDBusArgument &operator>>(const QDBusArgument &argument, CValueObject &valueObject)
    {
        argument.beginStructure();
        valueObject.unmarshallFromDbus(argument);
        argument.endStructure();
        return argument;
    }

    /*
     * to DBus
     */
    QDBusArgument &operator<<(QDBusArgument &argument, const CValueObject &valueObject)
    {
        argument.beginStructure();
        valueObject.marshallToDbus(argument);
        argument.endStructure();
        return argument;
    }

    /*
     * to CVariant
     */
    CVariant CValueObject::toCVariant() const
    {
        return CVariant(this->toQVariant());
    }

    /*
     * from CVariant
     */
    void CValueObject::convertFromCVariant(const CVariant &variant)
    {
        this->convertFromQVariant(variant.getQVariant());
    }

    /*
     * Implementations of pure virtual functions
     */
    uint CValueObject::getValueHash() const { return 0; }
    int CValueObject::compareImpl(const CValueObject &) const { return 0; }
    void CValueObject::marshallToDbus(QDBusArgument &) const {}
    void CValueObject::unmarshallFromDbus(const QDBusArgument &) {}
}
