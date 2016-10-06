#include <QWidget>
#include <QTimer>

#include <osgViewer/CompositeViewer>
#include <osgQt/GraphicsWindowQt>

class ViewerWidget : public QWidget, public osgViewer::CompositeViewer{
public:
	ViewerWidget(osgViewer::ViewerBase::ThreadingModel threadingModel =
		osgViewer::CompositeViewer::SingleThreaded);

	void addNodeFile(QString& filename);

	QWidget* addViewWidget(osgQt::GraphicsWindowQt* gw);

	osgQt::GraphicsWindowQt* createGraphicsWindow(int x, int y, int w, int h,
		const std::string& name = "", bool windowDecoration = false);

	virtual void paintEvent(QPaintEvent* event);
protected:
	QTimer m_timer;

private:
	osg::ref_ptr<osgViewer::View> m_view;
};