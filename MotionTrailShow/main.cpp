#define WIN32_LEAN_AND_MEAN
#define NOIME
#include <Windows.h>

#include <osg/Node>
#include <osg/Group>
#include <osg/ShapeDrawable>

#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <thread>

#include "MotionParser.h"

#define RANDOM_DATA
#define PI 3.1415926

static osg::ref_ptr<osg::Image> s_image_red, s_image_blue, s_image_yellow;

static bool s_update = false;

void time_count() {
	while (1) {
		Sleep(200);
		s_update = true;
	}
}

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

		osg::ref_ptr<osg::StateAttribute> sa = geom->getOrCreateStateSet()->getTextureAttribute(0, osg::StateAttribute::TEXTURE);
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

		float x_offset = 0.f, y_offset = 0.f;
		float v = rand() % 10;
		float theta = float(rand() % 471) / 100;
		x_offset = v * cos(theta - PI);
		y_offset = v * sin(theta);

		osg::Vec3 cur_pos = osg::Vec3(m_pos[m_idx].x, m_pos[m_idx].y, (float)m_pos[m_idx].h * 2);

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

#if 0
struct DrawableUpdateCallback : public osg::Drawable::UpdateCallback {
#ifdef  RANDOM_DATA
	DrawableUpdateCallback() {
		pre_pos = cur_pos = { 422371.688, 3380099.00, 450.f * 2 + 150.f };
		std::srand((unsigned)time(NULL));
	}
#else
	DrawableUpdateCallback(const vector<Pos>& pos) : m_pos(pos), m_idx(0) {
		if (!(pj_utm = pj_init_plus("+proj=utm +ellps=WGS84 +datum=WGS84 +north +zone=48 +units=m +no_defs")))
			assert(0);
		if (!(pj_latlong = pj_init_plus("+proj=longlat +datum=WGS84 +no_defs")))
			assert(0);
	}
#endif

	virtual void update(osg::NodeVisitor*, osg::Drawable* drawable) {
#ifdef  RANDOM_DATA
		float x_offset = 0.f, y_offset = 0.f;
		float v = rand() % 10;
		float theta = float(rand() % 471) / 100;
		x_offset = v * cos(theta - PI);
		y_offset = v * sin(theta);

		cur_pos = { pre_pos.x + x_offset, pre_pos.y + y_offset, pre_pos.z };

		osg::ref_ptr<osg::ShapeDrawable> shape = dynamic_cast<osg::ShapeDrawable*>(drawable);
		osg::ref_ptr<osg::Shape> s = new osg::Sphere(osg::Vec3(cur_pos.x, cur_pos.y, cur_pos.z), 50.f);
		shape->setColor(osg::Vec4(1.f, 0.f, 0.f, 1.f));
		shape->setShape(s);

		pre_pos = cur_pos;

		shape->dirtyBound();
		shape->dirtyDisplayList();
#else
		if (m_idx >= m_pos.size()) {
			return;
		}

		Pos cur = m_pos.at(m_idx);
		double x = cur.lon * DEG_TO_RAD;
		double y = cur.lat * DEG_TO_RAD;
		pj_transform(pj_latlong, pj_utm, 1, 1, &x, &y, NULL);

		osg::ref_ptr<osg::ShapeDrawable> shape = dynamic_cast<osg::ShapeDrawable*>(drawable);
		osg::ref_ptr<osg::Shape> s = new osg::Sphere(osg::Vec3(x + 10000, y, cur.h * 2 + 65.f), 25.f);
		shape->setColor(osg::Vec4(1.f, 0.f, 0.f, 1.f));
		shape->setShape(s);

		shape->dirtyBound();
		shape->dirtyDisplayList();

		m_idx++;
#endif
	}

#ifdef RANDOM_DATA
	osg::Vec3f pre_pos;
	osg::Vec3f cur_pos;
#else
	vector<Pos> m_pos;
	int m_idx;
	projPJ pj_utm, pj_latlong;
#endif
};
#endif

int main(int argc, char** argv) {
	osg::ArgumentParser arg(&argc, argv);
	arg.getApplicationUsage()->setApplicationName(arg.getApplicationName());
	arg.getApplicationUsage()->setDescription(arg.getApplicationName() + " is the standard OpenSceneGraph example which loads and visualises 3d models.");
	arg.getApplicationUsage()->setCommandLineUsage(arg.getApplicationName() + " [options] filename ...");

	osg::ref_ptr<osg::Group> root = new osg::Group;
	osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFiles(arg);
	terrain->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);
	root->addChild(terrain);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;

	// sphere
	//osg::ref_ptr<osg::ShapeDrawable> sph =
	//	new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(432371.688, 3385099.00, 450.f * 2 + 50.f), 25.f));
	//sph->setColor(osg::Vec4(1.f, 0.f, 0.f, 0.f));
	// sph->setDataVariance(osg::Object::DYNAMIC);

	// human
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	geometry->setDataVariance(osg::Object::DYNAMIC);

	// light off, from wherever the image looks the same
	geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array();
	v->setDataVariance(osg::Object::DYNAMIC);
	v->push_back(osg::Vec3(429320.13929, 3387250.20839, 1050.f));
	v->push_back(osg::Vec3(429920.13929, 3387250.20839, 1050.f));
	v->push_back(osg::Vec3(429920.13929, 3387250.20839, 1650.f));
	v->push_back(osg::Vec3(429320.13929, 3387250.20839, 1650.f));
	geometry->setVertexArray(v.get());

	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Y_AXIS);
	geometry->setNormalArray(normal.get());
	geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

	osg::ref_ptr<osg::Vec2Array> tcoords = new osg::Vec2Array;
	tcoords->push_back(osg::Vec2(0.0f, 0.0f));
	tcoords->push_back(osg::Vec2(1.0f, 0.0f));
	tcoords->push_back(osg::Vec2(1.0f, 1.0f));
	tcoords->push_back(osg::Vec2(0.0f, 1.0f));
	geometry->setTexCoordArray(0, tcoords.get());

	geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	s_image_red = osgDB::readImageFile("../data/human_red.png");
	s_image_blue = osgDB::readImageFile("../data/human_blue.png");
	s_image_yellow = osgDB::readImageFile("../data/human_yellow.png");

	s_image_red->scaleImage(512, 512, 1);
	s_image_blue->scaleImage(512, 512, 1);
	s_image_yellow->scaleImage(512, 512, 1);

	texture->setImage(s_image_red);
	geometry->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

	// blend for transparency
	geometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
	geometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	geode->addDrawable(geometry);
	// geode->addDrawable(sph);
	root->addChild(geode);

	osgViewer::Viewer viewer(arg);
	viewer.setSceneData(root);

	MotionParser mp;
	mp.parse("../data/motion_xy.csv");
	geometry->setUpdateCallback(new GeomUpdateCallback(mp.getPosition()));

	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
	viewer.addEventHandler(new osgViewer::ThreadingHandler);
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);
	viewer.addEventHandler(new osgViewer::LODScaleHandler);
	viewer.addEventHandler(new osgViewer::ScreenCaptureHandler);

	std::thread th(time_count);
	th.detach();

	viewer.getCamera()->setClearColor(osg::Vec4(0.5f, 0.5f, 0.5f, 1.f));
	viewer.realize();
	viewer.run();
	return EXIT_SUCCESS;
}