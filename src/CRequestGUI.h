#ifndef CREQUESTGUI_H
#define CREQUESTGUI_H

#include <QWidget>
#include <QNetworkReply>
#include <QNetworkCacheMetaData>
#include <QElapsedTimer>
#include <QSettings>
#include <QWebEngineView>

#include <QHexView.h>
#include <QSourceHighliter.h>


namespace Ui {
class CRequestGUI;
}

class CRequestManager;

class QTableWidget;

class CRequestGUI : public QWidget
{
    Q_OBJECT

public:
    explicit CRequestGUI(CRequestManager& reqMgr, QWidget *parent = nullptr);
    ~CRequestGUI();

    void Init();
    bool IsDefault() const;

    QString GetRequestURL(bool stripped = false) const;
	bool RebaseURL(const QString& newUrl);

	QString StoreToString() const;

    // IO
	bool Store(QSettings& settings) const;
    bool Restore(QSettings& settings);

Q_SIGNALS:
    void RequestTitleChanged(const QString& title);
    void RequestFailed();
    void RequestDone();
    void RequestCleared();
    void RequestStarted();

public Q_SLOTS:
    void OnRequestDone();
	void OnRequestError(QNetworkReply::NetworkError code);

private Q_SLOTS:
    void on_Run_clicked();

	void on_AddHeader_clicked();
    void on_RemoveHeader_clicked();
	void on_ClearHeaders_clicked();
    void on_SaveRequestHeadersContent_clicked();
    void on_CopyRequestHeadersContent_clicked();

	void on_AddParameter_clicked();
	void on_RemoveParameter_clicked();
	void on_ClearParameters_clicked();

    void on_AuthType_currentIndexChanged(int index);
    void on_AuthUser_editingFinished()      { RebuildURL(); }
    void on_AuthPassword_editingFinished()  { RebuildURL(); }
	void on_AuthToken_editingFinished()     { RebuildURL(); }

    void on_RequestURL_editingFinished();
    void on_RequestType_currentIndexChanged(int index);
	void on_RequestParams_cellChanged(int row, int column);
	void on_RequestDataType_currentIndexChanged(int index);
	void on_LoadRequestBody_clicked();
	void on_ResetRequestBody_clicked();

	void on_ReplyDataType_currentIndexChanged(int index);
    void on_HtmlAsTextRB_toggled(bool) { UpdateHtmlReply(); }
	void on_SaveReplyContent_clicked();
	void on_CopyReplyContent_clicked();
    void on_SaveHeadersContent_clicked();
    void on_CopyHeadersContent_clicked();

private:
    void SetDefaultHeaders();
	void SetRequestHeader(const QString& name, const QString& value);
	void SetRequestHeader(QNetworkRequest::KnownHeaders type, const QString& value);
	void RemoveRequestHeader(const QString& name);
	void RemoveRequestHeader(QNetworkRequest::KnownHeaders type);
    QNetworkCacheMetaData::RawHeaderList GetRequestHeaders() const;

    void LockRequest();
	void UnlockRequest();
	void ClearResult();

    void RebuildURL();
    void UpdateTabTitle();
    void UpdateRequestType();

	void DecodeReply(QNetworkReply* reply, const QByteArray& data);

    enum ReplyDisplayType {
        DT_PLAIN,
        DT_HTML,
        DT_JSON,
        DT_IMAGE,
        DT_HEX
    };
    bool ShowReplyContent(ReplyDisplayType showType, const QByteArray& data, const QString& contentType = "");
    void ShowPlainText(const QString& text, bool append);
    void UpdateHtmlReply();

	static bool WriteAsText(const QString& fileName, const QString& content);
	static bool WriteAsBin(const QString& fileName, const QByteArray& content);
	static bool WriteAsHex(const QString& fileName, const QByteArray& content);
	static bool WriteAsImage(const QString& fileName, const QByteArray& content);

	static QString GetHeadersAsText(QTableWidget* table);

    Ui::CRequestGUI *ui;

    CRequestManager& m_reqMgr;
	QElapsedTimer m_timer;

	QByteArray m_replyData;
    QHexView *m_hexView = nullptr;
	QWebEngineView* m_webView = nullptr;

    QSourceHighlite::QSourceHighliter* m_requestHL = nullptr;
    QSourceHighlite::QSourceHighliter* m_replyHL = nullptr;

	QString m_fileNameToUpload;
	QFile m_fileToUpload;

	QString m_cachedHtml, m_cachedUrl;
};

#endif // CREQUESTGUI_H
