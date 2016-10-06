#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QPointer>
#include <QAction>

#include "ui_integrationsystem.h"

class ViewerWidget;

struct Proj{
	QString dbname;
	QString usrname;
	QString pwd;
	QString terrain_path;
};

class MainWindow : public QMainWindow{
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	void initSkin(const QString& filename);
	void initBar();

	void readPrj();

	private slots:
	void openProj();
	void saveProj();
	void clsProj();

private:
	Ui::IntegrationSystemClass ui;
	QPointer<ViewerWidget> viewWidget;

	QPointer<QAction> openProjAction;
	QPointer<QAction> saveProjAction;
	QPointer<QAction> clsProjAction;
	QPointer<QAction> confProjAction;

	Proj m_prj;
};
#endif