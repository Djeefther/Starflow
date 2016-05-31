#ifndef UTILS_H_
#define UTILS_H_

//STD
#include <iostream>

//CV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include<opencv2\imgproc\imgproc.hpp>


#define PRINTLN_VALUE(a)	std::cout << (#a) << " = " << (a) << std::endl
#define PRINT_VALUE(a)		std::cout << (#a) << " = " << (a) << " "

#define FPRINTLN_VALUE(f,a)	(f) << (#a) << " = " << (a) << std::endl
#define FPRINT_VALUE(f,a)	(f) << (#a) << " = " << (a) << " "


struct Point {
	int x, y;
	Point() : x(0), y(0) {
	}
	Point(int x, int y) : x(x), y(y) {
	}
	Point(const Point& p) = default;
	~Point() = default;

	bool inside(int x_begin, int y_begin, int x_end, int y_end) const {
		return x_begin <= x && x < x_end && y_begin <= y && y < y_end;
	}

	bool operator==(const Point& p) const {
		return x == p.x && y == p.y;
	}

	bool operator!=(const Point& p) const {
		return x != p.x || y != p.y;
	}
	Point operator-(const Point& other) const {
		return Point(x - other.x, y - other.y);
	}
	Point operator+(const Point& other) const {
		return Point(x + other.x, y + other.y);
	}
};

std::ostream& operator<<(std::ostream& stream, const Point& p);

template<class Integral>
Integral sing(const Integral& c) {
	return c > 0 ? 1 : (c < 0 ? -1 : 0); //return 1, -1 or 0
}

template<class T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T>& t) {
	bool once = false;
	for (const auto & i : t) {
		stream << (once ? "," : "") << i;
		once = true;
	}
	return stream;
}


typedef cv::Vec3b color_t;//BGR

const color_t black(0, 0, 0);
const color_t white(255, 255, 255);
const color_t green(255, 0, 0);
const color_t blue(0, 255, 0);
const color_t red(0, 0, 255);
const color_t yellow(0, 255, 255);
const color_t orange(0, 127, 255);
const color_t ciano(255, 255, 0);
const color_t pink(203, 192, 255);
const color_t brown(0, 75, 150);
const color_t magenta(255, 0, 255);
const color_t light_green(0, 252, 124);
const color_t purple(128, 0, 128);
const color_t light_blue(230, 216, 173);
const color_t mustard(88, 219, 255);
const color_t grey(127, 127, 127);




#endif