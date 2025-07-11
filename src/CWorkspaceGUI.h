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
    int CloneCurrentRequest();
    bool SaveCurrentRequest();

    bool RebaseCurrentRequest();
    bool RebaseOpenRequests();

	bool LoadRequest(); // to do: return index
	bool HasRequests() const; 

    bool BookmarkCurrentRequest();

    static QString GetDefaultWorkspaceFileName();

	bool SaveWorkspace();
	bool LoadWorkspace();
	void StoreSession();    // store current workspace to last used path

	const QString& GetName() const { return m_name; }
	void SetName(const QString& name) { m_name = name; }

	bool IsDefault() const { return m_name.isEmpty() && m_filePath.isEmpty(); }

    bool Store(QSettings& settings) const;
    bool Restore(QSettings& settings);

protected:
    int AddRequestTab(int tabIndex = -1);
    CRequestGUI* GetCurrentRequest();
    CRequestGUI* GetRequest(int tabIndex);

	bool AddBookmark(const QString& name, const CRequestGUI& request);

private Q_SLOTS:
    void CloseRequestTab(int index);

	void on_Tabs_customContextMenuRequested(const QPoint& pos);

private:
    Ui::CWorkspaceGUI *ui;

    CRequestManager& m_reqMgr;

	QString m_lastLoadPath;
	QString m_lastSavePath;

    QString m_filePath;

    QString m_name;
};

#endif // CWORKSPACEGUI_H
