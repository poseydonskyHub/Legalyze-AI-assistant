#include "network/SseClient.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

SseClient::SseClient(QObject *parent)
    : QObject(parent),
      m_manager(new QNetworkAccessManager(this))
{
}

void SseClient::start(const QNetworkRequest &request, const QByteArray &payload)
{
    stop();
    m_reply = m_manager->post(request, payload);
    connect(m_reply, &QIODevice::readyRead, this, &SseClient::onReadyRead);
    connect(m_reply, &QNetworkReply::finished, this, &SseClient::finished);
}

void SseClient::stop()
{
    if (m_reply != nullptr) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    m_buffer.clear();
}

void SseClient::onReadyRead()
{
    if (m_reply == nullptr) {
        return;
    }

    m_buffer.append(m_reply->readAll());
}
