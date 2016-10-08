#include <QApplication>

#include "MainWindow.h"
#include "ViewerWidget.h"

int main(int argc, char** argv) {
	osg::ArgumentParser arguments(&argc, argv);
	QApplication app(argc, argv);
	MainWindow is;
	is.show();
	return app.exec();
}