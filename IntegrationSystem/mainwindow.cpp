#include "mainwindow.h"

#include "ViewerWidget.h"

#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent){
	ui.setupUi(this);

	setWindowTitle(QStringLiteral("��̬�켣չʾϵͳ"));
	setAcceptDrops(false);

	initSkin("../style/style.qss");
	initBar();
}

MainWindow::~MainWindow() {
}

void MainWindow::initSkin(const QString& filename){
	QFile f(filename);
	if (!f.exists()){
		QMessageBox::warning(this,
			QStringLiteral("����"),
			QStringLiteral("Ƥ����ʼ��ʧ�ܣ�"));
	}
	else{
		f.open(QFile::ReadOnly | QFile::Text);
		QTextStream ts(&f);
		setStyleSheet(ts.readAll());
	}
}

void MainWindow::initBar() {
	QPointer<QMenu> file = ui.menuBar->addMenu(QStringLiteral("�ļ�"));
	openProjAction = new QAction(QIcon("../resources/open.png"), QStringLiteral("�򿪹���"), this);
	openProjAction->setShortcut(tr("ctrl+o"));
	file->addAction(openProjAction);
	connect(openProjAction, SIGNAL(triggered(bool)), this, SLOT(openProj()));

	saveProjAction = new QAction(QIcon("../resources/save.png"), QStringLiteral("���湤��"), this);
	saveProjAction->setShortcut(tr("ctrl+s"));
	file->addAction(saveProjAction);
	connect(saveProjAction, SIGNAL(triggered(bool)), this, SLOT(saveProj()));

	clsProjAction = new QAction(QIcon("../resources/cls.png"), QStringLiteral("�رչ���"), this);
	clsProjAction->setShortcut(tr("ctrl+e"));
	file->addAction(clsProjAction);
	connect(clsProjAction, SIGNAL(triggered(bool)), this, SLOT(clsProj()));

	QPointer<QMenu> conf = ui.menuBar->addMenu(QStringLiteral("����"));
	confProjAction = new QAction(QIcon("../resources/conf.png"), QStringLiteral("����"), this);
	confProjAction->setShortcut(tr("ctrl+c"));
	conf->addAction(confProjAction);

	// help menu
	ui.menuBar->addMenu(QStringLiteral("����"));
}

void MainWindow::openProj() {
	QString prj_path = QFileDialog::getOpenFileName(this,
		QStringLiteral("�򿪹����ļ�"),
		"../",
		QStringLiteral("�����ļ� (*.prj)"));
	if (prj_path.isEmpty()){
		QMessageBox::information(this,
			QStringLiteral("��ʾ"),
			QStringLiteral("����ӹ����ļ���"),
			QStringLiteral("��"));
		return;
	}

	QFile prj_file(prj_path);
	if (!prj_file.exists()) {
		QMessageBox::warning(this,
			QStringLiteral("����"),
			QStringLiteral("�����ļ������ڣ�"));
	}

	if (!prj_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::warning(this,
			QStringLiteral("����"),
			QStringLiteral("�����ļ����ܴ򿪣�"));
	}

	QString db_name, usr_name, usr_pwd, terrain_path;
	db_name = prj_file.readLine();
	usr_name = prj_file.readLine();
	usr_pwd = prj_file.readLine();
	terrain_path = prj_file.readLine();
	m_prj = { db_name, usr_name, usr_pwd, terrain_path };
	prj_file.close();

	readPrj();
}

void MainWindow::saveProj() {
	QString prj_path = QFileDialog::getSaveFileName(this,
		QStringLiteral("���湤���ļ�"),
		"../",
		QStringLiteral("�����ļ� (*.prj)"));

	QFile prj_file(prj_path);
	if (!prj_file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&prj_file);
	out << m_prj.dbname << "\n"
		<< m_prj.usrname << "\n"
		<< m_prj.pwd << "\n"
		<< m_prj.terrain_path;

	prj_file.close();
}

void MainWindow::clsProj(){
	delete viewWidget;
	viewWidget = nullptr;
}

void MainWindow::readPrj() {
	osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::SingleThreaded;
	viewWidget = new ViewerWidget(threadingModel);
	viewWidget->addNodeFile(m_prj.terrain_path);
	viewWidget->setGeometry(100, 100, 800, 600);
	this->setCentralWidget(viewWidget);
}