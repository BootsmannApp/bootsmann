#include "CRequestGUI.h"
#include "ui_CRequestGUI.h"

#include "CRequestManager.h"

#include <QMessageBox>
#include <QJsonDocument>
#include <QTemporaryFile>
#include <QImageReader>
#include <QBuffer>
#include <QFileDialog>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>


CRequestGUI::CRequestGUI(CRequestManager& reqMgr, QWidget *parent)
    : QWidget(parent)
    , m_reqMgr(reqMgr)
    , ui(new Ui::CRequestGUI)
{
    ui->setupUi(this);

    ui->splitter->setSizes(QList<int>({ INT_MAX, INT_MAX }));

	SetDefaultHeaders();

    ui->RequestTabs->setCurrentIndex(0);
    ui->RequestBinaryLabel->hide();
    ui->RequestBody->show();

    ui->AuthStack->setCurrentIndex(0); // No authentication by default

	ui->ResponseHeaders->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    m_requestHL = new QSourceHighlite::QSourceHighliter(nullptr);
    m_replyHL = new QSourceHighlite::QSourceHighliter(nullptr);

	m_webView = new QWebEngineView(ui->ReplyStack);
	ui->ReplyStack->addWidget(m_webView);
}


CRequestGUI::~CRequestGUI()
{
    delete ui;

    delete m_requestHL;
    delete m_replyHL;
}


// process requests

void CRequestGUI::Init()
{
    ClearResult();

    ui->RequestURL->setFocus(Qt::OtherFocusReason);
}


bool CRequestGUI::Store(QSettings& settings) const
{
	// Request
    settings.beginGroup("Request");
    settings.setValue("RequestURL", ui->RequestURL->text());
    settings.setValue("RequestType", ui->RequestType->currentText());
    settings.setValue("RequestBody", ui->RequestBody->toPlainText());

	settings.setValue("RequestDataType", ui->RequestDataType->currentIndex());
    settings.setValue("RequestFileNameToUpload", m_fileNameToUpload);
	settings.endGroup();

    // Request headers
    settings.beginGroup("RequestHeaders");
    ui->RequestHeaders->Store(settings);
    settings.endGroup();

	// Request parameters
    settings.beginGroup("RequestParams");
    ui->RequestParams->Store(settings);
	settings.endGroup();
    
	// Autentication settings
	settings.beginGroup("Authentication");
	settings.setValue("AuthType", ui->AuthType->currentIndex());
	settings.setValue("AuthUser", ui->AuthUser->text().trimmed());
	settings.setValue("AuthPassword", ui->AuthPassword->text().trimmed());
    settings.setValue("AuthToken", ui->AuthToken->text().trimmed());
	settings.endGroup();

    // UI
    settings.beginGroup("UI");
	settings.setValue("SplitterState", ui->splitter->saveState());
	settings.endGroup();

    return true;
}


bool CRequestGUI::Restore(QSettings& settings)
{
    ui->RequestURL->blockSignals(true);
    ui->RequestType->blockSignals(true);
    ui->RequestHeaders->blockSignals(true);
    ui->RequestParams->blockSignals(true);
    //ui->AuthType->blockSignals(true);
    ui->AuthPassword->blockSignals(true);
    ui->AuthUser->blockSignals(true);

    // Request
    settings.beginGroup("Request");
	auto requestUrl = settings.value("RequestURL", "").toByteArray();
	QString requestUrlStr = QUrl::fromPercentEncoding(requestUrl);
	ui->RequestURL->setText(requestUrlStr);
	QString verb = settings.value("RequestType", "GET").toString();
	ui->RequestType->setCurrentText(verb);
	ui->RequestBody->setPlainText(settings.value("RequestBody", "").toString());

	ui->RequestDataType->setCurrentIndex(settings.value("RequestDataType", DT_PLAIN).toInt());
	m_fileNameToUpload = settings.value("RequestFileNameToUpload", "").toString();
    settings.endGroup();

    // Request headers
    settings.beginGroup("RequestHeaders");
	ui->RequestHeaders->Restore(settings);
    settings.endGroup();

    // Request parameters
    settings.beginGroup("RequestParams");
    ui->RequestParams->Restore(settings);
    settings.endGroup();

    // Autentication settings
    settings.beginGroup("Authentication");
	int authType = settings.value("AuthType", 0).toInt();
	ui->AuthUser->setText(settings.value("AuthUser", "").toString().trimmed());
	ui->AuthPassword->setText(settings.value("AuthPassword", "").toString().trimmed());
	ui->AuthToken->setText(settings.value("AuthToken", "").toString().trimmed());
    ui->AuthType->setCurrentIndex(authType);
    settings.endGroup();

    // UI
    settings.beginGroup("UI");
	ui->splitter->restoreState(settings.value("SplitterState").toByteArray());
    settings.endGroup();

    // done
    ui->RequestURL->blockSignals(false);
    ui->RequestType->blockSignals(false);
    ui->RequestHeaders->blockSignals(false);
    ui->RequestParams->blockSignals(false);

    //ui->AuthType->blockSignals(false);
    ui->AuthPassword->blockSignals(false);
    ui->AuthUser->blockSignals(false);

    // title
    UpdateTabTitle();

    return true;
}


