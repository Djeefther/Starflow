#ifndef STAR_FLOW_GAME_H_
#define STAR_FLOW_GAME_H_

//STD
#include <vector>
#include <utility>
#include <tuple>
#include <fstream>
#include <functional>
#include <map>

//MY
#include "direction.h"
#include "utils.h"


class StarFlowGame {
public:
	typedef std::vector < std::vector < std::tuple<int, Direction, Direction> > > board_type;
	typedef std::vector<Point> path_type;
	typedef std::vector<std::vector<Point>> set_path_type;
	static const size_t no_color = size_t(-1);
protected:
	int size_;
	board_type board_; //color,next_direction,last_direction
					   //color = 0 empty, color > 0 so the real color is minus 1
	std::vector<char> color_dictionary_;
	char no_color_dictionary_;//TODO
	std::vector<std::pair<Point, Point>> color_centers_; //
	std::vector<color_t> colors_;
	color_t color_no_color_;
public:
	StarFlowGame(int size = 0) {
		color_dictionary_ = { '#','%','@','*','$','&','O','X','?','+','!','=','~','[',']',';' };
		no_color_dictionary_ = '.';

		colors_ = { red,green,blue,yellow,orange,pink,ciano,white,brown,magenta,light_green,purple,light_blue,mustard,grey };
		color_no_color_ = black;

		resize(size);
	}

	void resize(int size) {
		size_ = size;
		board_.resize(size_);
		for (auto & i : board_)
			i.resize(size_, std::make_tuple(no_color, center, center));
	}

	inline auto& at_board(Point p) {
		return board_[p.y][p.x];
	}

	inline const auto& at_board(Point p) const {
		return board_[p.y][p.x];
	}

	Point color_begin(size_t i) {
		return color_centers_[i].first;
	}

	Point color_end(size_t i) {
		return color_centers_[i].second;
	}

	void add_color(Point p1, Point p2) {
		if (std::get<0>(at_board(p1)) != no_color || std::get<0>(at_board(p2)) != no_color) {
			throw std::exception("Not possible to add color");
		}

		int new_color = color_centers_.size();
		color_centers_.emplace_back(p1, p2);
		at_board(p1) = at_board(p2) = std::make_tuple(new_color, center, center);
	}

	size_t size() const {
		return size_;
	}

	void clear_points() {
		color_centers_.clear();

		for (auto& i : board_) {
			for (auto& j : i) {
				j = std::make_tuple(no_color, center, center);
			}
		}
	}

	void clear() {
		color_centers_.clear();
		board_.clear();
	}

	void print(std::ostream& stream = std::cout) {
		for (const auto& i : board_) {
			for (const auto& j : i) {
				if (std::get<0>(j) != no_color) {
					stream << color_dictionary_[std::get<0>(j)];
				}
				else {
					stream << no_color_dictionary_;
				}
			}
			stream << std::endl;
		}

		for (int i = 0; i < size_; i++)
			stream << "_";
		stream << "\n\n";
	}

	set_path_type path(int color) {
		std::vector<std::vector<Point>> paths;

		std::function<void(Point, Direction, int, Point)> recursive_path;
		recursive_path = [&paths, &recursive_path, color, this](Point current_point, Direction last_direction, int level, Point goal) {
			if (current_point == goal) {
				paths.emplace_back();
				Point p = current_point;
				Direction d;
				do {
					paths.back().push_back(p);
					//next point
					d = std::get<1>(board_[p.y][p.x]);
					p.x += j_direction[d];
					p.y += i_direction[d];
				} while (d != center);

				std::reverse(paths.back().begin(), paths.back().end());//the points was insert in the reverse order

				return;
			}

			for (int i = begin_non_center; i != end; i++) {
				Direction current_direction = (Direction)i;
				Point new_point = current_point;
				new_point.x += j_direction[current_direction];
				new_point.y += i_direction[current_direction];


				if (new_point.inside(0, 0, size_, size_)) {
					bool new_point_empty = std::get<0>(board_[new_point.y][new_point.x]) == no_color;
					bool new_point_goal = new_point == goal;

					if (new_point_empty || new_point_goal) {
						std::get<2>(board_[current_point.y][current_point.x]) = current_direction;
						auto old_value = at_board(new_point);
						at_board(new_point) = std::make_tuple(color, opposite(current_direction), center);
						recursive_path(new_point, current_direction, level + 1, goal);
						at_board(new_point) = old_value;
					}
				}
			}
		};

		auto current_point = color_centers_[color].first;
		auto goal = color_centers_[color].second;


		recursive_path(current_point, center, 0, goal);

		auto smaller_first = [](const auto& a, const auto& b) {
			return a.size() < b.size();
		};

		std::sort(paths.begin(), paths.end(), smaller_first);
		//std::cout << paths.size() << std::endl;
		//for (auto& i : paths) {
		//	for (auto& j : i) {
		//		std::cout << "(" << j.x << "," << j.y << ") ";
		//	}
		//	std::cout << std::endl;
		//	getchar();
		//}

		return paths;
	}

