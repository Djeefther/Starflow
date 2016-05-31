#include "utils.h"

std::ostream& operator<<(std::ostream& stream, const Point& p) {
	return stream << "(" << p.x << "," << p.y << ")";
}
