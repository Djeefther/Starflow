#include "star_flow_game.h"




void print_all_paths_files(StarFlowGame& sf) {
	for (int i = 0; i < sf.num_colors(); i++) {
		std::ofstream f(std::to_string(i) + ".txt");
		auto path = sf.path(i);
		FPRINTLN_VALUE(f, path.size());
		for (auto p : path) {
			sf.aplly_at_board(p, i);
			sf.print(f);
			sf.remove_at_board(p);
		}
	}
}
