#ifndef GENETIC_ALGORITHM_STARFLOWGAME_H_
#define GENETIC_ALGORITHM_STARFLOWGAME_H_

#include "star_flow_game.h"

/**
Wrapper that calls genetic_algorithm. Create all the operators to solve a StarFlowGame
extra_probability: When make new directions proability given extra to the directions that goes straight to the goal
mutation_probability: in the generations the probability of been mutate
initialize_mutation_probability: sucessive tests are made for that probability between 0 and 1, each one aproved a mutation is done
*/
void genetic_algorithm_starflowgame(StarFlowGame& starflow, size_t population, size_t keep_population, size_t generations,
	double extra_probability, double mutation_probability, double initialize_mutation_probability) {


	const size_t num_colors = starflow.num_colors();
	const size_t num_squares = starflow.size() * starflow.size();
	const size_t optimal = num_colors * num_squares + num_squares;

	struct Solution {
		StarFlowGame sf;
		std::vector<StarFlowGame::path_type> paths_applied;
		//Evalutions is based in this two intergers bellow that are computer by count_square_and_colors
		size_t squares_used;
		size_t colors_applied;
		//They are keeped to do not recount all time

		Solution() {
			count_squares_and_colors();
		}

		Solution(const StarFlowGame& sf) : sf(sf) {
			//paths_status.resize(sf.num_colors(), not_applied);
			paths_applied.resize(sf.num_colors());
			count_squares_and_colors();
		}

		Solution& operator=(const Solution&) = default;

		//to avaliate correctly this function should be called always something was modifed
		void count_squares_and_colors() {
			Point p;
			squares_used = 0;
			for (p.y = 0; p.y < sf.size(); p.y++) {
				for (p.x = 0; p.x < sf.size(); p.x++) {
					if (std::get<0>(sf.at_board(p)) != StarFlowGame::no_color) {
						squares_used++;
					}
				}
			}

			colors_applied = 0;
			for (int i = 0; i < sf.num_colors(); i++) {
				if (paths_applied[i].empty() == false) {
					colors_applied++;
				}
			}
		}
		/*
		extra_probability should be between 0 and 1 and it is the extra probability given in the directions in straight line to the goal
		*/
		void remove_path(const size_t color) {
			if (paths_applied[color].empty()) {
				throw std::exception("do not have a color");
			}

			sf.remove_at_board(paths_applied[color]);
			paths_applied[color].clear();
			count_squares_and_colors();
		}

		void make_a_random_path(const size_t color, const double extra_proability, std::default_random_engine& rgenerator) {
			//An djisktra algorithm with weight as the number of colors removed by this insertion (optimal is zero)
			//the orientation of the new path is random, but with extra_probability applied to make a straight line to the goal
			auto begin = sf.color_begin(color);
			auto end = sf.color_end(color);

			struct Location {
				Point p;
				std::shared_ptr<Location> last_location;
				size_t weight;
				size_t removed_color;//StarFlowGame::no_color if no color removed

				bool operator<(const Location& other) const {
					return weight > other.weight;//oposite to make smaller firt in priority_queue
				}

				Location() = default;
				Location(const Location&) = default;
				Location(Point p) : p(p), weight(0), removed_color(size_t(-1)) {
				}
				Location(Point p, const std::shared_ptr<Location>& last_location) :
					p(p), last_location(last_location), weight(last_location->weight), removed_color(StarFlowGame::no_color) {
				}

				void remove_color(size_t color) {
					//if it is a no_color
					if (color == StarFlowGame::no_color) {
						if (removed_color != StarFlowGame::no_color) {
							weight--;
							removed_color = color;
						}
						return;
					}

					//looking if already removed:
					for (auto l = last_location; l != nullptr; l = l->last_location) {
						if (l->removed_color == color) {
							return;//already removed
						}
					}

					//add this color
					if (removed_color == StarFlowGame::no_color) {
						weight++;
					}
					removed_color = color;
				}
			};

			const double common_probability = 1.0 - extra_proability;
			const Direction directions[4] = { up,down,left,right };


			std::priority_queue<Location> queue;
			queue.push(Location(begin));
			std::vector<std::vector<bool>> places_already_passed;

			places_already_passed.resize(sf.size());
			for (auto& i : places_already_passed) {
				i.resize(sf.size(), false);
			}
			places_already_passed[begin.y][begin.x] = true;


			while (!queue.empty()) {
				std::shared_ptr<Location> current_location = std::make_shared<Location>(queue.top());
				queue.pop();
				auto p = current_location->p;

				if (p == end) {//found solution!
							   //Make path and remove colors if need
					StarFlowGame::path_type path;

					while (current_location) {
						path.push_back(current_location->p);

						//remove color if need
						if (current_location->removed_color != StarFlowGame::no_color) {
							//removing from board and list of paths
							sf.remove_at_board(paths_applied[current_location->removed_color]);
							paths_applied[current_location->removed_color].clear();
						}
						current_location = current_location->last_location;
					}

					std::reverse(path.begin(), path.end());

					//aply new path
					sf.aplly_at_board(path, color);
					paths_applied[color] = std::move(path);
					//recount
					count_squares_and_colors();
					return;
				}

				//make probability
				Direction available_directions[4];
				Direction order_taken_directions[4];
				double prob_directions[4] = { 0,0,0,0 };

				//Extra probability
				Point dif = end - p;
				Point dir(sing(dif.x), sing(dif.y));//x and y just with -1,0 or 1

				if (dif.x && dif.y) { // diagonal 
					Direction d1 = direction(dir.y, 0);
					Direction d2 = direction(0, dir.x);

					prob_directions[d1 - 1] = extra_proability / 2.0;
					prob_directions[d2 - 1] = extra_proability / 2.0;

					//int sum = std::abs(dif.y) + std::abs(dif.x);
					//prob_directions[d1 - 1] = extra_proability * std::abs(dif.y) / sum;
					//prob_directions[d2 - 1] = extra_proability * std::abs(dif.x) / sum;
				}
				else { //directly
					Direction d = direction(dir.y, dir.x);
					prob_directions[d - 1] = extra_proability;
				}

				//count availables:
				size_t count_available_directions = 0;

				for (size_t i = 0; i < 4; i++) {
					auto next_p = p;
					next_p.x += j_direction[directions[i]];
					next_p.y += i_direction[directions[i]];

					bool available = next_p.inside(0, 0, sf.size(), sf.size());

					if (available) {//inside of board, cheking if it is a begin or end point
						available = available && !places_already_passed[next_p.y][next_p.x];//if already visiti, make false

						if (next_p != end && std::get<0>(sf.at_board(next_p)) != StarFlowGame::no_color) {
							//if they have a color and it is not the goal, should not be a center dot (begin or end) from other color.
							//Because this color could not be removed to this path pass
							available = available && std::get<1>(sf.at_board(next_p)) != center;
							available = available && std::get<2>(sf.at_board(next_p)) != center;
						}
					}

					if (available) {
						if (prob_directions[i] < 0) {
							int a = 0;
						}

						available_directions[count_available_directions] =
							directions[i];
						prob_directions[count_available_directions] =
							prob_directions[i];

						count_available_directions++;
					}
				}

				//atribute common probability
				double total_prob = 0;
				for (size_t i = 0; i < count_available_directions; i++) {
					prob_directions[i] += common_probability / count_available_directions;
					total_prob += prob_directions[i];
				}

				//random take directions
				for (size_t i = 0; i < count_available_directions; i++) {
					std::uniform_real_distribution<double> random_direction(0, total_prob);
					double v = random_direction(rgenerator);

					size_t j;
					for (j = 0; j < count_available_directions - 1; j++) {
						v -= prob_directions[j];
						if (v <= 0)
							break;
					}

					//take the j to i
					order_taken_directions[i] = available_directions[j];
					total_prob -= prob_directions[j];
					prob_directions[j] = 0;
				}

				//Going to next points
				for (size_t i = 0; i < count_available_directions; i++) {
					auto next_p = p;
					next_p.x += j_direction[order_taken_directions[i]];
					next_p.y += i_direction[order_taken_directions[i]];

					Location next_location(next_p, current_location);
					auto color_next_p = std::get<0>(sf.at_board(next_p));
					if (color_next_p != color) {
						next_location.remove_color(color_next_p);//not remove itself
					}
					queue.push(next_location);

					places_already_passed[next_p.y][next_p.x] = true;
				}
			}

			//If not a solution was found throw a exception, if was found a return was it called
			throw std::exception("no path found!!");
		}
	};

	//shuffle_colors is a shared vector used by some elements in this genetic_algorithm implementation
	//each function should take him and use std::shuffle function to take a new random list of color
	std::vector<size_t> shuffle_colors;
	for (size_t i = 0; i < num_colors; i++) {
		shuffle_colors.push_back(i);
	}

	auto mutate = [=](auto& rgenerator, auto& ind) {
#define mutate_just_add 1

#if mutate_just_add
		size_t colors_available = num_colors - ind.colors_applied;
		if (!colors_available && ind.squares_used == num_squares) {
			//optimal solution, don do nothing
			return;
		}
		else if (!colors_available) {
			//All colors but not complete, remove one of the colors
			std::uniform_int_distribution<size_t> rnd(0, num_colors - 1);
			ind.remove_path(rnd(rgenerator));
			return;
		}

		std::uniform_int_distribution<size_t> rnd(0, colors_available - 1);
#else
		std::uniform_int_distribution<size_t> rnd(0, ind.sf.num_colors() - 1);
#endif

		size_t color = rnd(rgenerator);

#if mutate_just_add
		for (size_t i = 0; i < num_colors; i++) {
			if (ind.paths_applied[i].empty()) {
				if (color == 0) {
					ind.make_a_random_path(i, extra_probability, rgenerator);
					return;
				}
				else {
					color--;
				}
			}
		}
#else
		if (ind.paths_applied[color].empty()) {
			ind.make_a_random_path(color, extra_probability, rgenerator);
		}
		else {
			ind.remove_path(color);
		}
#endif
	};

	auto is_optimal = [=](int eva) {
		return eva >= optimal;
	};

	auto crossover = [=, &shuffle_colors](auto& rgenerator,
		const Solution& parent1, const Solution& parent2,
		Solution& child1, Solution& child2) {

		std::uniform_int_distribution<int> random_bit(0, 1);

		child1 = child2 = Solution(starflow);

		//crossover choosing gens
		for (size_t i = 0; i < num_colors; i++) {
			bool b = random_bit(rgenerator);

			if (b) {
				child1.paths_applied[i] = parent1.paths_applied[i];
				child2.paths_applied[i] = parent2.paths_applied[i];
			}
			else {
				child2.paths_applied[i] = parent1.paths_applied[i];
				child1.paths_applied[i] = parent2.paths_applied[i];
			}
		}


		//Check if the genes are possible and apply them into the board_.
		//In a random order is going to apply all color and 
		//when is not possible to apply drop that path

		//initializing shufle colors
		std::vector<size_t> shuffle_colors;
		shuffle_colors.reserve(num_colors);
		for (size_t i = 0; i < num_colors; i++) {
			shuffle_colors.push_back(i);
		}

		//Lambda that do that to one child
		auto check_gen_and_apply = [&shuffle_colors, &rgenerator](Solution& child) {
			std::shuffle(shuffle_colors.begin(), shuffle_colors.end(), rgenerator);//shuffle in fact

			for (auto color : shuffle_colors) {
				if (child.paths_applied[color].empty() == false) {
					if (!child.sf.aplly_at_board(child.paths_applied[color], color)) {
						child.paths_applied[color].clear();//not possible to apply,remove
					}
				}
			}
			child.count_squares_and_colors();//recount
		};

		//apply to child1
		check_gen_and_apply(child1);
		//apply to child2
		check_gen_and_apply(child2);
	};

	auto evaluate = [=](const auto& ind) {
		return ind.colors_applied * num_squares + ind.squares_used;
	};

	auto initialize_population = [=, &shuffle_colors](auto& rgenerator) {
		Solution s(starflow);
		//n mutations with probability of P(n) = p^n * (1-p)
		//where p is initialize_mutation_probability capture by this lambda, sucessiful mutation while aproved by p probability

		std::uniform_real_distribution<double> rnd(0, 1);

		std::shuffle(shuffle_colors.begin(), shuffle_colors.end(), rgenerator);

		for (size_t i = 0; i < num_colors; i++) {
			if (rnd(rgenerator) < initialize_mutation_probability) {
				s.make_a_random_path(shuffle_colors[i], extra_probability, rgenerator);
			}
			else {
				break;
			}
		}

		return s;
	};


	auto show_population_it = [](auto it_begin, auto it_end, size_t cols, bool imwrite, bool imshow) {
		size_t num = it_end - it_begin;
		size_t rows = num / cols + (num % cols ? 1 : 0);
		size_t pad = 15;
		cols = cols > num ? num : cols;//if we have too many cols

		cv::Size individual_size = it_begin->sf.to_image(24, 1).size();
		cv::Size total_size(individual_size.width * cols + pad * (cols - 1),
			individual_size.height * rows + pad * (rows - 1));

		cv::Mat final_image = cv::Mat::zeros(total_size, CV_8UC3);

		cv::Point p(0, 0);

		for (auto it = it_begin; it != it_end; it++) {
			auto current_image = it->sf.to_image(24, 1);
			cv::Rect current_place(p, individual_size);
			current_image.copyTo(cv::Mat(final_image, current_place));

			//next p
			p.x += individual_size.width + pad;
			if (p.x >= total_size.width) {
				p.x = 0;
				p.y += individual_size.height + pad;
			}
		}

		static int i = 0;
		if (imwrite) {
			cv::imwrite(std::to_string(i++) + ".png", final_image);
		}
		if (imshow) {
			cv::imshow("", final_image);
			cv::waitKey(1);
		}
	};

	int generation = 0;
	auto show_population = [=, &generation](const std::vector<Solution>& v) {
		//show_population_it(v.begin(), v.end(), sqrt(v.size()) + 0.5, true, false);
		show_population_it(v.begin(), v.begin() + 12, 4, false, true);//4x3 is good in screen in full resuliton

		auto best_result = evaluate(v.front());
		double avg_result = 0;
		auto worst_result = evaluate(v.back());
		for_each(v.begin(), v.end(), [&](const auto& v) {avg_result += evaluate(v); });
		avg_result /= v.size();

		PRINTLN_VALUE(generation);
		PRINT_VALUE(best_result);
		PRINTLN_VALUE(optimal);
		PRINTLN_VALUE(avg_result);
		PRINTLN_VALUE(worst_result);
		std::cout << "\n";
		generation++;
	};

	auto final_population = genetic_algorithm(population, keep_population, generations, initialize_population, evaluate,
		is_optimal, crossover, mutation_probability, mutate, show_population);

	if (final_population.front().sf.is_solved() == StarFlowGame::solved) {
		starflow = final_population.front().sf; //if best results is solved, assigned
	}
}