	size_t num_colors() const {
		return color_centers_.size();
	}

	enum Solved {
		not_solved, solved_incomplete, solved
	};

	Solved is_solved() {
		for (int i = 0; i < color_centers_.size(); i++) {
			auto begin_path = color_centers_[i].first;
			auto end_path = color_centers_[i].second;

			auto p = begin_path;

			while (true) {
				auto direction = std::get<1>(at_board(p));

				if (direction == center)
					break;

				p.x += j_direction[direction];
				p.y += i_direction[direction];
			}

			if (p != end_path) {
				return not_solved;
			}
		}

		for (auto i : board_) {
			for (auto j : i) {
				if (std::get<0>(j) == no_color) {
					return solved_incomplete;
				}
			}
		}

		return solved;
	}

	bool aplly_at_board(const std::vector<Point>& path, int color) {
		//look if every cell is free
		for (auto p : path) {
			if (std::get<0>(at_board(p)) != no_color && std::get<0>(at_board(p)) != (color)) {
				return false;
			}
		}
		//put the directions
		auto next_p = path[0];
		std::get<2>(at_board(next_p)) = center;

		for (int i = 0; i < path.size() - 1; i++) {
			auto p = next_p;
			next_p = path[i + 1];

			//color
			std::get<0>(at_board(p)) = color;

			//directions
			auto d = direction(next_p.y - p.y, next_p.x - p.x);
			std::get<1>(at_board(p)) = d;
			std::get<2>(at_board(next_p)) = opposite(d);
		}
		return true;
	};

	void remove_at_board(const std::vector<Point>& path) {
		std::get<1>(at_board(path.front())) = center;
		std::get<2>(at_board(path.front())) = center;

		std::get<1>(at_board(path.back())) = center;
		std::get<2>(at_board(path.back())) = center;

		for (int i = 1; i < path.size() - 1; i++) {
			auto p = path[i];
			at_board(p) = std::make_tuple(no_color, center, center);
		}
	};

	void set_board(const std::vector<std::string>& board) {
		std::map<char, Point> dictionary;
		Point  p(0, 0);

		size_t size = board.size();
		for (const auto& line : board) {
			size = std::max(line.size(), size);
		}

		this->clear();
		this->resize(size);

		for (const auto& line : board) {
			for (const auto& col : line) {
				if (col != '.' && col != ' ') {
					auto it = dictionary.find(col);

					if (it == dictionary.end()) {
						dictionary.emplace(col, p);
					}
					else {
						add_color(it->second, p);
						dictionary.erase(it);
					}
				}

				p.x++;
			}

			p.x = 0;
			p.y++;
		}
	}

