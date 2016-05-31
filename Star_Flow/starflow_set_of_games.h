#ifndef STARFLOW_SET_OF_GAMES_H_
#define STARFLOW_SET_OF_GAMES_H_

#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <iostream>

/**
	A singleton with a set of games organized by size
*/
class StarflowSetOfGames {
public:
	typedef std::vector<std::string> board_type;
protected:
	static StarflowSetOfGames *s_instance;

	size_t num_boards_ = 0;
	std::map<size_t, std::vector<board_type>> boards_;
	mutable std::default_random_engine rgenerator_;
	
	StarflowSetOfGames();
public:
	static const size_t random = size_t(-1);

	static StarflowSetOfGames* instance();

	~StarflowSetOfGames() = default;
	const board_type& board(size_t n_board) const; //any_size
	const board_type& board(size_t size, size_t n_board) const;
	void add_board(const board_type& board);
	size_t num_boards() const;
	size_t num_boards(size_t size) const;
}; // StarflowSetOfGames;


#endif