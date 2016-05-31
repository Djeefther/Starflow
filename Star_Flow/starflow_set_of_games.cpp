#include "starflow_set_of_games.h"

StarflowSetOfGames * StarflowSetOfGames::s_instance = nullptr;

StarflowSetOfGames * StarflowSetOfGames::instance()
{
	if (s_instance == nullptr) {
		s_instance = new StarflowSetOfGames();
	}
	return s_instance;
}

const StarflowSetOfGames::board_type& StarflowSetOfGames::board(size_t n_board) const {
	if (n_board == random) {
		std::uniform_int_distribution<size_t> rnd(0, num_boards_ - 1);
		n_board = rnd(rgenerator_);
	}

	for (const auto& i : boards_) {
		if (n_board >= i.second.size()) {
			n_board -= i.second.size();
		}
		else {
			return i.second[n_board];
		}
	}

	std::cout << "StarflowSetOfGames warning : INDEX NOT AVAILABLE, returning random element\n";

	return board(random);
}

const StarflowSetOfGames::board_type& StarflowSetOfGames::board(size_t size, size_t n_board) const {
	decltype(boards_.cbegin()) it_size;

	if (size == random) {
		std::uniform_int_distribution<size_t> rnd(0,boards_.size()-1);
		it_size = boards_.cbegin();
		std::advance(it_size, rnd(rgenerator_));
	}
	else {
		it_size = boards_.find(size);

		if (it_size == boards_.end()) {
			std::cout << "StarflowSetOfGames warning : SIZE NOT AVAILABLE, returning random element\n";
			return board(random);//random any size
		}
	}

	if (n_board == random || n_board >= it_size->second.size()) {
		if (n_board >= it_size->second.size()) {
			std::cout << "StarflowSetOfGames warning : INDEX NOT AVAILABLE, returning random element\n";
		}

		std::uniform_int_distribution<size_t> rnd(0, it_size->second.size() - 1);
		n_board = rnd(rgenerator_);
	}

	return it_size->second[n_board];
}

void StarflowSetOfGames::add_board(const board_type & board)
{
	size_t size = board.size();
	for (const auto & i : board) {
		size = std::max(size, i.size());
	}


	boards_[size].push_back(board);
	num_boards_++;
}

size_t StarflowSetOfGames::num_boards() const
{
	return num_boards_;
}

size_t StarflowSetOfGames::num_boards(size_t size) const
{
	auto it = boards_.find(size);
	
	if (it == boards_.end()) {
		return 0;
	}
	
	return it->second.size();
}


StarflowSetOfGames::StarflowSetOfGames() {
	add_board({
		"A....",
		".....",
		"..D..",
		"CDB.A",
		"B...C",
	});

	add_board({
		"R....B...B",
		"...W......",
		"..WG......",
		".COb..O...",
		".g....b.G.",
		"........S.",
		"..R..P...C",
		"YS.......",
		".p....g.P.",
		".Y..p.....",
	});

	add_board({
		"G........",
		"R.b....P.",
		"Y....B...",
		"...Y.....",
		"........G",
		".OCp.B..P",
		".......bO",
		"....C..p.",
		".R.......",
	});

	add_board({
		"..PY...",
		".....C.",
		".C...G.",
		".O.B.R.",
		"....B..",
		"..O..R.",
		"...PGY.",
	});

	add_board({
		"GB...R..",
		"...G.BC.",
		"O....P..",
		".Y......",
		".PO..R..",
		"......Y.",
		".b..b.C.",
		"........",
	});

	add_board({
		".....",
		".A.D.",
		"..D..",
		"..B.C",
		"BAC..",
	});

	add_board({
		"ABC.DE",
		"....F.",
		"..C...",
		"..D...",
		"A.F...",
		"B.E...",
	});

	add_board({
		"A.B.C",
		"..D.E",
		".....",
		".B.C.",
		".ADE.",
	});

	add_board({
		".......b..",
		".Y..PC..G.",
		"...BO.....",
		"..........",
		".YP....O..",
		".....G....",
		"..........",
		".....B....",
		"..R.....C.",
		"R.......b.",
	});

	add_board({
		"......Y...",
		"....P.G.G.",
		"......RY..",
		"...b...B..",
		".....O....",
		"C....RbC..",
		"P.........",
		"BO........",
		"..........",
		"..........",
	});

	add_board({
		"...........",
		"......Y.W.P",
		"......S.O..",
		".SYC.......",
		"..g.....C..",
		".g.........",
		".O.......R.",
		".B..P..W...",
		".....B.p...",
		"R......G..p",
		"q.......q.G",
	});

	add_board({
		"P............",
		"R..C...WB..B.",
		".P.R...G...W.",
		".Y....G..S...",
		".............",
		".......SOg...",
		"......y.P....",
		"..........O..",
		".....x....b..",
		"C.......P....",
		".....y..g....",
		".b.x.........",
		"............Y",
	});

	add_board({
		"..............",
		".B...B........",
		"...........h..",
		"........M.....",
		".p........Y...",
		"...Rh.g.......",
		"..C.C..R......",
		"Y......g...W..",
		"....W..O.B....",
		"....G..G.S....",
		"..PO..........",
		"....P.....B..x",
		"............p.",
		".......S.Mx...",
	});
}
