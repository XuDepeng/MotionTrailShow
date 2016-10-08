#include "ViewerWidget.h"

#include <QGridLayout>
#include <QTimer>
#include <QPointer>

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgDB/ReadFile>
#include <osgQt/GraphicsWindowQt>
#include <osgUtil/Optimizer>

ViewerWidget::ViewerWidget(const QString& terrain, const QString& target,
	osgViewer::ViewerBase::ThreadingModel threadingModel) : QWidget() {
	setThreadingModel(threadingModel);
	setKeyEventSetsDone(0);
	setDataVariance(Object::DYNAMIC);

	init();

	m_terrain = createTerrain(terrain);
	m_terrain->setDataVariance(osg::Object::DYNAMIC);

	osg::BoundingSphere bs = m_terrain->computeBound();
	common::Pos p = { bs.center().x(), bs.center().y(), bs.center().z(), common::ACCELERATE };

	m_target = createTarget(target, p);
	m_target->setDataVariance(osg::Object::DYNAMIC);

	osg::ref_ptr<osgQt::GraphicsWindowQt> gw = createGraphicsWindow(0, 0, 100, 100);
	QPointer<QWidget> w = addViewWidget(gw.get());
	QPointer<QGridLayout> grid = new QGridLayout;
	grid->addWidget(w, 0, 0);
	setLayout(grid);

	connect(&m_drawtimer, SIGNAL(timeout()), this, SLOT(update()));
	m_drawtimer.start(20);
	connect(&m_updatetimer, SIGNAL(timeout()), this, SLOT(updateTarget()));
}

ViewerWidget::~ViewerWidget() {
	if (m_plist.count()) {
		m_plist.clear();
	}
}

void ViewerWidget::init() {
	m_image_red = osgDB::readImageFile("../resources/human_red.png");
	m_image_blue = osgDB::readImageFile("../resources/human_blue.png");
	m_image_yellow = osgDB::readImageFile("../resources/human_yellow.png");
	m_image_red->scaleImage(512, 512, 1);
	m_image_blue->scaleImage(512, 512, 1);
	m_image_yellow->scaleImage(512, 512, 1);

	m_view = new osgViewer::View;
	m_view->setDataVariance(Object::DYNAMIC);

	m_person = { "unnamed", 0, 0, 0 };
	m_idx = 0;
	m_interval = 500;
}

void ViewerWidget::setPath(const QList<common::Position>& p) {
	if (m_plist.count()) {
		m_plist.clear();
	}
	m_plist = p;
}

void ViewerWidget::setAnimateInfo(int interval, common::Color c) {
	osg::ref_ptr<osg::Camera> camera = m_view->getCamera();
	camera->setClearColor(osg::Vec4(c.r / 255., c.g / 255., c.b / 255., 1.0));
	m_interval = interval;
	m_updatetimer.setInterval(m_interval);
}

void ViewerWidget::setPersonInfo(common::Person p) {
	m_person = p;
}

QWidget* ViewerWidget::addViewWidget(osgQt::GraphicsWindowQt* gw) {
	addView(m_view.get());

	osg::ref_ptr<osg::Camera> camera = m_view->getCamera();
	camera->setGraphicsContext(gw);

	osg::ref_ptr<const osg::GraphicsContext::Traits> traits = gw->getTraits();

	camera->setClearColor(osg::Vec4(0.2, 0.2, 0.6, 1.0));
	camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
	camera->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(traits->width) /
		static_cast<double>(traits->height), 1.0f, 10000.0f);

	// light off, from wherever the image looks the same
	osg::ref_ptr<osg::Geode> tar_geode = new osg::Geode;
	tar_geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	tar_geode->addDrawable(m_target.get());

	osg::ref_ptr<osg::Geode> tex_geode = new osg::Geode;
	osg::ref_ptr<osg::Camera> text_cam = createTextCam();
	m_speed = createText(L"", width() - 125, height() - 20);
	m_speed->setDataVariance(osg::Object::DYNAMIC);
	tex_geode->addDrawable(m_speed);
	text_cam->addChild(tex_geode.get());

	osg::ref_ptr<osg::Group> root = new osg::Group;
	root->addChild(tar_geode.get());
	root->addChild(m_terrain.get());
	root->addChild(text_cam.get());
	root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	osgUtil::Optimizer op;
	op.optimize(root);

	m_view->setSceneData(root.get());

	m_view->addEventHandler(new osgViewer::StatsHandler);
	m_view->setCameraManipulator(new osgGA::TrackballManipulator);
	m_view->addEventHandler(new osgGA::StateSetManipulator(m_view->getCamera()->getOrCreateStateSet()));
	m_view->addEventHandler(new osgViewer::ScreenCaptureHandler);

	return gw->getGLWidget();
}

