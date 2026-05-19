#pragma once

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

class SseClient : public QObject
{
    Q_OBJECT

public:
    explicit SseClient(QObject *parent = nullptr);
    void start(const QNetworkRequest &request, const QByteArray &payload);
    void stop();

signals:
    void eventReceived(const QString &eventName, const QByteArray &payload);
    void finished();
    void errorOccurred(const QString &message);

private:
    void onReadyRead();

    QNetworkAccessManager *m_manager = nullptr;
    QNetworkReply *m_reply = nullptr;
    QByteArray m_buffer;
};