bool CRequestGUI::IsDefault() const
{
    // Implement logic to determine if the request is default.  
    // For example, check if certain fields are empty or have default values.  
    return ui->RequestURL->text().isEmpty() 
        && ui->RequestParams->rowCount() == 0 
        && ui->RequestBody->toPlainText().isEmpty();
}


void CRequestGUI::on_AddHeader_clicked()
{
	AddRequestHeader("<New Header>", "");
}


void CRequestGUI::on_RemoveHeader_clicked()
{
    auto currentRow = ui->RequestHeaders->currentRow();
    if (currentRow < 0) {
        return; // No row selected, do nothing
	}

    auto asked = QMessageBox::question(this, tr("Remove Header"),
        tr("Are you sure you want to remove the selected header?"), 
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (asked != QMessageBox::Yes) {
        return; // User chose not to remove the header
	}

    // Remove the currently selected row from the RequestHeaders table
    ui->RequestHeaders->DeleteCurrentRow();
}


void CRequestGUI::on_ClearHeaders_clicked()
{
    auto asked = QMessageBox::question(this, tr("Reset Headers"), 
        tr("Are you sure you want to reset all request headers to defaults?"), 
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (asked != QMessageBox::Yes) {
        return; // User chose not to clear headers
    }

	ui->RequestHeaders->setRowCount(0);
    SetDefaultHeaders();
	ui->RequestHeaders->setCurrentCell(0, 0); // Set focus to the first cell
}


void CRequestGUI::on_SaveRequestHeadersContent_clicked()
{
    QString filter = tr("All files (*)");
    filter = tr("Text files (*.txt)") + ";;" + filter;

    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Headers Content"), "", filter);
    if (filePath.isEmpty())
        return; // User canceled the save dialog

    QString content = GetHeadersAsText(ui->RequestHeaders);

    if (!WriteAsText(filePath, content)) {
        //QMessageBox::warning(this, tr("Error"), tr("Could not save headers to file: %1").arg(filePath));
    }
}


void CRequestGUI::on_CopyRequestHeadersContent_clicked()
{
    QString content = GetHeadersAsText(ui->RequestHeaders);

    qApp->clipboard()->setText(content);
}


void CRequestGUI::on_AddParameter_clicked()
{
    ui->RequestParams->blockSignals(true);
	ui->RequestParams->AddRowIfNotExists("<New Parameter>", "");
    ui->RequestParams->blockSignals(false);
}


void CRequestGUI::on_RemoveParameter_clicked()
{
    auto currentRow = ui->RequestParams->currentRow();
    if (currentRow < 0) {
        return; // No row selected, do nothing
    }

    auto asked = QMessageBox::question(this, tr("Remove Parameter"),
        tr("Are you sure you want to remove the selected parameter?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (asked != QMessageBox::Yes) {
        return;
    }

    // Remove the currently selected row
    ui->RequestParams->DeleteCurrentRow();

    // Update URL
    RebuildURL();
}


void CRequestGUI::on_ClearParameters_clicked()
{
    if (ui->RequestParams->rowCount() == 0)
        return;

    auto asked = QMessageBox::question(this, tr("Reset Parameters"),
        tr("Are you sure you want to remove all the parameters?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (asked != QMessageBox::Yes) {
        return; // User chose not to clear headers
    }

    ui->RequestParams->setRowCount(0);

    // Update URL
    RebuildURL();
}


void CRequestGUI::on_AuthType_currentIndexChanged(int index)
{
    if (index == 0) {
        ui->AuthStack->setCurrentIndex(0); // No authentication
    }
    else if (index == 1) {
        ui->AuthStack->setCurrentIndex(1); // Basic authentication
        ui->AuthUser->setFocus(); // Set focus to the login field
    }
    else if (index == 2) {
        ui->AuthStack->setCurrentIndex(2); // Bearer token authentication
		ui->AuthToken->setFocus(); // Set focus to the token field
    }
    else {
        ui->AuthStack->setCurrentIndex(3); // Custom authentication
    }

    RebuildURL();
}


void CRequestGUI::on_RequestURL_editingFinished()
{
	UpdateTabTitle();
        
    // parse URL and update request parameters
    QString request = ui->RequestURL->text().trimmed();
    QUrl sourceUrl = QUrl::fromUserInput(request);

	// check authentication type
	auto userName = sourceUrl.userName();
	auto password = sourceUrl.password();
    if (userName.isEmpty() && password.isEmpty()) {
        if (ui->AuthType->currentIndex() == 1) {
            ui->AuthType->setCurrentIndex(0); // Reset to No authentication
		}
    } else {
        ui->AuthType->setCurrentIndex(1); // Basic authentication
        ui->AuthUser->setText(userName);
        ui->AuthPassword->setText(password);
	}

    // Clear existing active parameters
    ui->RequestParams->DeleteActiveRows();

    // Extract query parameters from the URL
    auto queryItems = sourceUrl.query();
    if (queryItems.isEmpty())
        return;

    ui->RequestParams->setUpdatesEnabled(false);
    ui->RequestParams->blockSignals(true);

	auto queryPairs = queryItems.split('&', Qt::SkipEmptyParts);
    for (const auto& pair : queryPairs) {
        auto keyValue = pair.split('=');
		QString key, value;
        key = QUrl::fromPercentEncoding(keyValue[0].toUtf8());
        if (keyValue.size() == 2) {
            value = QUrl::fromPercentEncoding(keyValue[1].toUtf8());
        }

		// if parameter and value already exist, aktivate it
		int foundRow = ui->RequestParams->FindRow(key, value);
        if (foundRow >= 0) {
            ui->RequestParams->SetActive(foundRow, true);
        } else {
            // Add new parameter if it doesn't exist
            ui->RequestParams->AddRow(key, value, true);
		}
    }

    ui->RequestParams->setUpdatesEnabled(true);
    ui->RequestParams->blockSignals(false);
}


void CRequestGUI::on_RequestType_currentIndexChanged(int /*index*/)
{
    UpdateTabTitle();
}


void CRequestGUI::on_RequestParams_cellChanged(int /*row*/, int /*column*/)
{
    RebuildURL();
}


void CRequestGUI::on_RequestDataType_currentIndexChanged(int index)
{
    ui->RequestBinaryLabel->hide();
    ui->RequestBody->show();

    switch (index)
    {
    case DT_PLAIN:
        m_requestHL->setDocument(nullptr);
        AddRequestHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
	    break;

    case DT_JSON:
		m_requestHL->setDocument(ui->RequestBody->document());
        m_requestHL->setCurrentLanguage(QSourceHighlite::QSourceHighliter::CodeJSON);
        AddRequestHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        break;

    case DT_HTML:
        m_requestHL->setDocument(ui->RequestBody->document());
        m_requestHL->setCurrentLanguage(QSourceHighlite::QSourceHighliter::CodeXML);
        AddRequestHeader(QNetworkRequest::ContentTypeHeader, "text/html");
        break;

    default:
        ui->RequestBinaryLabel->show();
        ui->RequestBody->hide();
        m_requestHL->setDocument(nullptr);
        AddRequestHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
        break;
    }
}


void CRequestGUI::on_LoadRequestBody_clicked()
{
	QString filter = tr("All files (*)");

    switch (ui->RequestDataType->currentIndex())
    {
    case DT_PLAIN:
        filter = tr("Text files (*.txt);;All files (*)");
		break;
    case DT_JSON:
        filter = tr("JSON files (*.json);;All files (*)");
		break;
    case DT_HTML:
		filter = tr("HTML files (*.html);;All files (*)");
    default:
        break;
    }

    QString filePath = QFileDialog::getOpenFileName(this, tr("Data to upload"), "", filter);
    if (filePath.isEmpty())
        return;

    ui->RequestBinaryLabel->setText(tr("Binary data will be uploaded from:/n/n %1").arg(filePath));
	m_fileNameToUpload = filePath;

    if (ui->RequestDataType->currentIndex() >= DT_IMAGE) {
    }
    else {
		QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, tr("Error"), tr("Could not open file: %1").arg(file.errorString()));
            return;
		}
		auto content = file.readAll(); // Read the file content
		ui->RequestBody->setPlainText(QString::fromUtf8(content));
    }
}


void CRequestGUI::on_ResetRequestBody_clicked()
{
    auto asked = QMessageBox::question(this, tr("Reset Request Body"),
        tr("Are you sure you want to reset the request body / data to load?"), 
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (asked != QMessageBox::Yes) {
        return; // User chose not to reset the body
    }

    ui->RequestBody->clear();
	ui->RequestBinaryLabel->clear();
    m_fileNameToUpload.clear();
	m_fileToUpload.close(); // Close the file if it was opened
}


void CRequestGUI::SetDefaultHeaders()
{
    AddRequestHeader(QNetworkRequest::UserAgentHeader, qApp->applicationName() + " " + qApp->applicationVersion());
    AddRequestHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
}


void CRequestGUI::AddRequestHeader(const QString& name, const QString& value)
{
    ui->RequestHeaders->AddRowIfNotExists(name, value);
}


void CRequestGUI::AddRequestHeader(QNetworkRequest::KnownHeaders type, const QString& value)
{
    AddRequestHeader(CRequestManager::GetKnownHeader(type), value);
}


void CRequestGUI::RemoveRequestHeader(const QString& name)
{
    ui->RequestHeaders->DeleteActiveRows(name);
}


void CRequestGUI::RemoveRequestHeader(QNetworkRequest::KnownHeaders type)
{
    RemoveRequestHeader(CRequestManager::GetKnownHeader(type));
}


QNetworkCacheMetaData::RawHeaderList CRequestGUI::GetRequestHeaders() const
{
	auto activeHeaders = ui->RequestHeaders->GetEnabledParams();
    if (activeHeaders.isEmpty()) {
        return {}; // No headers to return
	}

    QNetworkCacheMetaData::RawHeaderList headers;

    for (const auto& header : activeHeaders) {
        headers.append({ header.first.toUtf8(), header.second.toUtf8() });
	}

    return headers;
}


void CRequestGUI::RebuildURL()
{
	ui->RequestURL->blockSignals(true); // Block signals to prevent infinite loop

    QUrl sourceUrl(ui->RequestURL->text());

    QString scheme = sourceUrl.scheme();
    QString user = sourceUrl.userInfo();
    QString password = sourceUrl.password();
    QString host = sourceUrl.host();
	QString query = sourceUrl.query();
	QString port = QString::number(sourceUrl.port(-1)); // -1 means use default port for the scheme
	QString fragment = sourceUrl.fragment();

    // cut parameters from the URL
    QUrl targetUrl(sourceUrl.adjusted(QUrl::RemoveQuery | QUrl::RemoveUserInfo));
	
	auto params = ui->RequestParams->GetEnabledParams();
    if (!params.isEmpty()) {
        QStringList queryParts;
        for (const auto& param : params) {
            queryParts.append(QString("%1=%2").arg(param.first, param.second));
        }
        targetUrl.setQuery(queryParts.join("&")); // Set new query parameters
	}
    else
		targetUrl.setQuery(""); // Clear query if no parameters

    // update login:password
    if (ui->AuthType->currentIndex() == 0) { // No authentication
        targetUrl.setUserInfo(""); // Clear user info
    } 
    else if (ui->AuthType->currentIndex() == 1) { // Basic authentication
        QString login = ui->AuthUser->text().trimmed();
        QString password = ui->AuthPassword->text().trimmed();

        if (login.isEmpty() && password.isEmpty()) {
            targetUrl.setUserInfo(""); // Clear user info
        } else {
            targetUrl.setUserName(login);
            targetUrl.setPassword(password);
		}
    }
    else if (ui->AuthType->currentIndex() == 2) { // Bearer token authentication
        targetUrl.setUserInfo(""); // Clear user info

        QString token = ui->AuthToken->text().trimmed();
        if (!token.isEmpty()) {
			ui->RequestHeaders->AddRowIfNotExists("Authorization", "Bearer " + token);
        } else {
			ui->RequestHeaders->DeleteActiveRows("Authorization");
        }
    }

	qDebug() << targetUrl.toString(QUrl::PrettyDecoded);

	ui->RequestURL->setText(targetUrl.toString(QUrl::PrettyDecoded));

	ui->RequestURL->blockSignals(false); // Unblock signals

    // request title
    UpdateTabTitle();
}


void CRequestGUI::UpdateTabTitle()
{
    QString verb = ui->RequestType->currentText();
    QString requestTitle = verb + " " + ui->RequestURL->text().trimmed();
    Q_EMIT RequestTitleChanged(requestTitle);
}


void CRequestGUI::on_Run_clicked()
{
    ClearResult();

    QString request = ui->RequestURL->text().trimmed();
    QString verb = ui->RequestType->currentText();

    if (request.isEmpty()){
        ui->ResultCode->setText(tr("ERROR"));
        ShowPlainText(tr("Request is empty"), false);
        return;
    }

	QNetworkReply* reply = nullptr;

    if (ui->RequestDataType->currentIndex() >= DT_IMAGE) {
        // If the request is an image or binary data, we need to upload a file
        if (m_fileNameToUpload.isEmpty()) {
            QMessageBox::warning(this, tr("No File Selected"), tr("Please select a file to upload."));
            return;
        }

        m_fileToUpload.setFileName(m_fileNameToUpload);
        if (!m_fileToUpload.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, tr("Error"), tr("Could not open file: %1").arg(m_fileToUpload.errorString()));
            return;
        }

        reply = m_reqMgr.UploadRequest(this, verb.toLocal8Bit(), QUrl(request), m_fileToUpload, GetRequestHeaders());
	}

    if (!reply) {
		auto payload = ui->RequestBody->toPlainText().toUtf8();
        reply = m_reqMgr.SendRequest(this, verb.toLocal8Bit(), QUrl(request), payload, GetRequestHeaders());
    }

    LockRequest();

    m_timer.start();

    if (reply == nullptr) {
        UnlockRequest();
        ui->ResultCode->setText(tr("ERROR"));
        ShowPlainText(tr("Request could not be processed"), false);
        return;
    }

    ui->ResultCode->setText(tr("IN PROGRESS..."));
}


// process responses

void CRequestGUI::OnRequestDone()
{
    auto ms = m_timer.elapsed();
    auto reply = qobject_cast<QNetworkReply*>(sender());

    // update response time and size
    ui->ResponseSizeLabel->setText(tr("%1 bytes").arg(reply->bytesAvailable()));
    ui->TimeLabel->setText(tr("%1 ms").arg(ms));

    reply->deleteLater(); // delete reply object after processing

    UnlockRequest();

    ui->ReplyDataType->show();
    ui->ReplyDataInfo->show();

    // update headers
    const auto& headers = reply->rawHeaderPairs();
    ui->ResponseHeaders->setRowCount(headers.size());

    int r = 0;
    for (const auto& header : headers) {
        auto& key = header.first;
        auto& value = header.second;
        ui->ResponseHeaders->setItem(r, 0, new QTableWidgetItem(key));
        ui->ResponseHeaders->setItem(r, 1, new QTableWidgetItem(value));
        r++;
    }

    // update body
    m_replyData = reply->readAll();
    DecodeReply(reply, m_replyData);

	ui->SaveReplyContent->setEnabled(m_replyData.size());
	ui->CopyReplyContent->setEnabled(m_replyData.size());
    ui->SaveHeadersContent->setEnabled(m_replyData.size());
    ui->CopyHeadersContent->setEnabled(m_replyData.size());

    // update status
    auto errorCode = reply->error();
	auto errorString = reply->errorString();
    auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    auto statusReason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();

    // if error happened, this will be handled in another slot
    QString statusText;
    if (reply->error() == QNetworkReply::NoError)
    {
        statusText = tr("DONE: %1").arg(statusCode);
        if (statusCode == 200) {
			// HTTP 200 OK
            errorString = ""; // Clear status reason for 200 OK
        }

        Q_EMIT RequestDone();
    }
    else
    {
        statusText = tr("FAILED: %1").arg(errorCode);

        Q_EMIT RequestFailed();
    }

    if (!statusReason.isEmpty()) {
        statusText += " [" + statusReason + "]";
    }
    ui->ResultCode->setText(statusText);

    ui->ServerErrorText->setText(errorString);
    ui->ServerErrorText->setVisible(!errorString.isEmpty());
}


void CRequestGUI::OnRequestError(QNetworkReply::NetworkError code)
{
    auto reply = qobject_cast<QNetworkReply*>(sender());
    auto errorText = reply->errorString();

    Q_EMIT RequestFailed();
}


void CRequestGUI::LockRequest()
{
    ui->Run->setEnabled(false);
    ui->RequestURL->setEnabled(false);
    ui->RequestType->setEnabled(false);
    ui->RequestBody->setEnabled(false);
    ui->RequestHeaders->setEnabled(false);
    ui->AddHeader->setEnabled(false);
    ui->RemoveHeader->setEnabled(false);
    ui->ClearHeaders->setEnabled(false);
    ui->ReplyDataType->setEnabled(false);
    ui->SaveReplyContent->setEnabled(false);
    ui->CopyReplyContent->setEnabled(false);

    Q_EMIT RequestStarted();
}


void CRequestGUI::UnlockRequest()
{
    ui->Run->setEnabled(true);
    ui->RequestURL->setEnabled(true);
    ui->RequestType->setEnabled(true);
    ui->RequestBody->setEnabled(true);
    ui->RequestHeaders->setEnabled(true);
    ui->AddHeader->setEnabled(true);
    ui->RemoveHeader->setEnabled(true);
    ui->ClearHeaders->setEnabled(true);
    ui->ReplyDataType->setEnabled(true);
    ui->SaveReplyContent->setEnabled(true);
    ui->CopyReplyContent->setEnabled(true);
	ui->SaveHeadersContent->setEnabled(true);
    ui->CopyHeadersContent->setEnabled(true);
}


void CRequestGUI::ClearResult()
{
    ui->ResponsePreview->clear();
    ui->ResponseText->clear();
    ui->ResponseHeaders->setRowCount(0);
    ui->ResponseHeaders->setColumnCount(2);
    ui->ResponseHeaders->setHorizontalHeaderLabels({ tr("Name"), tr("Value") });
    ui->TimeLabel->clear();
    ui->ResponseSizeLabel->clear();
    ui->ResultTabs->setCurrentIndex(0);
    ui->ReplyStack->setCurrentIndex(2);
    m_replyHL->setDocument(nullptr);
    ui->ServerErrorText->hide();
    ui->ReplyDataInfo->clear();
	ui->SaveReplyContent->setEnabled(false);
	ui->CopyReplyContent->setEnabled(false);
	ui->SaveHeadersContent->setEnabled(false);
	ui->CopyHeadersContent->setEnabled(false);
    ui->HtmlType->hide();

    Q_EMIT RequestCleared();
}


void CRequestGUI::DecodeReply(QNetworkReply* reply, const QByteArray& data)
{
    // Check the content type of the reply  
    auto contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();

    if (contentType.isEmpty() && data.isEmpty()) {
        ShowReplyContent(DT_PLAIN, data, contentType);
        return;
    }

    // images
    if (contentType.contains("image/", Qt::CaseInsensitive)) {
        bool ok = ShowReplyContent(DT_IMAGE, data, contentType);
    }
    // text, JSON, HTML...
    else if (contentType.contains("application/json", Qt::CaseInsensitive)) {
        ShowReplyContent(DT_JSON, data, contentType);
    }
    else if (contentType.contains("text/html", Qt::CaseInsensitive)) {
        ShowReplyContent(DT_HTML, data, contentType);
    }
    else if (contentType.contains("text/plain", Qt::CaseInsensitive)) {
        ShowReplyContent(DT_PLAIN, data, contentType);
    }
    else {
        // For other content types, display as binary data
        ShowReplyContent(DT_HEX, data, contentType);
    }
}


void CRequestGUI::ShowPlainText(const QString& text, bool append)
{
	ui->ReplyDataInfo->clear();

    if (append)
        ui->ResponseText->appendPlainText(text);
    else
        ui->ResponseText->setPlainText(text);
    
    ui->ReplyStack->setCurrentIndex(2);
}


void CRequestGUI::UpdateHtmlReply()
{
    if (ui->HtmlAsTextRB->isChecked()) {
        ui->ReplyStack->setCurrentIndex(2); // Show as plain text
    }
    else {
        ui->ReplyStack->setCurrentIndex(3); // Show as HTML preview
    }
}


bool CRequestGUI::WriteAsText(const QString& filePath, const QString& content)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QMessageBox::warning(nullptr, tr("Error"), tr("Could not open file for writing: %1").arg(file.errorString()));
        return false;
    }

	QTextStream out(&file);
	out << content;

    return true;
}


