#include "CReportGUI.h"
#include "ui_CReportGUI.h"

#include "CRequestGUI.h"


CReportGUI::CReportGUI(QWidget* parent): QWidget(parent),
	ui(new Ui::CReportGUI)
{
	ui->setupUi(this);
}


void CReportGUI::SetRequestProvider(CRequestGUI* requestProvider) 
{ 
	m_requestProvider = requestProvider; 

	connect(requestProvider, &CRequestGUI::RequestFailed, this, &CReportGUI::OnUpdateReport);
	connect(requestProvider, &CRequestGUI::RequestDone, this, &CReportGUI::OnUpdateReport);
}


void CReportGUI::UpdateReport()
{
	if (!m_requestProvider) {
		ui->ReportContent->setPlainText(tr("Error: Report provider is not defined."));
		return;
	}

	// request
	QString reportContent = ">> " + m_requestProvider->GetVerb() + "\n" + m_requestProvider->GetRequestURL() + "\n";

	// body
	auto body = m_requestProvider->GetRequestBody();
	reportContent += "\n" + tr(">> BODY [%1 bytes]").arg(body.first) + "\n";
	if (body.second.size())
		reportContent += body.second + "\n";

	// headers
	auto headers = m_requestProvider->GetRequestHeaders();
	reportContent += "\n" + tr(">> HEADERS [%1 total]").arg(headers.size()) + "\n";
	for (const auto& header : headers) {
		reportContent += QString("%1: %2\n").arg(header.first, header.second);
	}

	// authorization
	auto auth = m_requestProvider->GetAuthorization();
	if (!auth.isEmpty()) {
		reportContent += "\n" + tr(">> AUTHORIZATION [%1]").arg(auth.first().first) + "\n";
		for (int i = 1; i < auth.size(); ++i) {
			reportContent += QString("%1: %2\n").arg(auth[i].first, auth[i].second);
		}
	} else {
		reportContent += "\n" + tr(">> AUTHORIZATION: None") + "\n";
	}

	// reply info
	ReplyInfo info = m_requestProvider->GetReplyInfo();
	if (info.statusCode == -1) {
		reportContent += "\n" + tr("<< REQUEST WAS NOT ISSUED") + "\n";
		reportContent += info.statusText + "\n";
		reportContent += tr("Timestamp: %1").arg(info.requestStarted.toString(Qt::ISODate)) + "\n";
		ui->ReportContent->setPlainText(reportContent);
		return;
	}

	if (info.statusCode == 200) {
		reportContent += "\n" + tr("<< REQUEST WAS SUCCESSFUL") + "\n";
	} else {
		reportContent += "\n" + tr("<< REQUEST FAILED") + "\n";
	}

	reportContent += tr("Status code: %1").arg(info.statusCode) + "\n";
	reportContent += tr("Status text: %1").arg(info.statusText) + "\n";
	reportContent += tr("Reply size: %1 bytes").arg(info.replySize) + "\n";
	reportContent += tr("Duration: %1 ms").arg(info.elapsedTime) + "\n";
	reportContent += tr("Timestamp: %1").arg(info.requestStarted.toString(Qt::ISODate)) + "\n";

	ui->ReportContent->setPlainText(reportContent);
}

