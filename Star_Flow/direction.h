#ifndef DIRECTION_H_
#define DIRECTION_H_

enum Direction : unsigned char {
	//valid values
	center = 0, up = 1, down = 2, left = 3, right = 4,
	//error values
	error = -1,
	//alias to for iteration
	begin = 0,
	begin_non_center = 1,
	end = 5
};

const int i_direction[] = { 0,-1,1,0,0 };
const int j_direction[] = { 0,0,0,-1,1 };

Direction opposite(Direction d);
Direction direction(int delta_i, int delta_j);



#endif