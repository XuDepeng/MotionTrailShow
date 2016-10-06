#define WIN32_LEAN_AND_MEAN
#define NOIME
#include <Windows.h>

#include <osg/Node>
#include <osg/Group>
#include <osg/ShapeDrawable>
#include <osg/DrawPixels>
#include <osg/PositionAttitudeTransform>

#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgUtil/optimizer>

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <thread>

#include "MotionParser.h"

#define MODEL 1
#define PNG 0

static osg::ref_ptr<osg::Image> s_image_red, s_image_blue, s_image_yellow;
static osg::ref_ptr<osgText::Text> s_speed;
static bool s_update = false;

void time_count() {
	while (1) {
		Sleep(500);
		s_update = true;
	}
}

struct DrawpixelsUpdateCallback :public osg::Drawable::UpdateCallback {
	DrawpixelsUpdateCallback(const vector<Pos>& pos) : m_pos(pos), m_idx(0) {
	}

	void update(osg::NodeVisitor* nv, osg::Drawable* drawable) {
		if (!s_update) {
			return;;
		}

		if (m_idx >= m_pos.size()) {
			return;
		}

		osg::ref_ptr<osg::DrawPixels> dp = dynamic_cast<osg::DrawPixels*>(drawable);

		switch (m_pos[m_idx].state) {
		case CONSTANT:
			dp->setImage(s_image_yellow);
			break;
		case ACCELERATE:
			dp->setImage(s_image_red);
			break;
		case DECELERTATE:
			dp->setImage(s_image_blue);
			break;
		default:
			dp->setImage(s_image_yellow);
			break;
		}

		osg::Vec3 cur_pos = osg::Vec3(m_pos[m_idx].x, m_pos[m_idx].y, (float)m_pos[m_idx].h * 2 - 90.f);
		dp->setPosition(cur_pos);

		// update speed
		if (m_idx > 0) {
			float offset = m_pos[m_idx] - m_pos[m_idx - 1];
			std::string s = std::to_string(offset);
			s += " m/s";
			s_speed->setText(s);
			s_speed->setPosition(osg::Vec3(cur_pos.x() + 180.f, cur_pos.y(), cur_pos.z() + 650));
		}

		dp->dirtyDisplayList();
		dp->dirtyBound();

		m_idx++;

		s_update = false;
	}

	vector<Pos> m_pos;
	int m_idx;
};

struct GeomUpdateCallback :public osg::Drawable::UpdateCallback {
	GeomUpdateCallback(const vector<Pos>& pos) : m_pos(pos), m_idx(0) {
	}

	void update(osg::NodeVisitor* nv, osg::Drawable* drawable) {
		if (!s_update) {
			return;;
		}

		if (m_idx >= m_pos.size()) {
			return;
		}

		osg::ref_ptr<osg::Geometry> geom = dynamic_cast<osg::Geometry*>(drawable);
		osg::ref_ptr<osg::Vec3Array> vertices = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());

		// update texture
		osg::ref_ptr<osg::StateAttribute> sa =
			geom->getOrCreateStateSet()->getTextureAttribute(0, osg::StateAttribute::TEXTURE);
		osg::ref_ptr<osg::Texture2D> texture = (osg::Texture2D*)sa->asTexture();
		switch (m_pos[m_idx].state) {
		case CONSTANT:
			texture->setImage(s_image_yellow);
			break;
		case ACCELERATE:
			texture->setImage(s_image_red);
			break;
		case DECELERTATE:
			texture->setImage(s_image_blue);
			break;
		default:
			texture->setImage(s_image_yellow);
			break;
		}

		osg::Vec3 cur_pos = osg::Vec3(m_pos[m_idx].x - 300.f, m_pos[m_idx].y, (float)m_pos[m_idx].h * 2 - 90.f);