bool CRequestGUI::WriteAsBin(const QString& filePath, const QByteArray& content)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(nullptr, tr("Error"), tr("Could not open file for writing: %1").arg(file.errorString()));
        return false;
    }

	file.write(content);

    return true;
}


bool CRequestGUI::WriteAsHex(const QString& fileName, const QByteArray& content)
{
	QString txt = content.toHex(' ').toUpper();
	return WriteAsText(fileName, txt);
}


bool CRequestGUI::WriteAsImage(const QString& fileName, const QByteArray& content)
{
	QImage image;
    if (!image.loadFromData(content)) {
        QMessageBox::warning(nullptr, tr("Error"), tr("Could not write image content"));
        return false;
    }

    if (!image.save(fileName)) {
        QMessageBox::warning(nullptr, tr("Error"), tr("Could not save image to file: %1").arg(fileName));
        return false;
	}

    return true;
}


bool CRequestGUI::ShowReplyContent(ReplyDisplayType showType, const QByteArray& data, const QString& contentType)
{
    int cbIndex = (int)showType;
    ui->ReplyDataType->setCurrentIndex(cbIndex);

	ui->HtmlType->setVisible(showType == DT_HTML);

    switch (showType)
    {
    case DT_IMAGE:
    {
        ui->ReplyStack->setCurrentIndex(0);

        QImage pm;
        if (pm.loadFromData(data))
        {
            QBuffer buffer(&m_replyData);
            buffer.open(QIODevice::ReadOnly);
            QImageReader reader(&buffer);
            QString format = reader.format().toUpper();

            ui->ResponsePreview->setUpdatesEnabled(false);
            ui->ResponsePreview->document()->addResource(QTextDocument::ImageResource, QUrl("image://pm"), pm);
            ui->ResponsePreview->setHtml(QString("<img src='image://pm'/>"));
			ui->ResponsePreview->setUpdatesEnabled(true);

			ui->ReplyDataInfo->setText(QString("   %3   %1x%2").arg(pm.width()).arg(pm.height()).arg(format));
        }
        else
        {
			ui->ReplyDataInfo->clear();
            ui->ResponsePreview->setHtml(tr("<font color=red>Cannot decode image format</font>"));
            return false;
        }
    }
    break;

    case DT_HEX:
    {
        if (!m_hexView) {
            m_hexView = new QHexView(ui->ReplyStack);
            ui->ReplyStack->addWidget(m_hexView);
        }

        ui->ReplyStack->setCurrentWidget(m_hexView);

        m_hexView->setData(new QHexView::DataStorageArray(m_replyData));
    }
        break;

    case DT_HTML:
        m_replyHL->setDocument(ui->ResponseText->document());
        m_replyHL->setCurrentLanguage(QSourceHighlite::QSourceHighliter::CodeXML);

        {
            QString htmlContent = QString::fromUtf8(data);
            ShowPlainText(htmlContent, false);

			//m_webView->setUrl(QUrl("data:text/html;charset=utf-8," + QUrl::toPercentEncoding(htmlContent)));
            m_webView->setHtml(htmlContent, ui->RequestURL->text());

			//ui->ResponsePreview->setUpdatesEnabled(false);
   //         ui->ResponsePreview->setHtml(htmlContent);
			//ui->ResponsePreview->setUpdatesEnabled(true);
        }

		UpdateHtmlReply();
        break;

    case DT_JSON:
        m_replyHL->setDocument(ui->ResponseText->document());
        m_replyHL->setCurrentLanguage(QSourceHighlite::QSourceHighliter::CodeJSON);
        ShowPlainText(QString::fromUtf8(data), false);
        break;

    default:    // plain text
        m_replyHL->setDocument(nullptr);
        ShowPlainText(QString::fromUtf8(data), false);
        break;
    }

    return true;
}


