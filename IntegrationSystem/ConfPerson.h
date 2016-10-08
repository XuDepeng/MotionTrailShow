#pragma once
#include <QDialog>
#include <QPointer>
#include <QAction>

#include "ui_confperson.h"

#include "Common.h"

class ConfPerson : public QDialog {
	Q_OBJECT

public:
	ConfPerson(QWidget* parent = 0);
	~ConfPerson();

public slots:
	void setPerson();
	void resetPerson();

signals:
	void personInfo(common::Person p);

private:
	Ui::ConfPerson ui;
};