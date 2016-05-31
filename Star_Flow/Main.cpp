//STD
#include <iostream>
#include <cstdio>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <fstream>
#include <string>
#include <random>
#include <queue>
#include <memory>
#include <time.h>
#include <stack>
#include <list>
#include <forward_list>

//CV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include<opencv2\imgproc\imgproc.hpp>

//MY
#include "starflow_set_of_games.h"
#include "direction.h"
#include "star_flow_game.h"
#include "genetic_algorithm.h"
#include "genetic_algorithm_starflowgame.h"


int main() {
	StarFlowGame sf;

	auto list = {7,9,10, 8,11,12 };
	for (int i : list) {
		std::string name = std::to_string(i);
		sf.set_board(StarflowSetOfGames::instance()->board(i));
		sf.imshow(name.c_str());
		genetic_algorithm_starflowgame_(sf, 1000, 80, 500, 0.1, 0.3, 0.7);
		sf.imshow(name.c_str());
	}
	std::cout << "END\n";
	char key = '\0';
	while (key != 'q') {
		key = cv::waitKey(1);
	}
	return 0;
}