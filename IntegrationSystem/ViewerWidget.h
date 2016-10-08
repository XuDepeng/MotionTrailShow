#pragma once
#include <QWidget>
#include <QTimer>

#include <osgViewer/CompositeViewer>
#include <osgQt/GraphicsWindowQt>
#include <osgText/Text>

#include "Common.h"

class ViewerWidget : public QWidget, public osgViewer::CompositeViewer {
	Q_OBJECT
public:
	ViewerWidget(const QString& terrain, const QString& target,
		osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::CompositeViewer::SingleThreaded);
	~ViewerWidget();

	void setPath(const QList<common::Pos>& p);

	public slots:
	void setAnimateInfo(int interval, common::Color c);
	void setPersonInfo(common::Person p);

	void play();
	void pause();

	private slots:
	void updateTarget();

signals:
	void currentPos(QString position);

private:
	void init();

	osg::Node* createTerrain(const QString& filename);
	osg::Geometry* createTarget(const QString& filename,
		const common::Pos& p);
	osg::Camera* createTextCam();
	osgText::Text* createText(const std::wstring& t, int x, int y);

	QWidget* addViewWidget(osgQt::GraphicsWindowQt* gw);
	osgQt::GraphicsWindowQt* createGraphicsWindow(int x, int y, int w, int h);
	virtual void paintEvent(QPaintEvent* event);

protected:
	QTimer m_drawtimer;
	QTimer m_updatetimer;

private:
	osg::ref_ptr<osgViewer::View> m_view;
	osg::ref_ptr<osg::Geometry> m_target;
	osg::ref_ptr<osg::Node> m_terrain;

	osg::ref_ptr<osg::Image> m_image_red, m_image_blue, m_image_yellow;
	osg::ref_ptr<osgText::Text> m_speed;

	QList<common::Pos> m_plist;
	int m_idx;
	int m_interval;
	common::Person m_person;
};
