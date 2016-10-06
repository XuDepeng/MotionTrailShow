#include "ViewerWidget.h"

#include <QGridLayout>
#include <QTimer>
#include <QPointer>

#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgDB/ReadFile>
#include <osgQt/GraphicsWindowQt>

ViewerWidget::ViewerWidget(osgViewer::ViewerBase::ThreadingModel threadingModel) : QWidget(){
	setThreadingModel(threadingModel);
	setKeyEventSetsDone(0);
	setDataVariance(Object::DYNAMIC);

	QPointer<QWidget> w = addViewWidget(createGraphicsWindow(0, 0, 100, 100));
	QPointer<QGridLayout> grid = new QGridLayout;
	grid->addWidget(w, 0, 0);
	setLayout(grid);

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));
	m_timer.start(10);
}

QWidget* ViewerWidget::addViewWidget(osgQt::GraphicsWindowQt* gw) {
	m_view = new osgViewer::View;
	m_view->setDataVariance(Object::DYNAMIC);
	addView(m_view);

	osg::ref_ptr<osg::Camera> camera = m_view->getCamera();
	camera->setGraphicsContext(gw);

	osg::ref_ptr<const osg::GraphicsContext::Traits> traits = gw->getTraits();

	camera->setClearColor(osg::Vec4(0.2, 0.2, 0.6, 1.0));
	camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
	camera->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(traits->width) /
		static_cast<double>(traits->height), 1.0f, 10000.0f);

	m_view->setSceneData(nullptr);
	m_view->addEventHandler(new osgViewer::StatsHandler);
	m_view->setCameraManipulator(new osgGA::TrackballManipulator);
	m_view->addEventHandler(new osgGA::StateSetManipulator(m_view->getCamera()->getOrCreateStateSet()));
	m_view->addEventHandler(new osgViewer::ThreadingHandler);
	m_view->addEventHandler(new osgViewer::WindowSizeHandler);
	m_view->addEventHandler(new osgViewer::StatsHandler);
	m_view->addEventHandler(new osgViewer::RecordCameraPathHandler);
	m_view->addEventHandler(new osgViewer::LODScaleHandler);
	m_view->addEventHandler(new osgViewer::ScreenCaptureHandler);

	return gw->getGLWidget();
}

osgQt::GraphicsWindowQt* ViewerWidget::createGraphicsWindow(int x, int y, int w, int h,
	const std::string& name, bool windowDecoration) {
	osg::ref_ptr<osg::DisplaySettings> ds = osg::DisplaySettings::instance().get();
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
	traits->windowName = name;
	traits->windowDecoration = windowDecoration;
	traits->x = x;
	traits->y = y;
	traits->width = w;
	traits->height = h;
	traits->doubleBuffer = true;
	traits->alpha = ds->getMinimumNumAlphaBits();
	traits->stencil = ds->getMinimumNumStencilBits();
	traits->sampleBuffers = ds->getMultiSamples();
	traits->samples = ds->getNumMultiSamples();
	return new osgQt::GraphicsWindowQt(traits.get());
}

void ViewerWidget::paintEvent(QPaintEvent* event)	{
	frame();
}

void ViewerWidget::addNodeFile(QString& filename){
	m_view->setSceneData(osgDB::readNodeFile(filename.toStdString()));
}