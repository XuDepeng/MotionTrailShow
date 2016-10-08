#include <QtCore/QCoreApplication>

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "MotionParser.h"

bool connect(const QString& dbName) {
	QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
	db.setHostName("localhost");
	db.setDatabaseName(dbName);
	db.setUserName("root");
	db.setPassword("root");
	if (!db.open()) {
		qDebug() << "Database Error"
				<< db.lastError().text();
		return false;
	}
	return true;
}

int main(int argc, char* argv[]) {
	QCoreApplication a(argc, argv);

	MotionParser mp;
	mp.parse("../data/motion_xy.csv");

	if (!connect("motiontrail")) {
		return 1;	
	}

	QSqlQuery query;
	if (!query.exec("CREATE TABLE path ("
		"id INTEGER PRIMARY KEY AUTO_INCREMENT,"
		"height INT,"
		"x DECIMAL(18,6),"
		"y DECIMAL(18,6),"
		"state INT)")) {
		qDebug() << query.lastError().text();
		return 1;
	}

	query.finish();
	query.prepare("INSERT INTO path (height, x, y, state) VALUES (?, ?, ?, ?)");

	size_t n_pos = mp.getNPos();

	QVariantList h;
	QVariantList x;
	QVariantList y;
	QVariantList s;

	for (size_t i = 0; i < n_pos; ++i) {
		Pos cur = mp.getPosition(i);
		h.push_back(cur.h);
		x.push_back(cur.x);
		y.push_back(cur.y);
		s.push_back((int)cur.state);
	}
	query.addBindValue(h);
	query.addBindValue(x);
	query.addBindValue(y);
	query.addBindValue(s);

	if (!query.execBatch()) {
		qDebug() << query.lastError().text();
	}
	query.finish();
	return a.exec();
}
