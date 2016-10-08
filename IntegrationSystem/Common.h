#pragma once
#include <QString>

namespace common{
	typedef enum SpeedState {
		CONSTANT,
		ACCELERATE,
		DECELERTATE
	}SS;

	typedef struct Position{
		float x;
		float y;
		float h;
		SS state;

		float operator-(const Position& rhs) {
			return sqrt((x - rhs.x) * (x - rhs.x) + (y - rhs.y) * (y - rhs.y));
		}
	}Pos;

	struct Proj {
		QString hostname;
		QString dbname;
		QString usrname;
		QString pwd;
		QString tablename;
		QString terrain_path;
	};

	struct Person {
		QString name;
		int age;
		int height;
		int weight;
	};

	struct Color {
		int r;
		int g;
		int b;
	};
}