void genetic_algorithm_starflowgame_(StarFlowGame& starflow, size_t population, size_t keep_population, size_t generations,
	double extra_probability, double mutation_probability, double initialize_mutation_probability, bool imwrite = true, bool imshow = true) {

	const size_t num_colors = starflow.num_colors();
	const size_t num_squares = starflow.size() * starflow.size();
	const size_t optimal = num_colors * num_squares + num_squares;

	struct Solution {
		StarFlowGame sf;
		std::vector<StarFlowGame::path_type> paths_applied;
		//Evalutions is based in this two intergers bellow that are computer by count_square_and_colors
		size_t squares_used;
		size_t colors_applied;
		//They are keeped to do not recount all time

		std::shared_ptr<const Solution> parent1;
		std::shared_ptr<const Solution> parent2;//in case of mutation, just parent1 is different them nullptr

		Solution() {
			count_squares_and_colors();
		}

		Solution(const StarFlowGame& sf) : sf(sf) {
			//paths_status.resize(sf.num_colors(), not_applied);
			paths_applied.resize(sf.num_colors());
			count_squares_and_colors();
		}

		Solution& operator=(const Solution&) = default;

		//to avaliate correctly this function should be called always something was modifed
		void count_squares_and_colors() {
			Point p;
			squares_used = 0;
			for (p.y = 0; p.y < sf.size(); p.y++) {
				for (p.x = 0; p.x < sf.size(); p.x++) {
					if (std::get<0>(sf.at_board(p)) != StarFlowGame::no_color) {
						squares_used++;
					}
				}
			}

			colors_applied = 0;
			for (int i = 0; i < sf.num_colors(); i++) {
				if (paths_applied[i].empty() == false) {
					colors_applied++;
				}
			}
		}
		/*
		extra_probability should be between 0 and 1 and it is the extra probability given in the directions in straight line to the goal
		*/
		void remove_path(const size_t color) {
			if (paths_applied[color].empty()) {
				throw std::exception("do not have a color");
			}

			sf.remove_at_board(paths_applied[color]);
			paths_applied[color].clear();
			count_squares_and_colors();
		}

		void make_a_random_path(const size_t color, const double extra_proability, std::default_random_engine& rgenerator) {
			//An djisktra algorithm with weight as the number of colors removed by this insertion (optimal is zero)
			//the orientation of the new path is random, but with extra_probability applied to make a straight line to the goal
			auto begin = sf.color_begin(color);
			auto end = sf.color_end(color);

			struct Location {
				Point p;
				std::shared_ptr<Location> last_location;
				size_t weight;
				size_t removed_color;//StarFlowGame::no_color if no color removed

				bool operator<(const Location& other) const {
					return weight > other.weight;//oposite to make smaller firt in priority_queue
				}

				Location() = default;
				Location(const Location&) = default;
				Location(Point p) : p(p), weight(0), removed_color(size_t(-1)) {
				}
				Location(Point p, const std::shared_ptr<Location>& last_location) :
					p(p), last_location(last_location), weight(last_location->weight), removed_color(StarFlowGame::no_color) {
				}

				void remove_color(size_t color) {
					//if it is a no_color
					if (color == StarFlowGame::no_color) {
						if (removed_color != StarFlowGame::no_color) {
							weight--;
							removed_color = color;
						}
						return;
					}

					//looking if already removed:
					for (auto l = last_location; l != nullptr; l = l->last_location) {
						if (l->removed_color == color) {
							return;//already removed
						}
					}

					//add this color
					if (removed_color == StarFlowGame::no_color) {
						weight++;
					}
					removed_color = color;
				}
			};

			const double common_probability = 1.0 - extra_proability;
			const Direction directions[4] = { up,down,left,right };


			std::priority_queue<Location> queue;
			queue.push(Location(begin));
			std::vector<std::vector<bool>> places_already_passed;

			places_already_passed.resize(sf.size());
			for (auto& i : places_already_passed) {
				i.resize(sf.size(), false);
			}
			places_already_passed[begin.y][begin.x] = true;


			while (!queue.empty()) {
				std::shared_ptr<Location> current_location = std::make_shared<Location>(queue.top());
				queue.pop();
				auto p = current_location->p;

				if (p == end) {//found solution!
							   //Make path and remove colors if need
					StarFlowGame::path_type path;

					while (current_location) {
						path.push_back(current_location->p);

						//remove color if need
						if (current_location->removed_color != StarFlowGame::no_color) {
							//removing from board and list of paths
							sf.remove_at_board(paths_applied[current_location->removed_color]);
							paths_applied[current_location->removed_color].clear();
						}
						current_location = current_location->last_location;
					}

					std::reverse(path.begin(), path.end());

					//aply new path
					sf.aplly_at_board(path, color);
					paths_applied[color] = std::move(path);
					//recount
					count_squares_and_colors();
					return;
				}

				//make probability
				Direction available_directions[4];
				Direction order_taken_directions[4];
				double prob_directions[4] = { 0,0,0,0 };

				//Extra probability
				Point dif = end - p;
				Point dir(sing(dif.x), sing(dif.y));//x and y just with -1,0 or 1

				if (dif.x && dif.y) { // diagonal 
					Direction d1 = direction(dir.y, 0);
					Direction d2 = direction(0, dir.x);

					prob_directions[d1 - 1] = extra_proability / 2.0;
					prob_directions[d2 - 1] = extra_proability / 2.0;

					//int sum = std::abs(dif.y) + std::abs(dif.x);
					//prob_directions[d1 - 1] = extra_proability * std::abs(dif.y) / sum;
					//prob_directions[d2 - 1] = extra_proability * std::abs(dif.x) / sum;
				}
				else { //directly
					Direction d = direction(dir.y, dir.x);
					prob_directions[d - 1] = extra_proability;
				}

				//count availables:
				size_t count_available_directions = 0;

				for (size_t i = 0; i < 4; i++) {
					auto next_p = p;
					next_p.x += j_direction[directions[i]];
					next_p.y += i_direction[directions[i]];

					bool available = next_p.inside(0, 0, sf.size(), sf.size());

					if (available) {//inside of board, cheking if it is a begin or end point
						available = available && !places_already_passed[next_p.y][next_p.x];//if already visiti, make false

						if (next_p != end && std::get<0>(sf.at_board(next_p)) != StarFlowGame::no_color) {
							//if they have a color and it is not the goal, should not be a center dot (begin or end) from other color.
							//Because this color could not be removed to this path pass
							available = available && std::get<1>(sf.at_board(next_p)) != center;
							available = available && std::get<2>(sf.at_board(next_p)) != center;
						}
					}

					if (available) {
						if (prob_directions[i] < 0) {
							int a = 0;
						}

						available_directions[count_available_directions] =
							directions[i];
						prob_directions[count_available_directions] =
							prob_directions[i];

						count_available_directions++;
					}
				}

				//atribute common probability
				double total_prob = 0;
				for (size_t i = 0; i < count_available_directions; i++) {
					prob_directions[i] += common_probability / count_available_directions;
					total_prob += prob_directions[i];
				}

				//random take directions
				for (size_t i = 0; i < count_available_directions; i++) {
					std::uniform_real_distribution<double> random_direction(0, total_prob);
					double v = random_direction(rgenerator);

					size_t j;
					for (j = 0; j < count_available_directions - 1; j++) {
						v -= prob_directions[j];
						if (v <= 0)
							break;
					}

					//take the j to i
					order_taken_directions[i] = available_directions[j];
					total_prob -= prob_directions[j];
					prob_directions[j] = 0;
				}

				//Going to next points
				for (size_t i = 0; i < count_available_directions; i++) {
					auto next_p = p;
					next_p.x += j_direction[order_taken_directions[i]];
					next_p.y += i_direction[order_taken_directions[i]];

					Location next_location(next_p, current_location);
					auto color_next_p = std::get<0>(sf.at_board(next_p));
					if (color_next_p != color) {
						next_location.remove_color(color_next_p);//not remove itself
					}
					queue.push(next_location);

					places_already_passed[next_p.y][next_p.x] = true;
				}
			}

			//If not a solution was found throw a exception, if was found a return was it called
			throw std::exception("no path found!!");
		}
	};

	//shuffle_colors is a shared vector used by some elements in this genetic_algorithm implementation
	//each function should take him and use std::shuffle function to take a new random list of color
	std::vector<size_t> shuffle_colors;
	for (size_t i = 0; i < num_colors; i++) {
		shuffle_colors.push_back(i);
	}

	auto mutate = [=](auto& rgenerator, auto& ind) {
		//makes notes about mutatin in parents
		auto parent = ind;
		ind = std::make_shared<Solution>(*parent);//copy
		ind->parent1 = parent;
		ind->parent2 = nullptr;

#define mutate_just_add 1

#if mutate_just_add
		size_t colors_available = num_colors - ind->colors_applied;
		if (!colors_available && ind->squares_used == num_squares) {
			//optimal solution, don do nothing
			return;
		}
		else if (!colors_available) {
			//All colors but not complete, remove one of the colors
			std::uniform_int_distribution<size_t> rnd(0, num_colors - 1);
			ind->remove_path(rnd(rgenerator));
			return;
		}

		std::uniform_int_distribution<size_t> rnd(0, colors_available - 1);
#else
		std::uniform_int_distribution<size_t> rnd(0, ind->sf.num_colors() - 1);
#endif

		size_t color = rnd(rgenerator);

#if mutate_just_add
		for (size_t i = 0; i < num_colors; i++) {
			if (ind->paths_applied[i].empty()) {
				if (color == 0) {
					ind->make_a_random_path(i, extra_probability, rgenerator);
					return;
				}
				else {
					color--;
				}
			}
		}
#else
		if (ind->paths_applied[color].empty()) {
			ind->make_a_random_path(color, extra_probability, rgenerator);
		}
		else {
			ind->remove_path(color);
		}
#endif
	};

	auto is_optimal = [=](int eva) {
		return eva >= optimal;
	};

	auto crossover = [=, &shuffle_colors](auto& rgenerator,
		const std::shared_ptr<const Solution>& parent1, const std::shared_ptr<const Solution>& parent2,
		std::shared_ptr<Solution>& child1, std::shared_ptr<Solution>& child2) {

		std::uniform_int_distribution<int> random_bit(0, 1);

		child1 = std::make_shared<Solution>(starflow);
		child2 = std::make_shared<Solution>(starflow);

		child1->parent1 = child2->parent1 = parent1;
		child1->parent2 = child2->parent2 = parent2;

		//crossover choosing gens
		for (size_t i = 0; i < num_colors; i++) {
			bool b = random_bit(rgenerator);

			if (b) {
				child1->paths_applied[i] = parent1->paths_applied[i];
				child2->paths_applied[i] = parent2->paths_applied[i];
			}
			else {
				child2->paths_applied[i] = parent1->paths_applied[i];
				child1->paths_applied[i] = parent2->paths_applied[i];
			}
		}


		//Check if the genes are possible and apply them into the board_.
		//In a random order is going to apply all color and 
		//when is not possible to apply drop that path

		//initializing shufle colors
		std::vector<size_t> shuffle_colors;
		shuffle_colors.reserve(num_colors);
		for (size_t i = 0; i < num_colors; i++) {
			shuffle_colors.push_back(i);
		}

		//Lambda that do that to one child
		auto check_gen_and_apply = [&shuffle_colors, &rgenerator](std::shared_ptr<Solution>& child) {
			std::shuffle(shuffle_colors.begin(), shuffle_colors.end(), rgenerator);//shuffle in fact

			for (auto color : shuffle_colors) {
				if (child->paths_applied[color].empty() == false) {
					if (!child->sf.aplly_at_board(child->paths_applied[color], color)) {
						child->paths_applied[color].clear();//not possible to apply,remove
					}
				}
			}
			child->count_squares_and_colors();//recount
		};

		//apply to child1
		check_gen_and_apply(child1);
		//apply to child2
		check_gen_and_apply(child2);
	};

	auto evaluate = [=](const auto& ind) {
		return ind->colors_applied * num_squares + ind->squares_used;
	};

	auto initialize_population = [=, &shuffle_colors](auto& rgenerator) {
		auto s = std::make_shared<Solution>(starflow);
		//n mutations with probability of P(n) = p^n * (1-p)
		//where p is initialize_mutation_probability capture by this lambda, sucessiful mutation while aproved by p probability

		std::uniform_real_distribution<double> rnd(0, 1);

		std::shuffle(shuffle_colors.begin(), shuffle_colors.end(), rgenerator);

		for (size_t i = 0; i < num_colors; i++) {
			if (rnd(rgenerator) < initialize_mutation_probability) {
				s->make_a_random_path(shuffle_colors[i], extra_probability, rgenerator);
			}
			else {
				break;
			}
		}

		return s;
	};

	auto show_population_it = [](auto it_begin, auto it_end, size_t cols, bool imwrite, bool imshow) {
		size_t num = it_end - it_begin;
		size_t rows = num / cols + (num % cols ? 1 : 0);
		size_t pad = 15;
		cols = cols > num ? num : cols;//if we have too many cols

		cv::Size individual_size = (*it_begin)->sf.to_image(24, 1).size();
		cv::Size total_size(individual_size.width * cols + pad * (cols - 1),
			individual_size.height * rows + pad * (rows - 1));

		cv::Mat final_image = cv::Mat::zeros(total_size, CV_8UC3);

		cv::Point p(0, 0);

		for (auto it = it_begin; it != it_end; it++) {
			auto current_image = (*it)->sf.to_image(24, 1);
			cv::Rect current_place(p, individual_size);
			current_image.copyTo(cv::Mat(final_image, current_place));

			//next p
			p.x += individual_size.width + pad;
			if (p.x >= total_size.width) {
				p.x = 0;
				p.y += individual_size.height + pad;
			}
		}

		static int i = 0;
		if (imwrite) {
			cv::imwrite(std::to_string(i++) + ".png", final_image);
		}
		if (imshow) {
			cv::imshow("", final_image);
			cv::waitKey(1);
		}
	};

	auto show_population = [=](const std::vector<std::shared_ptr<Solution>>& v) {
		show_population_it(v.begin(), v.end(), sqrt(v.size()) + 0.5, true, false);
		show_population_it(v.begin(), v.begin() + 12, 4, false, true);//4x3 is good in screen in full resuliton

		auto best_result = evaluate(v.front());
		double avg_result = 0;
		auto worst_result = evaluate(v.back());
		for_each(v.begin(), v.end(), [&](const auto& v) {avg_result += evaluate(v); });
		avg_result /= v.size();

		static int generation = 0;
		PRINTLN_VALUE(generation);
		PRINT_VALUE(best_result);
		PRINTLN_VALUE(optimal);
		PRINTLN_VALUE(avg_result);
		PRINTLN_VALUE(worst_result);
		std::cout << "\n";
		generation++;
	};

	auto final_population = genetic_algorithm(population, keep_population, generations, initialize_population, evaluate,
		is_optimal, crossover, mutation_probability, mutate, show_population);

	if (final_population.front()->sf.is_solved() == StarFlowGame::solved) {
		starflow = final_population.front()->sf; //if best results is solved, assigned
	}

	final_population.resize(1);

	std::stack<std::shared_ptr<const Solution>> stack;
	std::shared_ptr<const Solution> p = final_population.front();
	while (true) {
		cv::Mat current = p->sf.to_image();
		cv::Mat parent1, parent2;
		cv::Mat total = cv::Mat::zeros(2 * current.rows, 2 * current.cols, current.type());

		cv::Rect r[2][2];
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				r[i][j] = cv::Rect(j*current.cols, i*current.rows, current.cols, current.rows);
			}
		}

		current.copyTo(cv::Mat(total, r[0][0]));

		if (p->parent1) {
			parent1 = p->parent1->sf.to_image();
			parent1.copyTo(cv::Mat(total, r[1][0]));
		}
		if (p->parent2) {
			parent2 = p->parent2->sf.to_image();
			parent2.copyTo(cv::Mat(total, r[1][1]));
		}

		cv::imshow("", total);

		bool continue_ = true;
		while (continue_) {
			auto c = cv::waitKey(1);

			continue_ = false;
			if (c == '1' && p->parent1) {
				stack.push(p);
				p = p->parent1;
			}
			else if (c == '2' && p->parent2) {
				stack.push(p);
				p = p->parent2;
			}
			else if (c == '0' && !stack.empty()) {
				p = stack.top();
				stack.pop();
			}
			else {
				continue_ = true;
			}
		}

	}


}

#endif