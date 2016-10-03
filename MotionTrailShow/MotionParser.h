#pragma once

#include <vector>

using std::vector;

typedef enum SpeedState {
	CONSTANT,
	ACCELERATE,
	DECELERTATE
} SS;

typedef struct Position {
	float x;
	float y;
	int h;
	SS state;

	float operator-(const Position& rhs) {
		return sqrt((x - rhs.x) * (x - rhs.x) + (y - rhs.y) * (y - rhs.y));
	}
} Pos;

class MotionParser {
public:
	MotionParser() = default;
	~MotionParser();

	void parse(const char* filename);

	size_t getNPos();
	Pos getPosition(const int& idx);
	vector<Pos>& getPosition();
private:
	vector<Pos> m_pos;
};