		// update speed
		if (m_idx > 0) {
			float offset = m_pos[m_idx] - m_pos[m_idx - 1];
			std::string s = std::to_string(offset);
			s += " m/s";
			s_speed->setText(s);
			s_speed->setPosition(osg::Vec3(cur_pos.x() + 180.f, cur_pos.y(), cur_pos.z() + 650));

			osg::ref_ptr<osg::Geode> geode = dynamic_cast<osg::Geode*>(geom->getParent(0));
			osg::Vec3 sp(m_pos[m_idx - 1].x, m_pos[m_idx - 1].y, 2 * m_pos[m_idx - 1].h + 10.f);
			osg::Vec3 ep(m_pos[m_idx].x, m_pos[m_idx].y, 2 * m_pos[m_idx].h + 10.f);
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

		(*vertices)[0] = cur_pos;
		(*vertices)[1] = osg::Vec3(cur_pos.x() + 600.f, cur_pos.y(), cur_pos.z());
		(*vertices)[2] = osg::Vec3(cur_pos.x() + 600.f, cur_pos.y(), cur_pos.z() + 600.f);
		(*vertices)[3] = osg::Vec3(cur_pos.x(), cur_pos.y(), cur_pos.z() + 600.f);
		geom->setVertexArray(vertices.get());

		vertices->dirty();
		geom->dirtyDisplayList();
		geom->dirtyBound();

		m_idx++;

		s_update = false;
	}

	vector<Pos> m_pos;
	int m_idx;
};

struct PatUpdateCallback :public osg::NodeCallback {
	PatUpdateCallback(const vector<Pos>& pos) : m_pos(pos), m_idx(0) {
	}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)	{
		if (!s_update) {
			return;;
		}

		if (m_idx >= m_pos.size()) {
			return;
		}

		osg::ref_ptr<osg::PositionAttitudeTransform> pat =
			dynamic_cast<osg::PositionAttitudeTransform*>(node);

		osg::Vec3 cur_pos = osg::Vec3(m_pos[m_idx].x, m_pos[m_idx].y, (float)m_pos[m_idx].h * 2 + 450.f);
		pat->setPosition(cur_pos);

		// update speed
		if (m_idx > 0) {
			float offset = m_pos[m_idx] - m_pos[m_idx - 1];
			std::string s = std::to_string(offset);
			s += " m/s";
			s_speed->setText(s);
			s_speed->setPosition(osg::Vec3(cur_pos.x(), cur_pos.y(), cur_pos.z() + 250));
		}

		m_idx++;
		s_update = false;

		// note, callback is responsible for scenegraph traversal so
		// they must call traverse(node,nv) to ensure that the
		// scene graph subtree (and associated callbacks) are traversed.
		traverse(node, nv);
	}

	vector<Pos> m_pos;
	int m_idx;
};

struct GeodeUpdateCallback :public osg::NodeCallback {
	GeodeUpdateCallback(const vector<Pos>& pos) : m_pos(pos), m_idx(0) {
	}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)	{
		if (!s_update) {
			printf("te\n");
			return;
		}

		if (m_idx >= m_pos.size()) {
			return;
		}

		osg::ref_ptr<osg::Geode> geode = dynamic_cast<osg::Geode*>(node);
		osg::Vec3 cur_pos = osg::Vec3(m_pos[m_idx].x, m_pos[m_idx].y, (float)m_pos[m_idx].h * 2 + 450.f);

		// update speed
		if (m_idx > 0) {
			osg::Vec3 sp(m_pos[m_idx - 1].x, m_pos[m_idx - 1].y, 2 * m_pos[m_idx - 1].h + 10.f);
			osg::Vec3 ep(m_pos[m_idx].x, m_pos[m_idx].y, 2 * m_pos[m_idx].h + 10.f);
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
			geode->addDrawable(beam.get());
		}

		m_idx++;
		s_update = false;
		traverse(node, nv);
	}

	vector<Pos> m_pos;
	int m_idx;
};

osg::ref_ptr<osg::Geode> create_text(osgText::Text* t) {
	osg::ref_ptr<osg::GraphicsContext::WindowingSystemInterface> wsi = osg::GraphicsContext::getWindowingSystemInterface();
	unsigned int width = 0, height = 0;
	wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);

	t->setFont("Fonts/simhei.ttf");
	t->setPosition(osg::Vec3(430320.13929, 3387250.20839, 1050.f));
	t->setColor(osg::Vec4(1.0, 1.0, 1.0, 1.0));
	t->setCharacterSize(100.0);
	t->setLayout(osgText::Text::LEFT_TO_RIGHT);
	t->setDrawMode(osgText::Text::TEXT);
	t->setAxisAlignment(osgText::Text::SCREEN);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(t);

	osg::ref_ptr<osg::StateSet> ss = geode->getOrCreateStateSet();
	ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	return geode.get();
}

