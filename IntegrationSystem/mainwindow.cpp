#include "MainWindow.h"

#include "ViewerWidget.h"
#include "ConfPerson.h"
#include "ConfAnimate.h"
#include "AddPath.h"

#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent) {
	ui.setupUi(this);

	setWindowTitle(QStringLiteral("运动轨迹展示系统"));
	setAcceptDrops(false);

	initSkin("../style/style.qss");
	initBar();
}

MainWindow::~MainWindow() {
}

void MainWindow::initSkin(const QString& filename) {
	QFile f(filename);
	if (!f.exists()) {
		QMessageBox::warning(this,
							 QStringLiteral("警告"),
							 QStringLiteral("皮肤初始化失败！"));
	}
	else {
		f.open(QFile::ReadOnly | QFile::Text);
		QTextStream ts(&f);
		setStyleSheet(ts.readAll());
	}
}

void MainWindow::initBar() {
	QPointer<QMenu> file = ui.menuBar->addMenu(QStringLiteral("文件"));
	openProjAction = new QAction(QIcon("../resources/open.png"), QStringLiteral("打开工程"), this);
	openProjAction->setShortcut(tr("ctrl+o"));
	file->addAction(openProjAction);
	connect(openProjAction, SIGNAL(triggered(bool)), this, SLOT(openProj()));

	saveProjAction = new QAction(QIcon("../resources/save.png"), QStringLiteral("保存工程"), this);
	saveProjAction->setShortcut(tr("ctrl+s"));
	file->addAction(saveProjAction);
	connect(saveProjAction, SIGNAL(triggered(bool)), this, SLOT(saveProj()));

	clsProjAction = new QAction(QIcon("../resources/cls.png"), QStringLiteral("关闭工程"), this);
	clsProjAction->setShortcut(tr("ctrl+e"));
	file->addAction(clsProjAction);
	connect(clsProjAction, SIGNAL(triggered(bool)), this, SLOT(clsProj()));

	QPointer<QMenu> terrain = ui.menuBar->addMenu(QStringLiteral("地形"));
	addMapAction = new QAction(QIcon("../resources/map.png"), QStringLiteral("导入"), this);
	addMapAction->setShortcut(tr("ctrl+t"));
	terrain->addAction(addMapAction);
	connect(addMapAction, SIGNAL(triggered(bool)), this, SLOT(addMap()));

	QPointer<QMenu> path = ui.menuBar->addMenu(QStringLiteral("轨迹"));
	addPathAction = new QAction(QIcon("../resources/path.png"), QStringLiteral("导入"), this);
	addPathAction->setShortcut(tr("ctrl+p"));
	path->addAction(addPathAction);
	addPat = new AddPath(this);
	connect(addPathAction, SIGNAL(triggered(bool)), addPat, SLOT(open()));
	connect(addPat, SIGNAL(dbInfo(common::Proj)), this, SLOT(addPath(common::Proj)));

	QPointer<QMenu> conf = ui.menuBar->addMenu(QStringLiteral("设置"));
	confAnimateAction = new QAction(QIcon("../resources/animation.png"), QStringLiteral("动画参数"), this);
	confAnimateAction->setShortcut(tr("ctrl+a"));
	conf->addAction(confAnimateAction);
	confAnimate = new ConfAnimate(this);
	connect(confAnimateAction, SIGNAL(triggered(bool)), confAnimate, SLOT(open()));

	confPersonAction = new QAction(QIcon("../resources/person.png"), QStringLiteral("运动目标参数"), this);
	confPersonAction->setShortcut(tr("ctrl+r"));
	conf->addAction(confPersonAction);
	confPerson = new ConfPerson(this);
	connect(confPersonAction, SIGNAL(triggered(bool)), confPerson, SLOT(open()));


	// help menu
	ui.menuBar->addMenu(QStringLiteral("帮助"));

	startAnimateAction = new QAction(QIcon("../resources/play.png"), QStringLiteral("播放"), this);
	ui.mainToolBar->setIconSize(QSize(16, 16));
	ui.mainToolBar->addAction(startAnimateAction);


	pauseAnimateAction = new QAction(QIcon("../resources/pause.png"), QStringLiteral("播放"), this);
	ui.mainToolBar->addAction(pauseAnimateAction);
}

QString MainWindow::conDB(const QString& dbName,
					   const QString& host,
					   const QString& usr,
					   const QString& pwd) {
	QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
	db.setHostName(host);
	db.setDatabaseName(dbName);
	db.setUserName(usr);
	db.setPassword(pwd);
		
	if (!db.open()) {
		QMessageBox::critical(this,
							  "Database Error",
							  db.lastError().text());
		return QString::null;
	}
	return db.connectionName();
}