osgQt::GraphicsWindowQt* ViewerWidget::createGraphicsWindow(int x, int y, int w, int h) {
	osg::ref_ptr<osg::DisplaySettings> ds = osg::DisplaySettings::instance().get();
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
	traits->windowName = "";
	traits->windowDecoration = false;
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

void ViewerWidget::paintEvent(QPaintEvent* event) {
	frame();
}

void ViewerWidget::updateTarget() {
	if (m_idx >= m_plist.count()) {
		m_speed->setText(L"轨迹已绘制完毕！");
		m_updatetimer.stop();
		return;
	}

	common::Pos cur = m_plist.at(m_idx);
	QString pos_str = "%1,%2,%3";
	pos_str = pos_str.arg(cur.x).arg(cur.y).arg(cur.h);
	emit currentPos(pos_str);

	osg::ref_ptr<osg::Vec3Array> vertices = dynamic_cast<osg::Vec3Array*>(m_target->getVertexArray());
	osg::Vec3 cur_pos = osg::Vec3f(m_plist.at(m_idx).x - 100.f, m_plist.at(m_idx).y, 2 * m_plist.at(m_idx).h - 28.f);
	(*vertices)[0] = cur_pos;
	(*vertices)[1] = osg::Vec3(cur_pos.x() + 200.f, cur_pos.y(), cur_pos.z());
	(*vertices)[2] = osg::Vec3(cur_pos.x() + 200.f, cur_pos.y(), cur_pos.z() + 200.f);
	(*vertices)[3] = osg::Vec3(cur_pos.x(), cur_pos.y(), cur_pos.z() + 200.f);
	m_target->setVertexArray(vertices.get());

	// update texture
	osg::ref_ptr<osg::StateAttribute> sa =
		m_target->getOrCreateStateSet()->getTextureAttribute(0, osg::StateAttribute::TEXTURE);
	osg::ref_ptr<osg::Texture2D> texture = (osg::Texture2D*)sa->asTexture();

	switch (m_plist[m_idx].state) {
	case common::CONSTANT:
		texture->setImage(m_image_yellow);
		break;
	case common::ACCELERATE:
		texture->setImage(m_image_red);
		break;
	case common::DECELERTATE:
		texture->setImage(m_image_blue);
		break;
	default:
		texture->setImage(m_image_yellow);
		break;
	}

	if (m_idx > 0) {
		float offset = m_plist[m_idx] - m_plist[m_idx - 1];
		QString s = QStringLiteral("当前运动目标名称:%1\n")
			+ QStringLiteral("当前速度:%2 m/s\n")
			+ QStringLiteral("当前热量:%3 cal");
		s = s.arg(m_person.name).arg(offset).arg(m_person.weight * offset * 1.036);
		m_speed->setText(s.toStdWString().c_str());

		osg::ref_ptr<osg::Geode> geode = dynamic_cast<osg::Geode*>(m_target->getParent(0));
		osg::Vec3 sp(m_plist[m_idx - 1].x, m_plist[m_idx - 1].y, 2 * m_plist[m_idx - 1].h + 10.f);
		osg::Vec3 ep(m_plist[m_idx].x, m_plist[m_idx].y, 2 * m_plist[m_idx].h + 10.f);
		osg::ref_ptr<osg::Geometry> beam = new osg::Geometry;
		osg::ref_ptr<osg::Vec3Array> points = new osg::Vec3Array;
		points->push_back(sp);
		points->push_back(ep);
		osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
		color->push_back(osg::Vec4(1.0, 0.0, 0.0, 1.0));
		beam->setVertexArray(points.get());
		beam->setColorArray(color.get());
		beam->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);
		beam->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, 2));
		geode->addDrawable(beam);
	}

	vertices->dirty();
	m_target->dirtyDisplayList();
	m_target->dirtyBound();

	m_idx++;
}

