#ifndef CREQUESTMANAGER_H
#define CREQUESTMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkCacheMetaData>

class CRequestManager : public QObject
{
    Q_OBJECT

public:
    explicit CRequestManager(QObject *parent = nullptr);

    QNetworkReply* SendRequest(
        QObject* handler, 
        const QByteArray& verb, 
        const QUrl& url, 
        const QByteArray& payload = "", 
        QNetworkCacheMetaData::RawHeaderList headers = {});

    QNetworkReply* UploadRequest(
        QObject* handler,
        const QByteArray& verb,
        const QUrl& url,
        QIODevice& dataSource,
        QNetworkCacheMetaData::RawHeaderList headers = {});

	static const QByteArrayList& GetKnownHeaders() { return s_knownHeaders; }
    static const QByteArray GetKnownHeader(QNetworkRequest::KnownHeaders type);

Q_SIGNALS:
    void RequestSuccess(QNetworkReply* reply, int code, const QString& errorMsg);
    void RequestError(QNetworkReply* reply, int code, const QString& errorMsg);

protected:
    QNetworkReply* DoProcessReply(QObject* handler, QNetworkReply* reply);

private:
    QNetworkAccessManager m_manager;

	static QByteArrayList s_knownHeaders;
	static QMap<QNetworkRequest::KnownHeaders, QByteArray> s_knownHeaderMap;
};

#endif // CREQUESTMANAGER_H
