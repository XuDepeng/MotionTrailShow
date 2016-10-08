#include <ConfAnimate.h>

ConfAnimate::ConfAnimate(QWidget* parent):
QDialog(parent){
	ui.setupUi(this);
	
	connect(ui.confirmBtn, SIGNAL(clicked()), this, SLOT(setAnimate()));
	connect(ui.resetBtn, SIGNAL(clicked()), this, SLOT(resetAnimate()));
}

ConfAnimate::~ConfAnimate() {
}

void ConfAnimate::setAnimate() {
	int interval = ui.intervalLn->text().toInt();
	common::Color c;
	c.r = ui.rSpinBox->value();
	c.g = ui.gSpinBox->value();
	c.b = ui.bSpinBox->value();

	emit animateInfo(interval, c);
	close();
}

void ConfAnimate::resetAnimate() {
	ui.intervalLn->setText("500");
	ui.rSpinBox->setValue(51);
	ui.gSpinBox->setValue(51);
	ui.bSpinBox->setValue(153);
}