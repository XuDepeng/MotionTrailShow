#pragma once
#include <QtWidgets/QMainWindow>
#include <QPointer>
#include <QAction>

#include "ui_integrationsystem.h"

#include "Common.h"

class ViewerWidget;
class ConfPerson;
class ConfAnimate;
class AddPath;

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget* parent = 0);
	~MainWindow();

private:
	void initSkin(const QString& filename);
	void initBar();

	QString conDB(const QString& dbName,
		const QString& host = "localhost",
		const QString& usr = "root",
		const QString& pwd = "root");

	void addViewWidget();

	QList<common::Pos> importPath();

	private slots:
	void openProj();
	void saveProj();
	void clsProj();

	void addMap();
	void addPath();
	void addPath(common::Proj p);

	void setStatusBar(QString str);
private:
	Ui::IntegrationSystemClass ui;
	QPointer<ViewerWidget> viewWidget;
	QPointer<ConfAnimate> confAnimate;
	QPointer<ConfPerson> confPerson;
	QPointer<AddPath> addPat;

	QPointer<QAction> openProjAction;
	QPointer<QAction> saveProjAction;
	QPointer<QAction> clsProjAction;

	QPointer<QAction> confBgAction;
	QPointer<QAction> confAnimateAction;
	QPointer<QAction> confPersonAction;

	QPointer<QAction> addMapAction;
	QPointer<QAction> addPathAction;

	QPointer<QAction> startAnimateAction;
	QPointer<QAction> pauseAnimateAction;

	common::Proj m_prj;
};