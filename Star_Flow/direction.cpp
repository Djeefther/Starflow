#include "direction.h"

Direction opposite(Direction d) {
	switch (d)
	{
	case up:
		return down;
	case down:
		return up;
	case left:
		return right;
	case right:
		return left;
	default:
		return d;
		break;
	}
}

Direction direction(int delta_i, int delta_j) {
	switch (delta_i)
	{
	case -1:
		switch (delta_j)
		{
		case 0:
			return up;
		default:
			return error;
		}
	case 0:
		switch (delta_j)
		{
		case -1:
			return left;
		case 0:
			return center;
		case 1:
			return right;
		default:
			return error;
		}
	case 1:
		switch (delta_j)
		{
		case 0:
			return down;
		default:
			return error;
		}
	default:
		return error;
	}
}