void CRequestGUI::on_ReplyDataType_currentIndexChanged(int index)
{
	ShowReplyContent((ReplyDisplayType)index, m_replyData);
}


void CRequestGUI::on_SaveReplyContent_clicked()
{
	QString filter = tr("All files (*)");

	ReplyDisplayType replyTypeChosen = (ReplyDisplayType)ui->ReplyDataType->currentIndex();

    switch (replyTypeChosen)
    {   
    case CRequestGUI::DT_PLAIN:
		filter = tr("Text files (*.txt)") + ";;" + filter;
        break;
    case CRequestGUI::DT_HTML:
		filter = tr("HTML files (*.html)") + ";;" + filter;
        break;
    case CRequestGUI::DT_JSON:
		filter = tr("JSON files (*.json)") + ";;" + filter;
        break;
    case CRequestGUI::DT_IMAGE:
        filter =
            tr("Windows Bitmap (*.bmp)") + ";;" +
            tr("Portable Network Graphics (*.png)") + ";;" +
            tr("Joint Photographic Experts Group (*.jpg *.jpeg)") + ";;" +
            tr("Portable Pixmap (*.ppm)") + ";;" + 
            tr("X11 Pixmap (*.xpm)") + ";;" + 
            filter;
        break;
    case CRequestGUI::DT_HEX:
		filter = tr("Hex text files (*.hex);;Binary files (*.bin)") + ";;" + filter;
        break;
    default:
		return; // Unsupported type
    }

	QString selectedFilter;
	QString filePath = QFileDialog::getSaveFileName(this, tr("Save Reply Content"), "", filter, &selectedFilter);
    if (filePath.isEmpty())
		return; // User canceled the save dialog

    switch (replyTypeChosen)
    {
    case CRequestGUI::DT_PLAIN:
    case CRequestGUI::DT_HTML:
    case CRequestGUI::DT_JSON:
		WriteAsText(filePath, ui->ResponseText->toPlainText());
        break;
    case CRequestGUI::DT_IMAGE:
        WriteAsImage(filePath, m_replyData);
        break;
    case CRequestGUI::DT_HEX:
        // Save as hex or binary
        if (filePath.toLower().endsWith(".hex"))
            WriteAsHex(filePath, m_replyData);
        else
			WriteAsBin(filePath, m_replyData);
        break;
    default:
        break;
    }
}


