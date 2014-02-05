#ifndef BLACKMISC_STATUSMESSAGE_H
#define BLACKMISC_STATUSMESSAGE_H

#include "valueobject.h"
#include <QDateTime>

namespace BlackMisc
{

    /*!
     * \brief Streamable status message, e.g. from Core -> GUI
     */
    class CStatusMessage : public CValueObject
    {
    public:
        /*!
         * \brief The StatusType enum
         */
        enum StatusType
        {
            TypeUnknown, // not set
            TypeUnspecific, // intentionally set
            TypeValidation,
            TypeTrafficNetwork,
            TypeSettings,
            TypeCore,
            TypeAudio,
            TypeGui
        };

        enum StatusSeverity
        {
            SeverityInfo,
            SeverityWarning,
            SeverityError
        };

        /*!
         * \brief Properties by index
         */
        enum ColumnIndex
        {
            IndexType,
            IndexTypeAsString,
            IndexSeverity,
            IndexSeverityAsString,
            IndexMessage,
            IndexTimestamp
        };

    private:
        StatusType m_type;
        StatusSeverity m_severity;
        QString m_message;
        QDateTime m_timestamp;

    public:
        /*!
         * \brief StatusMessage
         */
        CStatusMessage() :
            m_type(TypeUnknown), m_severity(SeverityInfo) {}

        /*!
         * \brief StatusMessage
         * \param type
         * \param severity
         * \param message
         */
        CStatusMessage(StatusType type, StatusSeverity severity, const QString &message);

        /*!
         * \brief Status type
         * \return
         */
        StatusType getType() const
        {
            return this->m_type;
        }

        /*!
         * \brief Status severity
         * \return
         */
        StatusSeverity getSeverity() const
        {
            return this->m_severity;
        }

        /*!
         * \brief Message
         * \return
         */
        QString getMessage() const
        {
            return this->m_message;
        }

        /*!
         * \brief Value hash
         * \return
         */
        virtual uint getValueHash() const;

        /*!
         * \brief As QVariant
         * \return
         */
        QVariant toQVariant() const
        {
            return QVariant::fromValue(*this);
        }

        //! \brief Type as string
        const QString &getTypeAsString() const;

        //! \brief representing icon
        virtual const QPixmap &toIcon() const override { return CStatusMessage::convertToIcon(*this); }

        //! \brief Type as string
        const QString &getSeverityAsString() const;

        //! \copydoc CValueObject::propertyByIndex(int)
        QVariant propertyByIndex(int index) const;

        //! \copydoc CValueObject::propertyByIndex(const QVariant, int)
        void propertyByIndex(const QVariant &variant, int index);

        //! \copydoc CValueObject::propertyByIndexAsString
        QString propertyByIndexAsString(int index, bool i18n) const;

        /*!
         * \brief Equal operator ==
         * \param other
         * @return
         */
        bool operator ==(const CStatusMessage &other) const;

        /*!
         * \brief Unequal operator ==
         * \param other
         * @return
         */
        bool operator !=(const CStatusMessage &other) const;

        /*!
         * \brief Register metadata
         */
        static void registerMetadata();

        /*!
         * \brief Validation error
         * \param message
         * \return
         */
        static CStatusMessage getValidationError(const QString &message);

        /*!
         * \brief Unspecific info message
         * \param message
         * \return
         */
        static CStatusMessage getInfoMessage(const QString &message);

        //! \representing icon
        static const QPixmap &convertToIcon(const CStatusMessage &statusMessage);

    protected:
        /*!
         * \brief Marshall to DBus
         * \param argument
         */
        virtual void marshallToDbus(QDBusArgument &arg) const;

        /*!
         * \brief Unmarshall from DBus
         * \param argument
         */
        virtual void unmarshallFromDbus(const QDBusArgument &arg);

        /*!
         * \brief Convert to String
         * \param i18n
         * \return
         */
        virtual QString convertToQString(bool i18n = false) const;

        /*!
         * \copydoc CValueObject::getMetaTypeId
         */
        virtual int getMetaTypeId() const;

        /*!
         * \copydoc CValueObject::isA
         */
        virtual bool isA(int metaTypeId) const;

        /*!
         * \copydoc CValueObject::compareImpl
         */
        virtual int compareImpl(const CValueObject &other) const;
    };

}

Q_DECLARE_METATYPE(BlackMisc::CStatusMessage)

#endif // guard