	Solved try_solve() {
		std::vector<std::vector<std::vector<Point>>> colors_paths;
		size_t max_offset = 0;
		size_t prod_offset = 1;

		for (int i = 0; i < num_colors(); i++) {
			colors_paths.push_back(path(i));
			max_offset = std::max(colors_paths.back().size(), max_offset);
			prod_offset *= colors_paths.back().size();

			if (colors_paths.back().size() == 0) {
				return not_solved; //not possible to solve if one of the paths size is zero
			}

			std::cout << "-> " << colors_paths.back().size() << "\n";
		}
		std::cout << "!\n";

		PRINTLN_VALUE(prod_offset);

#if 1

		auto number_offset_to_offset = [](const std::vector<size_t>& number_offset, std::vector<size_t>& offset) {
			offset.clear();
			for (size_t i = 0; i < number_offset.size(); i++) {
				offset.resize(offset.size() + number_offset[i], i);
			}
		};

		auto number_offset_init = [this]() {
			return std::vector<size_t>(1, num_colors());
		};

		auto number_offset_next = [](std::vector<size_t>& number_offset) {
			size_t first_non_zero_it = 0;
			while (number_offset[first_non_zero_it] == 0)
				first_non_zero_it++;

			if (first_non_zero_it + 1 == number_offset.size()) {
				number_offset.push_back(0);
			}

			number_offset[first_non_zero_it]--;
			number_offset[first_non_zero_it + 1]++;
			std::swap(number_offset.front(), number_offset[first_non_zero_it]);
		};

		auto number_offset_is_end = [&max_offset, this](const std::vector<size_t>& number_offset) {
			return number_offset.size() > max_offset;
		};

#else

		auto number_offset_to_offset = [](const std::vector<size_t>& number_offset, std::vector<size_t>& offset) {
			offset = number_offset;
		};

		auto number_offset_init = [this]() {
			return std::vector<size_t>(num_colors(), 0);
		};

		auto number_offset_next = [&, this](std::vector<size_t>& number_offset) {
			for (size_t i = 0; i < number_offset.size(); i++) {
				number_offset[i]++;
				if (number_offset[i] >= colors_paths[i].size()) {
					number_offset[i] = 0;
				}
				else {
					break;
				}
			}
		};

		auto number_offset_is_end = [&colors_paths, this](const std::vector<size_t>& number_offset) {
			return number_offset.back() >= colors_paths.back().size();
		};

#endif

		//offset of ones and zeros //TOOD: all numbers possibilities
		std::vector<size_t> number_offset;
		std::vector<size_t> offset;
		std::vector<size_t> last_offset_used(num_colors(), size_t(-1));
		size_t tests = 1;
		size_t total_tests = std::pow(max_offset, num_colors());

		PRINTLN_VALUE(total_tests);
		PRINTLN_VALUE(max_offset);
		PRINTLN_VALUE(num_colors());

		for (number_offset = number_offset_init();
		!number_offset_is_end(number_offset);
			number_offset_next(number_offset))
		{
			number_offset_to_offset(number_offset, offset);

			//FPRINTLN_VALUE(f,number_offset);

			do { //permutation of offsets
				tests++;

				if (tests % 5000000 == 0) {
					double perc = 100 * double(tests) / total_tests;
					std::cout << tests << "\t" << perc << "%\n";
				}
				//FPRINT_VALUE(f,offset);

				//Checking if the offset vector apoints to valid paths in all colors
				bool invalid_offset = false;
				for (int color = 0; color < num_colors(); color++) {
					int current_offset = offset[color];
					if (current_offset >= colors_paths[color].size()) {
						//f << " invalid\n";
						invalid_offset = true;
						break;
					}
				}

				if (invalid_offset) {
					continue;
					//impossible to apply one of that paths, so try next iteration
				}

				//apply them
				int color = 0;
				bool all_colors = true;

				for (; color < num_colors(); color++) {
					size_t current_offset = offset[color];
					size_t current_last_offset_used = last_offset_used[color];
					size_t current_colors_path = colors_paths[color].size();


					if (current_last_offset_used < current_colors_path) {
						//Has an alaready applied path

						if (current_last_offset_used != current_offset) {
							//Applied with different path 
							remove_at_board(colors_paths[color][current_last_offset_used]);
							last_offset_used[color] = size_t(-1);
						}
						else {
							//Applied with the current path
							continue;//go to next iteration (next color)
						}
					}

					if (!aplly_at_board(colors_paths[color][current_offset], color)) {
						all_colors = false;
						break;//not possible to apply
					}
					else {
						//A new one it was applied
						last_offset_used[color] = current_offset;
					}
				}


				if (all_colors) {
					auto s = is_solved();
					if (s != not_solved) {
						PRINTLN_VALUE(tests);
						return s;
					}
				}
				//if not solved or not all colors	
			} while (std::next_permutation(offset.begin(), offset.end()));

		}

		//remove if left some of them
		for (int i = 0; i < last_offset_used.size(); i++) {
			size_t current_last_offset_used = last_offset_used[i];
			if (current_last_offset_used < colors_paths[i].size()) {
				//has a still aplied path
				remove_at_board(colors_paths[i][current_last_offset_used]);
			}
		}

		PRINTLN_VALUE(tests);
		return not_solved;
	}

