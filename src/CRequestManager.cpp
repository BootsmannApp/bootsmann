#include "CRequestManager.h"


QByteArrayList CRequestManager::s_knownHeaders;
QMap<QNetworkRequest::KnownHeaders, QByteArray> CRequestManager::s_knownHeaderMap;


CRequestManager::CRequestManager(QObject *parent)
    : QObject{parent}
{
	// initialize known headers
    if (s_knownHeaders.isEmpty()) {
        for (int i = 0; i < QNetworkRequest::NumKnownHeaders; ++i) {
            auto header = static_cast<QNetworkRequest::KnownHeaders>(i);
            QNetworkRequest tempRequest;
            tempRequest.setHeader(header, "true");
            if (tempRequest.header(header).isNull()) {
                continue; // skip headers that are not set
			}
            s_knownHeaders.append(tempRequest.rawHeaderList().first());
			s_knownHeaderMap.insert(header, s_knownHeaders.last());
		}
	}
}


QNetworkReply* CRequestManager::SendRequest(QObject* handler, const QByteArray& verb, const QUrl& url, const QByteArray& payload,
    QNetworkCacheMetaData::RawHeaderList headers)
{
    QNetworkRequest request;
    request.setUrl(url);

    for (const auto& header : headers) {
        request.setRawHeader(header.first, header.second);
	}

    if (verb == "GET"){
        auto reply = m_manager.get(request);
        return DoProcessReply(handler, reply);
    }

    if (verb == "POST"){
        auto reply = m_manager.post(request, payload);
        return DoProcessReply(handler, reply);
    }

    if (verb == "PUT"){
        auto reply = m_manager.put(request, payload);
        return DoProcessReply(handler, reply);
    }

    if (verb == "DELETE"){
        auto reply = m_manager.deleteResource(request);
        return DoProcessReply(handler, reply);
    }

    if (verb == "HEAD"){
        auto reply = m_manager.head(request);
        return DoProcessReply(handler, reply);
    }

    // verb unknown, custom request instead
    auto reply = m_manager.sendCustomRequest(request, verb, payload);
    return DoProcessReply(handler, reply);
}


QNetworkReply* CRequestManager::UploadRequest(
    QObject* handler,
    const QByteArray& verb,
    const QUrl& url,
    QIODevice& dataSource,
    QNetworkCacheMetaData::RawHeaderList headers)
{
    QNetworkRequest request;
    request.setUrl(url);

    for (const auto& header : headers) {
        request.setRawHeader(header.first, header.second);
    }

    if (verb == "GET") {
        auto reply = m_manager.get(request);
        return DoProcessReply(handler, reply);
    }

    if (verb == "POST") {
        auto reply = m_manager.post(request, &dataSource);
        return DoProcessReply(handler, reply);
    }

    if (verb == "PUT") {
        auto reply = m_manager.put(request, &dataSource);
        return DoProcessReply(handler, reply);
    }

    if (verb == "DELETE") {
        auto reply = m_manager.deleteResource(request);
        return DoProcessReply(handler, reply);
    }

    if (verb == "HEAD") {
        auto reply = m_manager.head(request);
        return DoProcessReply(handler, reply);
    }

    // verb unknown, custom request instead
    auto reply = m_manager.sendCustomRequest(request, verb, &dataSource);
    return DoProcessReply(handler, reply);
}


const QByteArray CRequestManager::GetKnownHeader(QNetworkRequest::KnownHeaders type)
{
	return s_knownHeaderMap.value(type, QByteArray());
}


QNetworkReply* CRequestManager::DoProcessReply(QObject* handler, QNetworkReply* reply)
{
    if (!handler || !reply){
        return nullptr;
    }

    connect(reply, SIGNAL(finished()), handler, SLOT(OnRequestDone()));
    connect(reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), handler, SLOT(OnRequestError(QNetworkReply::NetworkError)));

    // temp reply ID
    return reply;
}

