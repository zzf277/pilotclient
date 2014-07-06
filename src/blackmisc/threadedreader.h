#ifndef BLACKMISC_THREADED_READER_H
#define BLACKMISC_THREADED_READER_H

//! \file

#include <QReadWriteLock>
#include <QDateTime>
#include <QTimer>
#include <QNetworkReply>
#include <QFuture>
#include <QCoreApplication>

// Header only class, to avoid orward instantiation across subprojects

namespace BlackMisc
{
    /*!
     * Support for threaded based reading and parsing tasks such
     * as data files via http, or file system and parsing (such as FSX models)
     */
    template <class FutureRet = void> class CThreadedReader
    {

    public:
        //! Destructor
        virtual ~CThreadedReader()
        {
            delete m_updateTimer;
            this->stop();
        }

        //! Thread safe, set update timestamp
        //! \remarks thread safe
        QDateTime getUpdateTimestamp() const
        {
            QReadLocker(&this->m_lock);
            return this->m_updateTimestamp;
        }

        //! Thread safe, set update timestamp
        //! \remarks thread safe
        void setUpdateTimestamp(const QDateTime &updateTimestamp)
        {
            QWriteLocker(&this->m_lock);
            this->m_updateTimestamp = updateTimestamp;
        }

        //! Thread safe, mark as stopped
        virtual void stop()
        {
            if (this->isStopped()) return;
            this->setStopFlag();
            this->setInterval(0);

            // shutdown pending
            if (this->m_pendingFuture.isRunning())
            {
                // cancel does not work with all futures, especially not with QConcurrent::run
                // the stop flag should the job
                // but I will cancel anyway
                this->m_pendingFuture.cancel();
            }
            if (this->m_pendingNetworkReply && this->m_pendingNetworkReply->isRunning())
            {
                this->m_pendingNetworkReply->abort();
                QCoreApplication::processEvents(QEventLoop::AllEvents, 250); // allow the abort to be called
            }

            // cancel or stop flag above should terminate QFuture
            this->m_pendingFuture.waitForFinished();

        }

        //! Thread safe, is in state stopped?
        //! \remarks thread safe
        bool isStopped() const
        {
            QReadLocker rl(&this->m_lock);
            return this->m_stopped;
        }

        /*!
         * Set the update time
         * \param updatePeriodMs <=0 stops the timer
         * \remarks thread safe
         */
        void setInterval(int updatePeriodMs)
        {
            Q_ASSERT(this->m_updateTimer);
            QWriteLocker(&this->m_lock);
            if (updatePeriodMs < 1)
                this->m_updateTimer->stop();
            else
                this->m_updateTimer->start(updatePeriodMs);
        }

        //! Get the timer interval (ms)
        //! \remarks thread safe
        int interval() const
        {
            QReadLocker rl(&this->m_lock);
            return this->m_updateTimer->interval();
        }

    protected:
        //! Constructor
        CThreadedReader() :
            m_updateTimer(nullptr), m_stopped(false), m_pendingNetworkReply(nullptr), m_lock(QReadWriteLock::Recursive)
        {
            this->m_updateTimer = new QTimer();
        }

        //! Has pending network replay
        //! \remarks thread safe
        void setPendingNetworkReply(QNetworkReply *reply)
        {
            QWriteLocker(&this->m_lock);
            this->m_pendingNetworkReply = reply;
        }

        //! Has pending operation
        //! \remarks thread safe
        void setPendingFuture(QFuture<FutureRet> future)
        {
            QWriteLocker(&this->m_lock);
            this->m_pendingFuture = future;
        }

        //! Thread safe, mark as to be stopped
        //! \remarks thread safe
        void setStopFlag()
        {
            QWriteLocker wl(&this->m_lock);
            this->m_stopped = true;
        }

        QDateTime m_updateTimestamp; //!< when was file / resource read
        QTimer *m_updateTimer; //!< update times
        bool m_stopped; //!< mark as stopped, threads should terminate
        QFuture<FutureRet> m_pendingFuture; //!< optional future to be stopped
        QNetworkReply *m_pendingNetworkReply; //!< optional future to be stopped
        mutable QReadWriteLock m_lock; //!< lock
    };
} // namespace

#endif // guard
