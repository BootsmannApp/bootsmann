#ifndef CMAINGUI_H
#define CMAINGUI_H

#include <QWidget>
#include <QMainWindow>

#include "CRequestManager.h"


namespace Ui {
	class CMainGUI;
}

class CWorkspaceGUI;


class CMainGUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit CMainGUI(QWidget *parent = nullptr);
    ~CMainGUI();

	QString GetConfigFileName() const;
	void StoreSession();
	void RestoreSession();

public Q_SLOTS:
	void on_actionExit_triggered();
	void on_actionAbout_triggered();
	void on_actionAboutQt_triggered();
	void on_actionDonate_triggered();
	//void on_actionSettings_triggered();

	void on_actionNewRequest_triggered();
	void on_actionSaveRequest_triggered();
	void on_actionLoadRequest_triggered();

	void on_actionSaveWorkspace_triggered();
	void on_actionLoadWorkspace_triggered();
	void on_actionCloseWorkspace_triggered();

private Q_SLOTS:
	void OnQuitApplication();

private:
    Ui::CMainGUI *ui = nullptr;
	CRequestManager m_reqMgr;
	CWorkspaceGUI* m_activeWorkspace = nullptr;
};

#endif // CMAINGUI_H
