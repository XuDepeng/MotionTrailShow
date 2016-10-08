#include "AddPath.h"

AddPath::AddPath(QWidget* parent):
QDialog(parent){
	ui.setupUi(this);

	connect(ui.confirmBtn, SIGNAL(clicked()), this, SLOT(setDbInfo()));
}

AddPath::~AddPath() {
}

void AddPath::setDbInfo() {
	common::Proj p;
	p.hostname = ui.hostLn->text();
	p.dbname = ui.dbLn->text();
	p.usrname = ui.usrLn->text();
	p.pwd = ui.pwdLn->text();
	p.tablename = ui.tabLn->text();
	p.terrain_path = QString::null;

	emit dbInfo(p);
	close();
}