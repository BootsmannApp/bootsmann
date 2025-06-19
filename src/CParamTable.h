#pragma once

#include <QTableWidget>
#include <QList>
#include <QPair>
#include <QSettings>

class CParamTable : public QTableWidget
{
	//Q_OBJECT

public:
	CParamTable(QWidget* parent = nullptr);

	int AddRow(const QString& name = "", const QString& value = "", bool on = true);
	int AddRowIfNotExists(const QString& name = "", const QString& value = "", bool on = true);
	bool DeleteRow(int row);
	bool DeleteCurrentRow();
	int DeleteActiveRows();
	int DeleteInactiveRows();
	int DeleteActiveRows(const QString& name);

	bool IsActive(int row) const;
	bool SetActive(int row, bool on);
	int FindRow(const QString& name, const QString& value) const;

	typedef QList<QPair<QString, QString>> ParamList;
	ParamList GetEnabledParams() const;

	// IO
	void Store(QSettings& settings) const;
	void Restore(QSettings& settings);
};

