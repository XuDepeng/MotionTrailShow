#include "MotionParser.h"

#include <fstream>
#include <string>

using std::ifstream;
using std::string;

MotionParser::~MotionParser() {
	if (m_pos.size()) {
		m_pos.clear();
		m_pos.shrink_to_fit();
	}
}

void MotionParser::parse(const char* filename) {
	ifstream is;
	is.open(filename, std::ios::in);
	if (!is.is_open()) {
		printf_s("cannot open file!\n");
		return;
	}

	string str;
	while (!is.eof()) {
		std::getline(is, str);
		int id = 0;
		float x = 0.f, y = 0.f;
		int h = 0;
		SS state = SS::CONSTANT;

		sscanf_s(str.c_str(), "%d,%d,%f,%f,%d", &id, &h, &x, &y, &state);
		Pos p{ x, y, h, state };
		m_pos.push_back(p);
	}

	is.close();
}

size_t MotionParser::getNPos() {
	return m_pos.size();
}

Pos MotionParser::getPosition(const int& idx) {
	return m_pos[idx];
}

vector<Pos>& MotionParser::getPosition() {
	return m_pos;
}