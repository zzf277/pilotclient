/* Copyright (C) 2019
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "clientconnection.h"
#include <QNetworkDatagram>

using namespace BlackCore::Afv::Crypto;

namespace BlackCore
{
    namespace Afv
    {
        namespace Connection
        {
            CClientConnection::CClientConnection(const QString &apiServer, QObject *parent) :
                QObject(parent),
                m_apiServerConnection(apiServer, this)
            {
                qDebug() << "ClientConnection instantiated";

                //    connect(&m_apiServerConnection, &ApiServerConnection::authenticationFinished, this, &ClientConnection::apiConnectionFinished);
                //    connect(&m_apiServerConnection, &ApiServerConnection::addCallsignFinished, this, &ClientConnection::addCallsignFinished);
                //    connect(&m_apiServerConnection, &ApiServerConnection::removeCallsignFinished, this, &ClientConnection::removeCallsignFinished);

                connect(&m_voiceServerTimer, &QTimer::timeout, this, &CClientConnection::voiceServerHeartbeat);

                connect(&m_udpSocket, &QUdpSocket::readyRead, this, &CClientConnection::readPendingDatagrams);
                connect(&m_udpSocket, qOverload<QAbstractSocket::SocketError>(&QUdpSocket::error), this, &CClientConnection::handleSocketError);
            }

            void CClientConnection::connectTo(const QString &userName, const QString &password, const QString &callsign)
            {
                if (m_connection.m_connected)
                {
                    qDebug() << "Client already connected";
                    return;
                }

                m_connection.m_userName = userName;
                m_connection.m_callsign = callsign;
                bool result = m_apiServerConnection.connectTo(userName, password, m_networkVersion);
                if (!result) { return; }
                m_connection.m_tokens = m_apiServerConnection.addCallsign(m_connection.m_callsign);
                m_connection.m_authenticatedDateTimeUtc = QDateTime::currentDateTimeUtc();
                m_connection.createCryptoChannels();

                connectToVoiceServer();

                // taskServerConnectionCheck.Start();

                m_connection.m_connected = true;
                qDebug() << "Connected:" << callsign;
            }

            void CClientConnection::disconnectFrom(const QString &reason)
            {
                if (! m_connection.m_connected)
                {
                    qDebug() << "Client not connected";
                    return;
                }

                m_connection.m_connected = false;
                // TODO emit disconnected(reason)
                qDebug() << "Disconnected:" << reason;

                if (! m_connection.m_callsign.isEmpty())
                {
                    m_apiServerConnection.removeCallsign(m_connection.m_callsign);
                }

                // TODO connectionCheckCancelTokenSource.Cancel(); //Stops connection check loop
                disconnectFromVoiceServer();
                m_apiServerConnection.forceDisconnect();
                m_connection.m_tokens = {};

                qDebug() << "Disconnection complete";
            }

            bool CClientConnection::receiveAudioDto() const
            {
                return m_receiveAudioDto;
            }

            void CClientConnection::setReceiveAudioDto(bool receiveAudioDto)
            {
                m_receiveAudioDto = receiveAudioDto;
            }

            void CClientConnection::updateTransceivers(const QString &callsign, const QVector<TransceiverDto> &transceivers)
            {
                m_apiServerConnection.updateTransceivers(callsign, transceivers);
            }

            QVector<StationDto> CClientConnection::getAllAliasedStations()
            {
                return m_apiServerConnection.getAllAliasedStations();
            }

            void CClientConnection::connectToVoiceServer()
            {
                QHostAddress localAddress(QHostAddress::AnyIPv4);
                m_udpSocket.bind(localAddress);
                m_voiceServerTimer.start(3000);

                qDebug() << "Connected to voice server (" + m_connection.m_tokens.VoiceServer.addressIpV4 << ")";
            }

            void CClientConnection::disconnectFromVoiceServer()
            {
                m_voiceServerTimer.stop();
                m_udpSocket.disconnectFromHost();
                qDebug() << "All TaskVoiceServer tasks stopped";
            }

            void CClientConnection::readPendingDatagrams()
            {
                while (m_udpSocket.hasPendingDatagrams())
                {
                    QNetworkDatagram datagram = m_udpSocket.receiveDatagram();
                    processMessage(datagram.data());
                }
            }

            void CClientConnection::processMessage(const QByteArray &messageDdata, bool loopback)
            {
                CryptoDtoSerializer::Deserializer deserializer = CryptoDtoSerializer::deserialize(*m_connection.voiceCryptoChannel, messageDdata, loopback);

                if (deserializer.dtoNameBuffer == AudioRxOnTransceiversDto::getShortDtoName())
                {
                    // qDebug() << "Received audio data";
                    AudioRxOnTransceiversDto audioOnTransceiverDto = deserializer.getDto<AudioRxOnTransceiversDto>();
                    if (m_connection.m_receiveAudio && m_connection.m_connected)
                    {
                        emit audioReceived(audioOnTransceiverDto);
                    }
                }
                else if (deserializer.dtoNameBuffer == HeartbeatAckDto::getShortDtoName())
                {
                    m_connection.m_lastVoiceServerHeartbeatAckUtc = QDateTime::currentDateTimeUtc();
                    qDebug() << "Received voice server heartbeat";
                }
                else
                {
                    qWarning() << "Received unknown data:" << deserializer.dtoNameBuffer << deserializer.dataLength;
                }
            }

            void CClientConnection::handleSocketError(QAbstractSocket::SocketError error)
            {
                Q_UNUSED(error)
                qDebug() << "UDP socket error" << m_udpSocket.errorString();
            }

            void CClientConnection::voiceServerHeartbeat()
            {
                const QUrl voiceServerUrl("udp://" + m_connection.m_tokens.VoiceServer.addressIpV4);
                qDebug() << "Sending voice server heartbeat to" << voiceServerUrl.host();
                HeartbeatDto keepAlive;
                keepAlive.callsign = m_connection.m_callsign.toStdString();
                const QByteArray dataBytes = CryptoDtoSerializer::serialize(*m_connection.voiceCryptoChannel, CryptoDtoMode::AEAD_ChaCha20Poly1305, keepAlive);
                m_udpSocket.writeDatagram(dataBytes, QHostAddress(voiceServerUrl.host()), static_cast<quint16>(voiceServerUrl.port()));
            }
        } // ns
    } // ns
} // ns