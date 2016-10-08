#pragma once
#include <QDialog>
#include <QPointer>
#include <QAction>

#include "ui_addpath.h"

#include "Common.h"

class AddPath : public QDialog {
	Q_OBJECT

public:
	AddPath(QWidget* parent = 0);
	~AddPath();

private slots:
	void setDbInfo();

signals:
	void dbInfo(common::Proj p);

private:
	Ui::AddPath ui;
};