int main(int argc, char** argv) {
	osg::ArgumentParser arg(&argc, argv);
	arg.getApplicationUsage()->setApplicationName(arg.getApplicationName());
	arg.getApplicationUsage()->setDescription(arg.getApplicationName() + " belongs to nnu xudepeg.");
	arg.getApplicationUsage()->setCommandLineUsage(arg.getApplicationName() + " [options] filename ...");

	osg::ref_ptr<osg::Group> root = new osg::Group;
	root->setDataVariance(osg::Object::DYNAMIC);
	std::cout << root << std::endl;

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->setDataVariance(osg::Object::DYNAMIC);
	root->addChild(geode.get());

	osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFiles(arg);
	terrain->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);
	root->addChild(terrain.get());

	// human
#if PNG
	s_image_red = osgDB::readImageFile("../data/human_red.png");
	s_image_blue = osgDB::readImageFile("../data/human_blue.png");
	s_image_yellow = osgDB::readImageFile("../data/human_yellow.png");

	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
	geom->setDataVariance(osg::Object::DYNAMIC);

	// light off, from wherever the image looks the same
	geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array();
	v->setDataVariance(osg::Object::DYNAMIC);
	v->push_back(osg::Vec3(429320.13929, 3387250.20839, 1050.f));
	v->push_back(osg::Vec3(429920.13929, 3387250.20839, 1050.f));
	v->push_back(osg::Vec3(429920.13929, 3387250.20839, 1650.f));
	v->push_back(osg::Vec3(429320.13929, 3387250.20839, 1650.f));
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
	s_image_red->scaleImage(512, 512, 1);
	s_image_blue->scaleImage(512, 512, 1);
	s_image_yellow->scaleImage(512, 512, 1);

	texture->setImage(s_image_red);
	geom->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

	// blend for transparency
	geom->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
	geom->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	geode->addDrawable(geom);

	root->addChild(geode);
#endif
#if MODEL
	osg::ref_ptr<osg::Node> model = osgDB::readNodeFile("../data/Finn/Finn.obj");
	osg::ref_ptr<osg::PositionAttitudeTransform> pat = new osg::PositionAttitudeTransform;
	pat->setDataVariance(osg::Object::DYNAMIC);
	pat->setPosition(osg::Vec3(430320.13929, 3387250.20839, 1500.f));
	pat->setScale(osg::Vec3(100, 100, 100));
	pat->addChild(model.get());
	root->addChild(pat.get());
#endif

	// speed
	s_speed = new osgText::Text;
	s_speed->setDataVariance(osg::Object::DYNAMIC);
	root->addChild(create_text(s_speed.get()));

	osgUtil::Optimizer optimizer;
	optimizer.optimize(root.get());
	osgViewer::Viewer viewer(arg);
	viewer.setSceneData(root);

	MotionParser mp;
	mp.parse("../data/motion_xy.csv");

#if PNG
	geom->setUpdateCallback(new GeomUpdateCallback(mp.getPosition()));
#endif
#if MODEL

	geode->setUpdateCallback(new GeodeUpdateCallback(mp.getPosition()));
	pat->setUpdateCallback(new PatUpdateCallback(mp.getPosition()));
#endif

	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
	viewer.addEventHandler(new osgViewer::ThreadingHandler);
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);
	viewer.addEventHandler(new osgViewer::LODScaleHandler);
	viewer.addEventHandler(new osgViewer::ScreenCaptureHandler);

	std::thread th(time_count);
	th.detach();

	// viewer.getCamera()->setClearColor(osg::Vec4(0.5f, 0.5f, 0.5f, 1.f));
	viewer.realize();
	viewer.run();
	return EXIT_SUCCESS;
}