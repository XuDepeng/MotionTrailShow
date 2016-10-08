#pragma once
#include <QDialog>
#include <QPointer>
#include <QAction>

#include "ui_confanimate.h"

#include "Common.h"

class ConfAnimate : public QDialog {
	Q_OBJECT

public:
	ConfAnimate(QWidget* parent = 0);
	~ConfAnimate();

public slots:
	void setAnimate();
	void resetAnimate();

	signals:
	void animateInfo(int interval, common::Color c);

private:
	Ui::ConfAnimate ui;
};