	cv::Mat to_image(unsigned size_square = 30, unsigned pad = 2, color_t pad_color = brown) const {
		if (size_ == 0)
			return cv::Mat();//empty

		unsigned image_size = size_square*size_ + pad*(size_ + 1);
		cv::Mat image = cv::Mat(image_size, image_size, CV_8UC3, color_no_color_);

		//drawing vertical and horizontal lines
		cv::Rect horizontal_line(0, 0, image_size, pad);
		cv::Rect vertical_line(0, 0, pad, image_size);
		for (int i = 0; i < size_ + 1; i++)
		{
			cv::rectangle(image, horizontal_line, pad_color, CV_FILLED);
			cv::rectangle(image, vertical_line, pad_color, CV_FILLED);

			horizontal_line.y += pad + size_square;
			vertical_line.x += pad + size_square;
		}


		auto sub_square_ij = [=](int i, int j) {
			return cv::Rect((i*size_square) / 3, (j*size_square) / 3, size_square / 3, size_square / 3);
			//return cv::Rect(0, 0, 0, 0);
		};

		//offset of current_square
		cv::Point offset_center(size_square / 2, size_square / 2);
		cv::Rect offset_sub_square_up = sub_square_ij(1, 0);
		cv::Rect offset_sub_square_left = sub_square_ij(0, 1);
		cv::Rect offset_sub_square_right = sub_square_ij(2, 1);
		cv::Rect offset_sub_square_down = sub_square_ij(1, 2);
		cv::Rect offset_sub_square_center = sub_square_ij(1, 1);

		cv::Point current_square_top_left(pad, pad);

		for (int i = 0; i < size_; i++) {
			for (int j = 0; j < size_; j++) {
				auto current_square = board_[i][j];
				auto current_color = std::get<0>(current_square);
				if (current_color != no_color) {
					auto point_center = offset_center + current_square_top_left;
					auto sub_square_up = offset_sub_square_up + current_square_top_left;
					auto sub_square_left = offset_sub_square_left + current_square_top_left;
					auto sub_square_right = offset_sub_square_right + current_square_top_left;
					auto sub_square_down = offset_sub_square_down + current_square_top_left;
					auto sub_square_center = offset_sub_square_center + current_square_top_left;

					Direction directions[] = { std::get<1>(current_square), std::get<2>(current_square) };

					for (int i = 0; i < 2; i++) {
						switch (directions[i])
						{
						case up:
							cv::rectangle(image, sub_square_up, colors_[current_color], CV_FILLED);
							break;
						case down:
							cv::rectangle(image, sub_square_down, colors_[current_color], CV_FILLED);
							break;
						case left:
							cv::rectangle(image, sub_square_left, colors_[current_color], CV_FILLED);
							break;
						case right:
							cv::rectangle(image, sub_square_right, colors_[current_color], CV_FILLED);
							break;
						case center:
							cv::circle(image, point_center, size_square / 3, colors_[current_color], CV_FILLED);
							break;
						}
					}

					cv::rectangle(image, sub_square_center, colors_[current_color], CV_FILLED);
				}

				//
				current_square_top_left.x += size_square + pad;
			}
			current_square_top_left.y += size_square + pad;
			current_square_top_left.x = pad;
		}

		return image;
	}
	void imshow(const char * window_name = "") const {
		auto image = to_image();
		cv::imshow(window_name, image);
		cv::waitKey(1);
	}
};

void print_all_paths_files(StarFlowGame& sf);


#endif