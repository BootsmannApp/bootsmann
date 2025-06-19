#ifndef CMAINGUI_H
#define CMAINGUI_H

#include <QWidget>
#include <QMainWindow>

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

public Q_SLOTS:
	void on_actionExit_triggered();
	void on_actionAbout_triggered();
	void on_actionAboutQt_triggered();
	void on_actionDonate_triggered();
	//void on_actionSettings_triggered();
	void on_actionNewRequest_triggered();
	void on_actionSaveRequest_triggered();
	void on_actionLoadRequest_triggered();

private:
    Ui::CMainGUI *ui;
	CWorkspaceGUI* m_activeWorkspace;
};

#endif // CMAINGUI_H
