#ifndef CWORKSPACEGUI_H
#define CWORKSPACEGUI_H

#include <QWidget>
#include <QSettings>

namespace Ui {
class CWorkspaceGUI;
}

class CRequestManager;
class CRequestGUI;

class CWorkspaceGUI : public QWidget
{
    Q_OBJECT

public:
    explicit CWorkspaceGUI(CRequestManager& reqMgr, QWidget *parent = nullptr);
    ~CWorkspaceGUI();

	int CreateNewRequest() { return AddRequestTab(); }
	bool SaveCurrentRequest();
	bool LoadRequest();
	bool HasRequests() const; 

	bool SaveWorkspace();
	bool LoadWorkspace();

    bool Store(QSettings& settings) const;
    bool Restore(QSettings& settings);

protected:
    int AddRequestTab();
    CRequestGUI* GetCurrentRequest();
    CRequestGUI* GetRequest(int tabIndex);

private Q_SLOTS:
    void CloseRequestTab(int index);

private:
    Ui::CWorkspaceGUI *ui;

    CRequestManager& m_reqMgr;

	QString m_lastLoadPath;
	QString m_lastSavePath;
};

#endif // CWORKSPACEGUI_H