void ViewerWidget::play() {
	m_updatetimer.start(m_interval);
}

void ViewerWidget::pause() {
	m_updatetimer.stop();
}

osg::Node* ViewerWidget::createTerrain(const QString& filename) {
	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(filename.toStdString());
	return node.release();
}

osg::Geometry* ViewerWidget::createTarget(const QString& filename, const common::Pos& p) {
	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;

	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array();
	v->setDataVariance(osg::Object::DYNAMIC);
	v->push_back(osg::Vec3(p.x, p.y, p.h));
	v->push_back(osg::Vec3(p.x + 200.f, p.y, p.h));
	v->push_back(osg::Vec3(p.x + 200.f, p.y, p.h + 200.f));
	v->push_back(osg::Vec3(p.x, p.y, p.h + 200.f));
	geom->setVertexArray(v.get());

	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Y_AXIS);
	geom->setNormalArray(normal.get());
	geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

	osg::ref_ptr<osg::Vec2Array> tcoords = new osg::Vec2Array;
	tcoords->push_back(osg::Vec2(0.0f, 0.0f));
	tcoords->push_back(osg::Vec2(1.0f, 0.0f));
	tcoords->push_back(osg::Vec2(1.0f, 1.0f));
	tcoords->push_back(osg::Vec2(0.0f, 1.0f));
	geom->setTexCoordArray(0, tcoords.get());

	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	osg::ref_ptr<osg::Image> image_red = osgDB::readImageFile(filename.toStdString());
	image_red->scaleImage(512, 512, 1);
	texture->setImage(image_red);
	geom->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

	// blend for transparency
	geom->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
	geom->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	return geom.release();
}

osg::Camera* ViewerWidget::createTextCam() {
	osg::ref_ptr<osg::Camera> text_cam = new osg::Camera;
	text_cam->setProjectionMatrix(osg::Matrixd::ortho2D(0, width(), 0, height()));
	text_cam->setReferenceFrame(osg::Transform::ABSOLUTE_RF); // absolute frame refrence
	text_cam->setViewMatrix(osg::Matrix::identity());
	text_cam->setClearMask(GL_DEPTH_BUFFER_BIT); // clear buffer
	text_cam->setRenderOrder(osg::Camera::POST_RENDER);
	text_cam->setAllowEventFocus(false);

	return text_cam.release();
}

osgText::Text* ViewerWidget::createText(const std::wstring& t, int x, int y) {
	osg::ref_ptr<osgText::Font> font = new osgText::Font;
	font = osgText::readFontFile("simhei.ttf");

	osg::ref_ptr<osgText::Text> text = new osgText::Text;
	text->setFont(font.get());
	text->setText(t._Myptr());
	text->setPosition(osg::Vec3(x, y, 0));
	text->setCharacterSize(10.f);
	text->setFontResolution(60, 60);
	text->setColor(osg::Vec4(1.f, 1.f, 1.f, 1.f));
	text->setDrawMode(osgText::Text::TEXT);
	text->setAxisAlignment(osgText::Text::SCREEN);

	return text.release();
}