void CRequestGUI::on_CopyReplyContent_clicked()
{
    ReplyDisplayType replyTypeChosen = (ReplyDisplayType)ui->ReplyDataType->currentIndex();

    switch (replyTypeChosen)
    {
    case CRequestGUI::DT_PLAIN:
    case CRequestGUI::DT_HTML:
    case CRequestGUI::DT_JSON:
		qApp->clipboard()->setText(ui->ResponseText->toPlainText());
        break;
    case CRequestGUI::DT_IMAGE:
		qApp->clipboard()->setImage(QImage::fromData(m_replyData));
        break;
    case CRequestGUI::DT_HEX:
    {
		auto mimeData = new QMimeData();
		mimeData->setData("application/octet-stream", m_replyData);
		qApp->clipboard()->setMimeData(mimeData);
    }
        break;
    default:
        break;
    }
}


QString CRequestGUI::GetHeadersAsText(QTableWidget* table)
{
    QString content;
    for (int i = 0; i < table->rowCount(); ++i) {
        auto nameItem = table->item(i, 0);
        auto valueItem = table->item(i, 1);
        if (nameItem && valueItem) {
            content += QString("%1: %2\n").arg(nameItem->text(), valueItem->text());
        }
    }
    return content;
}


void CRequestGUI::on_SaveHeadersContent_clicked()
{
    QString filter = tr("All files (*)");
    filter = tr("Text files (*.txt)") + ";;" + filter;

    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Headers Content"), "", filter);
    if (filePath.isEmpty())
        return; // User canceled the save dialog

	QString content = GetHeadersAsText(ui->ResponseHeaders);

    if (!WriteAsText(filePath, content)) {
        //QMessageBox::warning(this, tr("Error"), tr("Could not save headers to file: %1").arg(filePath));
	}
}


void CRequestGUI::on_CopyHeadersContent_clicked()
{
    QString content = GetHeadersAsText(ui->ResponseHeaders);

    qApp->clipboard()->setText(content);
}
