#include "ConfPerson.h"

ConfPerson::ConfPerson(QWidget* parent):
QDialog(parent){
	ui.setupUi(this);

	connect(ui.confirmBtn, SIGNAL(clicked()), this, SLOT(setPerson()));
	connect(ui.resetBtn, SIGNAL(clicked()), this, SLOT(resetPerson()));
}

ConfPerson::~ConfPerson() {
}

void ConfPerson::setPerson() {
	common::Person p;
	p.name = ui.nameLn->text();
	p.age = ui.ageSpinBox->value();
	p.height = ui.heightLn->text().toInt();
	p.weight = ui.weightLn->text().toInt();

	emit personInfo(p);
	close();
}

void ConfPerson::resetPerson() {
	ui.nameLn->setText("David");
	ui.ageSpinBox->setValue(25);
	ui.heightLn->setText("180");
	ui.weightLn->setText("75");
}
