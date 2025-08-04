#pragma once

#include <QWidget>

namespace Ui {
    class CReportGUI;
}

class CRequestGUI; // Forward declaration to avoid circular dependency


class CReportGUI : public QWidget
{
    Q_OBJECT

public:
    explicit CReportGUI(QWidget *parent = nullptr);
    //~CReportGUI();

    void SetRequestProvider(CRequestGUI* requestProvider);

public Q_SLOTS:
	void OnUpdateReport() { UpdateReport(); }

	void on_RegenerateButton_clicked() { UpdateReport(); }

private:
    void UpdateReport();

    Ui::CReportGUI *ui;

	CRequestGUI* m_requestProvider = nullptr; // Pointer to the request provider
};