void MainWindow::openProj() {
	QString prj_path = QFileDialog::getOpenFileName(this,
													QStringLiteral("打开工程文件"),
													"../",
													QStringLiteral("工程文件 (*.prj)"));
	if (prj_path.isEmpty()) {
		QMessageBox::information(this,
								 QStringLiteral("提示"),
								 QStringLiteral("请添加工程文件！"),
								 QStringLiteral("是"));
		return;
	}

	QFile prj_file(prj_path);
	if (!prj_file.exists()) {
		QMessageBox::warning(this,
							 QStringLiteral("警告"),
							 QStringLiteral("工程文件不存在！"));
	}

	if (!prj_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::warning(this,
							 QStringLiteral("警告"),
							 QStringLiteral("工程文件不能打开！"));
	}

	clsProj();

	m_prj.hostname = prj_file.readLine().trimmed();
	m_prj.dbname = prj_file.readLine().trimmed();
	m_prj.usrname = prj_file.readLine().trimmed();
	m_prj.pwd = prj_file.readLine().trimmed();
	m_prj.tablename = prj_file.readLine().trimmed();
	m_prj.terrain_path = prj_file.readLine().trimmed();

	prj_file.close();

	addViewWidget();
	addPath();
}

void MainWindow::saveProj() {
	QString prj_path = QFileDialog::getSaveFileName(this,
													QStringLiteral("保存工程文件"),
													"../",
													QStringLiteral("工程文件 (*.prj)"));

	QFile prj_file(prj_path);
	if (!prj_file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&prj_file);
	out << m_prj.hostname << "\n"
		<< m_prj.dbname << "\n"
		<< m_prj.usrname << "\n"
		<< m_prj.pwd << "\n"
		<< m_prj.tablename << "\n"
		<< m_prj.terrain_path;

	prj_file.close();
}

void MainWindow::clsProj() {
	if (viewWidget) {
		delete viewWidget.data();
		viewWidget = nullptr;

		m_prj = {};
	}
}

void MainWindow::addMap() {
	QString map_path = QFileDialog::getOpenFileName(this,
													QStringLiteral("导入地形"),
													"../",
													QStringLiteral("地形文件 (*.ive *.osg *.3ds)"));
	if (map_path.isEmpty()) {
		QMessageBox::information(this,
								 QStringLiteral("提示"),
								 QStringLiteral("请添加地形文件！"),
								 QStringLiteral("是"));
		return;
	}

	m_prj.terrain_path = map_path;
	addViewWidget();
}

void MainWindow::addPath() {
	QString con_name = conDB(m_prj.dbname, m_prj.hostname, m_prj.usrname, m_prj.pwd);
	if (con_name.isNull()) {
		return;
	}

	QList<common::Pos> pos_lst = importPath();
	viewWidget->setPath(pos_lst);
	pos_lst.clear();

	QSqlDatabase::removeDatabase(con_name);

	QMessageBox::information(this,
		QStringLiteral("提醒"),
		QStringLiteral("轨迹加载完毕！")
		);
}

void MainWindow::addPath(common::Proj p) {
	p.terrain_path = m_prj.terrain_path;
	m_prj = p;

	addPath();
}

void MainWindow::setStatusBar(QString str) {
	QString s = QStringLiteral("当前位置:(") + str + ")";
	statusBar()->showMessage(s, 500);
}

void MainWindow::addViewWidget() {
	viewWidget = new ViewerWidget(m_prj.terrain_path,
		"../resources/human_red.png",
		osgViewer::ViewerBase::SingleThreaded);
	viewWidget->setGeometry(100, 100, 800, 600);
	this->setCentralWidget(viewWidget);

	connect(startAnimateAction, SIGNAL(triggered(bool)), viewWidget, SLOT(play()));
	connect(pauseAnimateAction, SIGNAL(triggered(bool)), viewWidget, SLOT(pause()));

	connect(viewWidget, SIGNAL(currentPos(QString)), this, SLOT(setStatusBar(QString)));
	connect(confPerson, SIGNAL(personInfo(common::Person)), viewWidget, SLOT(setPersonInfo(common::Person)));
	connect(confAnimate, SIGNAL(animateInfo(int, common::Color)), viewWidget, SLOT(setAnimateInfo(int, common::Color)));

	QMessageBox::information(this,
		QStringLiteral("提醒"),
		QStringLiteral("地形加载完毕！")
		);
}

QList<common::Pos> MainWindow::importPath() {
	QList<common::Pos> pos_lst;

	QSqlQuery query;
	query.exec(QString("SELECT x, y, height, state FROM %1").arg(m_prj.tablename));

	while (query.next()) {
		float x = query.value(0).toFloat();
		float y = query.value(1).toFloat();
		int h = query.value(2).toInt();
		common::SS s = (common::SS)query.value(3).toInt();

		common::Pos p{ x, y, h, s };
		pos_lst.push_back(p);
	}

	return  pos_lst;
}