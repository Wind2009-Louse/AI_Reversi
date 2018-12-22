#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <algorithm>
#include "omp.h"
#include <stdlib.h>// rand
#include <ctime>
#include <fstream>
#include <sstream>
using namespace std;

#define BOT_PIECE 1
#define PLAYER_PIECE -1
#define EMPTY_PIECE 0
#define BLACK_PIECE "●"
#define WHITE_PIECE "○"

#define BOARD_SIZE 8
#define FINAL_POWER 1
#define HEREDITART_PER_THREAD 3

#define SEARCH_MAX_DEPTH 5 // search depth
#define SEARCH_MAX_DEPTH_END 13 // search depth while at end
#define MOVE_INITIAL_POWER 5 // move power at first(cut down as more pieces on board)
#define MOVE_DIFF_POWER 2 // power makes by move possibility's difference

const vector<string> full_alpha_list = { "Ａ", "Ｂ", "Ｃ", "Ｄ", "Ｅ", "Ｆ", "Ｇ",
									"Ｈ", "Ｉ", "Ｊ", "Ｋ", "Ｌ", "Ｍ", "Ｎ",
									"Ｏ", "Ｐ", "Ｑ", "Ｒ", "Ｓ", "Ｔ", "Ｕ",
									"Ｖ", "Ｗ", "Ｘ", "Ｙ", "Ｚ" };
const short find_dir[8][2] = { {-1,-1},{0,-1},{1,-1},
								{-1,0},{1,0},
							{ -1,1 },{ 0,1 },{ 1,1 }};
// var_list[struct_type][struct_id][index] = place_id
const vector< vector< vector<int> > > var_lists = {
	// first line
	{ { 0,1,2,3,4,5,6,7 },{ 56,57,58,59,60,61,62,63 },{ 0,8,16,24,32,40,48,56 },{ 7,15,23,31,39,47,55,63 },
	{ 7,6,5,4,3,2,1,0 },{ 63,62,61,60,59,58,57,56 },{ 56,48,40,32,24,16,8,0 },{ 63,55,47,39,31,23,15,7 } },
	// second line
	{ { 8,9,10,11,12,13,14,15 },{ 48,49,50,51,52,53,54,55 },{ 1,9,17,25,33,41,49,57 },{ 6,14,22,30,38,46,54,62 },
	{ 15,14,13,12,11,10,9,8 },{ 55,54,53,52,51,50,49,48 },{ 57,49,41,33,25,17,9,1 },{ 62,54,46,38,30,22,14,6 } },
	// third line
	{ { 16,17,18,19,20,21,22,23 },{ 40,41,42,43,44,45,46,47 },{ 2,10,18,26,34,42,50,58 },{ 5,13,21,29,37,45,53,61 },
	{ 23,22,21,20,19,18,17,16 },{ 47,46,45,44,43,42,41,40 },{ 58,50,42,34,26,18,10,2 },{ 61,53,45,37,29,21,13,5 } },
	// forth line
	{ { 24,25,26,27,28,29,30,31 },{ 32,33,34,35,36,37,38,39 },{ 3,11,19,27,35,43,51,59 },{ 4,12,20,28,36,44,52,60 },
	{ 31,30,29,28,27,26,25,24 },{ 39,38,37,36,35,34,33,32 },{ 59,51,43,35,27,19,11,3 },{ 60,52,44,36,28,20,12,4 } },
	// 5-length corner
	{ { 4,11,18,25,32 },{ 3,12,21,30,39 },{ 24,33,42,51,60 },{ 31,38,45,52,59 },
	{ 32,25,18,11,4 },{ 39,30,21,12,3 },{ 60,51,42,33,24 },{ 59,52,45,38,31 } },
	// 6-length corner
	{ { 5,12,19,26,33,40 },{ 2,11,20,29,38,47 },{ 16,25,34,43,52,61 },{ 23,30,37,44,51,58 },
	{ 40,33,26,19,12,5 },{ 47,38,29,20,11,2 },{ 61,52,43,34,25,16 },{ 58,51,44,37,30,23 } },
	// 7-length corner
	{ { 1,10,19,28,37,46,55 },{ 6,13,20,27,34,41,48 },{ 8,17,26,35,44,53,62 },{ 15,22,29,36,43,50,57 },
	{ 55,46,37,28,19,10,1 },{ 48,41,34,27,20,13,6 },{ 62,53,44,35,26,17,8 },{ 57,50,43,36,29,22,15 } },
	// 8-length corner
	{ { 0,9,18,27,36,45,54,63 },{ 7,14,21,28,35,42,49,56 },{ 63,54,45,36,27,18,9,0 },{ 56,49,42,35,28,21,14,7 } },
	// corner 3*3
	{ { 0,1,2,8,9,10,16,17,18 },{ 7,15,23,6,14,22,5,13,21 },{ 63,62,61,55,54,53,47,46,45 },{ 56,48,40,57,49,41,58,50,42 },
	{ 0,8,16,1,9,17,2,10,18 },{ 7,6,5,15,14,13,23,22,21 },{ 63,55,47,62,54,46,61,53,45 },{ 56,57,58,48,49,50,40,41,42 }},
	// corner 2*4
	{ { 0,1,2,3,8,9,10,11 },{ 7,6,5,4,15,14,13,12 },{ 0,8,16,24,1,9,17,25 },{ 7,15,23,31,6,14,22,30 },
	{ 56,57,58,59,48,49,50,51 },{ 56,48,40,42,57,49,41,33 },{ 63,62,61,60,55,54,53,52 },{ 63,55,47,39,62,54,46,38 } },
	// near-corner 3*3
	{{ 1,2,3,9,10,11,17,18,19},{ 1,9,17,2,10,18,3,11,19},{ 8,9,10,16,17,18,24,25,26},{ 8,16,24,9,17,25,10,18,26 },
	{ 6,5,4,14,13,12,22,21,20},{ 6,14,22,5,13,21,4,12,20},{ 15,14,13,23,22,21,31,30,29},{ 15,23,31,14,22,30,13,21,29 },
	{ 48,49,50,40,41,42,32,33,34 },{ 48,40,32,49,41,33,50,42,34 },{ 57,58,59,49,50,51,41,42,43 },{ 57,49,41,58,50,42,59,51,43 },
	{ 55,54,53,47,46,45,39,38,37 },{ 55,47,39,54,46,38,53,45,37 },{ 62,61,60,54,53,52,46,45,44 },{ 62,54,46,61,53,45,60,52,44 }},
	// near-edge 3*3
	{{ 9,10,11,17,18,19,25,26,27 },{ 9,17,25,10,18,26,11,19,27 },{ 14,13,12,22,21,20,30,29,28 },{ 14,22,30,13,21,29,12,20,28 },
	{ 49,50,51,41,42,43,33,34,35 },{ 49,41,33,50,42,34,51,43,35 },{ 54,53,52,46,45,44,38,37,36 },{ 54,46,38,53,45,37,52,44,36 }}
};

vector <vector<int> > struct_vars;
vector <vector<int> > struct_times;
vector <int> move_var;
vector <int> move_times;
void struct_write();

double get_range_rand(double rand_min, double rand_max) {
	double range = rand_max - rand_min;
	const int rand_split = 100000;
	double rand_num = (double)(rand() % rand_split) / rand_split;
	return (rand_min + range*rand_num);
}

template<typename T> int find_in_vector(vector<T> v, T f) {
	for (int i = 0; i < v.size(); ++i)
		if (v[i] == f) return i;
	return -1;
}

struct Hereditary {
	short type;
	// move diff
	double move_initial_power;
	double move_diff_power;

	// power
	double corner_power;
	double edge_power;
	double near_corner_power;
	double near_edge_power;
	
	// stable & unstable
	double stable_power;
	double fake_stable_power;
	double unstable_power;

	// else
	double non_current_move_power;
	double importance_power;

	unsigned int total_win;
	unsigned int total_play;

	Hereditary(string filename="", short t = 0) {
		if (filename == "") {
			filename = "Hereditary.txt";
		}
		ifstream ifile;
		ifile.open(filename);
		if (ifile.is_open()) {
			vector<double> powers;
			while (!ifile.eof()) {
				double temp;
				ifile >> temp;
				powers.push_back(temp);
			}
			ifile.close();
			if (powers.size() == 11) {
				move_initial_power = powers[0];
				move_diff_power = powers[1];
				corner_power = powers[2];
				edge_power = powers[3];
				near_corner_power = powers[4];
				near_edge_power = powers[5];
				stable_power = powers[6];
				fake_stable_power = powers[7];
				unstable_power = powers[8];
				non_current_move_power = powers[9];
				importance_power = powers[10];
				total_play = 0;
				total_win = 0;
				cout << "Read from file!" << endl;
				set();
				return;
			}
			else if (powers.size() == 12) {
				type = powers[0];
				move_initial_power = powers[1];
				move_diff_power = powers[2];
				corner_power = powers[3];
				edge_power = powers[4];
				near_corner_power = powers[5];
				near_edge_power = powers[6];
				stable_power = powers[7];
				fake_stable_power = powers[8];
				unstable_power = powers[9];
				non_current_move_power = powers[10];
				importance_power = powers[11];
				total_play = 0;
				total_win = 0;
				cout << "Read from file!" << endl;
				set();
				return;
			}
		}
		ifile.close();
		move_initial_power = MOVE_INITIAL_POWER;
		move_diff_power = MOVE_DIFF_POWER;
		corner_power = 3;
		edge_power = 2;
		near_corner_power = 0.1;
		near_edge_power = 0.2;
		stable_power = 10;
		fake_stable_power = 2;
		unstable_power = 0.1;
		non_current_move_power = 0.9;
		importance_power = 1.5;

		total_win = 0;
		total_play = 0;
		type = t;
		set();
	}
	Hereditary(Hereditary* copy) {
		move_initial_power = copy->move_initial_power;
		move_diff_power = copy->move_diff_power;
		corner_power = copy->corner_power;
		edge_power = copy->edge_power;
		near_corner_power = copy->near_corner_power;
		near_edge_power = copy->near_edge_power;
		stable_power = copy->stable_power;
		fake_stable_power = copy->fake_stable_power;
		unstable_power = copy->unstable_power;
		non_current_move_power = copy->non_current_move_power;
		importance_power = copy->importance_power;
		type = copy->type;
		total_win = 0;
		total_play = 0;
		set();
	}

	// 0.5~2
	void small_variation() {
		variation(0.5, 2);
	}

	// 0.2~5
	void huge_variation() {
		variation(0.2, 5);
	}
	
	void set() {
		double total_power_1 = 0;
		double total_power_2 = 0;
		total_power_1 += edge_power;
		total_power_1 += corner_power;
		total_power_1 += near_edge_power;
		total_power_1 += near_corner_power;
		total_power_2 += stable_power;
		total_power_2 += unstable_power;
		total_power_2 += fake_stable_power;
		total_power_1 /= 5;
		total_power_2 /= 5;
		edge_power /= total_power_1;
		corner_power /= total_power_1;
		near_edge_power /= total_power_1;
		near_corner_power /= total_power_1;
		stable_power /= total_power_2;
		unstable_power /= total_power_2;
		fake_stable_power /= total_power_2;
	}

	void variation(double range_min, double range_max) {
		move_initial_power *= get_range_rand(range_min, range_max);
		move_diff_power *= get_range_rand(range_min, range_max);
		corner_power *= get_range_rand(range_min, range_max);
		edge_power *= get_range_rand(range_min, range_max);
		near_corner_power *= get_range_rand(range_min, range_max);
		near_edge_power *= get_range_rand(range_min, range_max);
		stable_power *= get_range_rand(range_min, range_max);
		fake_stable_power *= get_range_rand(range_min, range_max);
		unstable_power *= get_range_rand(range_min, range_max);
		non_current_move_power *= get_range_rand(range_min, range_max);
		importance_power *= get_range_rand(range_min, range_max);
		set();
	}

	double win_rate() {
		if (total_play == 0) {
			return 0;
		}
		return (double)total_win / (double)total_play;
	}
};

bool hereditary_cmp(Hereditary* h1, Hereditary* h2) {
	if (h1->win_rate() == h2->win_rate()) {
		return h1->total_play > h2->total_play;
	}
	return h1->win_rate() > h2->win_rate();
};

Hereditary* get_hereditary(string filename = "") {
	Hereditary* initial_her = new Hereditary(filename);
	int mode = -1;
	while (mode == -1) {
		cout << "默认遗传子(0) / 设定遗传子(1）:";
		cin >> mode;
		if (mode < 0 || mode > 1) {
			cout << "Illegal input!" << endl;
			if (cin.fail()) {
				cin.sync();
				cin.clear();
				cin.ignore();
			}
			mode = -1;
		}
	}
	// set Hereditary
	if (mode == 1) {
		cout << "type:";
		cin >> initial_her->type;
		cout << "move_initial_power:";
		cin >> initial_her->move_initial_power;
		cout << "move_diff_power:";
		cin >> initial_her->move_diff_power;
		cout << "corner_power:";
		cin >> initial_her->corner_power;
		cout << "edge_power";
		cin >> initial_her->edge_power;
		cout << "near_corner_power:";
		cin >> initial_her->near_corner_power;
		cout << "near_edge_power:";
		cin >> initial_her->near_edge_power;
		cout << "stable_power:";
		cin >> initial_her->stable_power;
		cout << "fake_stable_power:";
		cin >> initial_her->fake_stable_power;
		cout << "unstable_power:";
		cin >> initial_her->unstable_power;
		cout << "non_current_move_power:";
		cin >> initial_her->non_current_move_power;
		cout << "importance_power:";
		cin >> initial_her->importance_power;
	}
	return initial_her;
}

struct Board {
	double alpha;
	double beta;
	short board[BOARD_SIZE * BOARD_SIZE];
	bool is_bot_first;
	bool is_on_bot;
	int search_depth;
	Hereditary* herediatry;

	Board(bool bot_first = false) {
		alpha = SHRT_MIN;
		beta = SHRT_MAX;
		is_on_bot = bot_first;
		is_bot_first = bot_first;
		for (int i = 0; i < BOARD_SIZE*BOARD_SIZE; ++i) board[i]=EMPTY_PIECE;
		short black = bot_first ? BOT_PIECE : PLAYER_PIECE;
		short white = bot_first ? PLAYER_PIECE : BOT_PIECE;
		short upleft = BOARD_SIZE / 2 - 1;
		onboard(upleft, upleft) = -black;
		onboard(upleft+1, upleft+1) = -black;
		onboard(upleft, upleft+1) = -white;
		onboard(upleft+1, upleft) = -white;
		search_depth = -1;
		herediatry = new Hereditary;
		structvar_initial();
	}
	Board(Hereditary* her, bool bot_first = false) {
		alpha = SHRT_MIN;
		beta = SHRT_MAX;
		is_on_bot = bot_first;
		is_bot_first = bot_first;
		for (int i = 0; i < BOARD_SIZE*BOARD_SIZE; ++i) board[i] = EMPTY_PIECE;
		short black = bot_first ? BOT_PIECE : PLAYER_PIECE;
		short white = bot_first ? PLAYER_PIECE : BOT_PIECE;
		short upleft = BOARD_SIZE / 2 - 1;
		onboard(upleft, upleft) = -black;
		onboard(upleft + 1, upleft + 1) = -black;
		onboard(upleft, upleft + 1) = -white;
		onboard(upleft + 1, upleft) = -white;
		search_depth = -1;
		herediatry = her;
		structvar_initial();
	}
	Board(Board* b) {
		alpha = b->alpha;
		beta = b->beta;
		memcpy_s(board, BOARD_SIZE * BOARD_SIZE * sizeof(short), b->board, BOARD_SIZE * BOARD_SIZE * sizeof(short));
		is_on_bot = b->is_on_bot;
		is_bot_first = b->is_bot_first;
		search_depth = b->search_depth;
		herediatry = b->herediatry;
	}
	void structvar_initial() {
		bool need_rewrite = false;
		for (int i = 0; i < var_lists.size(); ++i) {
			if (struct_vars.size() <= i) {
				struct_vars.push_back(vector<int>());
			}
			if (struct_times.size() <= i) {
				struct_times.push_back(vector<int>());
			}
			struct_vars[i].clear();
			struct_times[i].clear();
			ifstream struct_file;
			int var_size = var_lists[i][0].size();
			
			string filename = "struct/";
			filename += to_string(i);
			filename += ".txt";
			struct_file.open(filename);
			if (struct_file.is_open()) {
				for (int j = 0; j < pow(3, var_size); ++j) {
					int _temp;
					struct_file >> _temp;
					struct_vars[i].push_back(_temp);
				}
				struct_file.close();
			}
			else {
				struct_file.close();
				need_rewrite = true;
				for (int j = 0; j < pow(3, var_size); ++j) {
					struct_vars[i].push_back(0);
				}
			}

			filename = "struct/t_";
			filename += to_string(i);
			filename += ".txt";
			struct_file.open(filename);
			if (struct_file.is_open()) {
				for (int j = 0; j < pow(3, var_size); ++j) {
					int _temp;
					struct_file >> _temp;
					struct_times[i].push_back(_temp);
				}
				struct_file.close();
			}
			else {
				struct_file.close();
				need_rewrite = true;
				for (int j = 0; j < pow(3, var_size); ++j) {
					struct_times[i].push_back(0);
				}
			}
		}
		// move
		move_var.clear();
		move_times.clear();
		ifstream struct_file;
		struct_file.open("struct/move.txt");
		if (struct_file.is_open()) {
			for (int j = 0; j < 3600; ++j) {
				int _temp;
				struct_file >> _temp;
				move_var.push_back(_temp);
			}
			struct_file.close();
		}
		else {
			struct_file.close();
			need_rewrite = true;
			for (int j = 0; j < 3600; ++j) {
				move_var.push_back(0);
			}
		}
		struct_file.open("struct/t_move.txt");
		if (struct_file.is_open()) {
			for (int j = 0; j < 3600; ++j) {
				int _temp;
				struct_file >> _temp;
				move_times.push_back(_temp);
			}
			struct_file.close();
		}
		else {
			struct_file.close();
			need_rewrite = true;
			for (int j = 0; j < 3600; ++j) {
				move_times.push_back(0);
			}
		}

		if (need_rewrite) {
			struct_write();
		}
	}
	// x,y's range: 0~BOARD_SIZE
	short& onboard(int x, int y) {
		if (x < 0 || x > BOARD_SIZE-1 || y < 0 || y > BOARD_SIZE-1) {
			cout << "Error!" << endl;
			system("pause");
			throw;
		}
		return board[y * BOARD_SIZE + x];
	}
	vector<short> next_possible() {
		vector<short> results;
		short self_piece = (is_on_bot) ? BOT_PIECE : PLAYER_PIECE;
		short enemy_piece = (!is_on_bot) ? BOT_PIECE : PLAYER_PIECE;
		for (int i = 0; i < BOARD_SIZE*BOARD_SIZE; ++i) {
			if (board[i] == EMPTY_PIECE) {
				for (int j = 0; j < 8; ++j) {
					bool find_enemy = false;
					bool find_self = false;
					short _x = i % BOARD_SIZE + find_dir[j][0];
					short _y = i / BOARD_SIZE + find_dir[j][1];
					// find {enemy, enenmy, enemy..., self}
					while (_x > -1 && _x < BOARD_SIZE && _y > -1 && _y < BOARD_SIZE) {
						if (onboard(_x, _y) == enemy_piece) {
							find_enemy = true;
						}
						else if (onboard(_x, _y) == self_piece) {
							find_self = true;
							break;
						}
						else if (onboard(_x, _y) == EMPTY_PIECE) {
							break;
						}
						_x += find_dir[j][0];
						_y += find_dir[j][1];
					} 
					if (find_enemy && find_self) {
						results.push_back(i);
						break;
					}
				}
			}
		}
		return results;
	}
	void print(bool print_optimal = false) {
		vector<short> optimal;
		if (print_optimal) optimal = next_possible();
		string bot_piece = (is_bot_first) ? BLACK_PIECE : WHITE_PIECE;
		string player_piece = (is_bot_first) ? WHITE_PIECE : BLACK_PIECE;
		string block = "";
		for (int i = 0; i < BOARD_SIZE; ++i) {
			block += "━";
		}
		cout << "┏" << block << "┓" << endl;
		for (int _y = 0; _y < BOARD_SIZE; ++_y) {
			cout << "┃";
			for (int _x = 0; _x < BOARD_SIZE; ++_x) {
				short this_board = onboard(_x, _y);
				if (this_board == BOT_PIECE) {
					cout << bot_piece;
				}
				else if (this_board == PLAYER_PIECE) {
					cout << player_piece;
				}
				else {
					short this_index = _y * BOARD_SIZE + _x;
					int find_index = find_in_vector(optimal, this_index);
					if (find_index == -1) {
						cout << "　";
					}
					else {
						cout << full_alpha_list[find_index];
					}
				}
			}
			cout << "┃" << endl;
		}
		cout << "┗" << block << "┛" << endl;
	}
	vector<short> enemy_possible() {
		is_on_bot = !is_on_bot;
		vector<short> enemy_pos = next_possible();
		is_on_bot = !is_on_bot;
		return enemy_pos;
	}
	void new_step(short step_where) {
		short self_piece = (is_on_bot) ? BOT_PIECE : PLAYER_PIECE;
		short enemy_piece = (is_on_bot) ? PLAYER_PIECE : BOT_PIECE;
		board[step_where] = self_piece;
		is_on_bot = !is_on_bot;
		// reverse
		for (int i = 0; i < 8; ++i) {
			vector<short> to_be_reverse;
			short _x = step_where % BOARD_SIZE + find_dir[i][0];
			short _y = step_where / BOARD_SIZE + find_dir[i][1];
			bool next_self = false;
			while (_x >= 0 && _x <= BOARD_SIZE-1 && _y >= 0 && _y <= BOARD_SIZE-1) {
				if (onboard(_x, _y) == enemy_piece) {
					to_be_reverse.push_back(_y * BOARD_SIZE + _x);
				}
				else if (onboard(_x, _y) == self_piece) {
					next_self = true;
					break;
				}
				else if (onboard(_x, _y) == EMPTY_PIECE) {
					break;
				}
				else {
					cout << "Error!" << endl;
					throw;
				}
				_x += find_dir[i][0];
				_y += find_dir[i][1];
			}
			if (!next_self) {
				to_be_reverse.clear();
			}
			for (int t = 0; t < to_be_reverse.size(); ++t) {
				board[to_be_reverse[t]] = self_piece;
			}
		}
	}
	short bot_on_ideal_step(bool debug = true, bool use_struct = false, bool random_move = false) {
		// whether use end search?
		int empty_count = BOARD_SIZE*BOARD_SIZE;
		for (int i = 0; i < BOARD_SIZE*BOARD_SIZE; ++i) {
			if (board[i] != EMPTY_PIECE) empty_count--;
		}
		if (empty_count <= SEARCH_MAX_DEPTH_END) {
			search_depth = SEARCH_MAX_DEPTH_END;
		}
		else {
			search_depth = SEARCH_MAX_DEPTH;
		}
		
		alpha = DBL_MIN;
		beta = DBL_MAX;
		vector<short> next_steps = next_possible();
		pair<int, double> expection = get_self_value(use_struct, random_move);
		short next_step_id = expection.first;
		if (debug) {
			string type = (use_struct ? "(模板评估)" : "(全局评估)");
			cout << "电脑认为能够达到的期望值：" << type << expection.second << endl;
			cout << "电脑下子：(" << 1 + next_step_id / BOARD_SIZE << ", " << 1 + next_step_id % BOARD_SIZE << ")。" << endl;
		}
		new_step(next_step_id);
		return next_step_id;
	}
	// <from which children(index of vector), value>
	pair<int,double> get_self_value(bool use_struct, bool random_move = false) {
		double value = 0;
		vector<short> next_steps = next_possible();
		pair<int, double> result(-1, 0);
		if (next_steps.size() == 0) {
			// to leaf
			vector<short> enemy_pos = enemy_possible();
			if (enemy_pos.size() == 0) {
				bool has_bot = false;
				bool has_player = false;
				for (int i = 0; i < BOARD_SIZE*BOARD_SIZE; ++i) {
					if (!has_bot && board[i] == BOT_PIECE) has_bot = true;
					if (!has_player && board[i] == PLAYER_PIECE) has_player = true;
					value += board[i];
				}
				if (!has_player) value = BOARD_SIZE*BOARD_SIZE;
				else if (!has_player) value = -BOARD_SIZE*BOARD_SIZE;
				return pair<int, double>(-1, value*FINAL_POWER);
			}
			// reverse
			else {
				is_on_bot = !is_on_bot;
				next_steps = enemy_pos;
			}
		}
		// to bottom
		if (search_depth == 0) {
			double value;
			if (use_struct) {
				value = calculate_struct();
			}
			else {
				value = calculate_current_value();
			}
			return pair<int, double>(-1, value);
		}	
		if (random_move) {
			random_shuffle(next_steps.begin(), next_steps.end());
		}
		for (int i = 0; i < next_steps.size(); ++i) {
			Board* next_board = new Board(this);
			next_board->new_step(next_steps[i]);
			next_board->search_depth--;
			// only random at the first level
			pair<int,double> next_value = next_board->get_self_value(use_struct);
			delete next_board;
			if (
				((result.first == -1) || 
				(result.second < next_value.second && is_on_bot) ||
					(result.second > next_value.second && !is_on_bot)) )
			{
				result.first = next_steps[i];
				result.second = next_value.second;
				// alpha update
				if ((alpha < result.second) && is_on_bot) {
					alpha = result.second;
				}
				// beta update
				else if ((beta > result.second) && !is_on_bot) {
					beta = result.second;
				}
			}
			// cut
			if (alpha > beta) {
				//result.first = -2;
				return result;
			}
		}
		return result;
	}
	double calculate_current_value(bool is_current = false) {
		double bot_value = 0;
		double player_value = 0;
		int not_empty_count = 0;
		vector<int> bot_stables;
		vector<int> player_stables;
		for (int _id = 0; _id < BOARD_SIZE*BOARD_SIZE; ++_id) {
			short _x = _id % BOARD_SIZE;
			short _y = _id / BOARD_SIZE;
			short this_piece = board[_id];
			if (this_piece == EMPTY_PIECE) {
				continue;
			}
			not_empty_count++;
			// simple importance
			double importance = 1;
			int to_edge_x = min(abs(_x - 0), abs(_x - (BOARD_SIZE - 1)));
			int to_edge_y = min(abs(_y - 0), abs(_y - (BOARD_SIZE - 1)));
			// corner
			if (to_edge_x == 0 && to_edge_y == 0){
				importance *= herediatry->corner_power;
			}
			else {
				// near corner
				if (to_edge_x < 2 && to_edge_y < 2) {
					importance *= herediatry->near_corner_power;
				}
				// near the edge
				else if (to_edge_x == 1 || to_edge_y == 1) {
					importance *= herediatry->near_edge_power;
				}
				// at the edge
				else if (_x == 0 || _x == BOARD_SIZE - 1 || _y == 0 || _y == BOARD_SIZE - 1) {
					importance *= herediatry->edge_power;
				}
			}
			// stable importance
			bool is_stable = true;
			bool is_fake_stable = true;
			bool is_unstable = false;
			for (int i = 0; i < 4; ++i) {
				short enemy_surround = 0;
				int check_x = _x + find_dir[i][0];
				int check_y = _y + find_dir[i][1];
				while (check_x >= 0 && check_x < BOARD_SIZE && check_y >= 0 && check_y < BOARD_SIZE) {
					short piece_check = board[check_y * BOARD_SIZE + check_x] * this_piece;
					// self
					if (piece_check == 1) {
						check_x += find_dir[i][0];
						check_y += find_dir[i][1];
					}
					// enemy
					else if (piece_check == -1) {
						enemy_surround++;
						break;
					}
					else if (piece_check == 0) {
						enemy_surround += 4;
						break;
					}
				}
				check_x = _x - find_dir[i][0];
				check_y = _y - find_dir[i][1];
				while (check_x >= 0 && check_x < BOARD_SIZE && check_y >= 0 && check_y < BOARD_SIZE) {
					short piece_check = board[check_y * BOARD_SIZE + check_x] * this_piece;
					// self
					if (piece_check == 1) {
						check_x -= find_dir[i][0];
						check_y -= find_dir[i][1];
					}
					else if (piece_check == -1) {
						// enemy
						enemy_surround++;
						break;
					}
					else if (piece_check == 0) {
						enemy_surround += 4;
						break;
					}
				}
				// 0(total line), 1(single enemy), 4(single empty)
				if ((enemy_surround != 0) && (enemy_surround != 1) && (enemy_surround != 4)) {
					is_stable = false;
					// 0(total line), 1(single enemy), 2(both enemy), 4(single empty)
					if (enemy_surround != 2) {
						is_fake_stable = false;
					}
				}
				// 5(enemy & empty)
				if (enemy_surround == 5) {
					is_unstable = true;
					break;
				}
			}
			if (is_unstable) {
				importance *= herediatry->unstable_power;
			}
			else if (is_stable) {
				if (is_current) {
					if (this_piece == BOT_PIECE) {
						bot_stables.push_back(_y*BOARD_SIZE + _x);
					}
					else {
						player_stables.push_back(_y*BOARD_SIZE + _x);
					}
				}
				importance *= herediatry->stable_power;
			}
			else if (is_fake_stable) {
				importance *= herediatry->fake_stable_power;
			}
			if (this_piece == BOT_PIECE) {
				bot_value += this_piece * pow(herediatry->importance_power, importance);
			}
			else {
				player_value += this_piece * pow(herediatry->importance_power, importance);
			}
		}
		// movable check
		double bot_move_power;
		double player_move_power;
		if (is_on_bot) {
			bot_move_power = next_possible().size();
			is_on_bot = !is_on_bot;
			player_move_power = next_possible().size();
			is_on_bot = !is_on_bot;
			player_move_power *= herediatry->non_current_move_power;
		}
		else {
			player_move_power = next_possible().size();
			is_on_bot = !is_on_bot;
			bot_move_power = next_possible().size();
			is_on_bot = !is_on_bot;
			bot_move_power *= herediatry->non_current_move_power;
		}
		double result = 0;
		double total_power = player_move_power + bot_move_power;
		double power = MOVE_INITIAL_POWER * (1 - pow(not_empty_count, 3) / pow(BOARD_SIZE, 6));
		if (herediatry->type == 0) {
			bot_move_power = pow(1 + (herediatry->move_diff_power)*(bot_move_power / total_power), power);
			player_move_power = pow(1 + (herediatry->move_diff_power)*(player_move_power / total_power), power);
			double _temp = bot_move_power + player_move_power;
			result = bot_value * (bot_move_power / _temp) + player_value * (player_move_power / _temp);
		}
		else {
			bot_move_power = pow(1 + (herediatry->move_diff_power)*bot_move_power, power);
			player_move_power = pow(1 + (herediatry->move_diff_power)*player_move_power, power);
			result = bot_value + player_value + bot_move_power - player_move_power;
		}
		if (is_current) {
			cout << "当前电脑稳定子：";
			for (int i = 0; i < bot_stables.size(); ++i) {
				cout << bot_stables[i] << ",";
			}
			cout << endl << "玩家稳定子：";
			for (int i = 0; i < player_stables.size(); ++i) {
				cout << player_stables[i] << ",";
			}
			cout << endl << "电脑评估值：(" << bot_value << ", " << bot_move_power << ")" << endl;
			cout << "玩家评估值：（" << -player_value << ", " << player_move_power << ")" << endl;
			cout << "总评估值：" << result << "。" << endl;
		}
		return result;
	}
	double calculate_struct() {
		int current_stage = 0;
		for (int i = 0; i < BOARD_SIZE*BOARD_SIZE; ++i) {
			if (board[i] == 0) current_stage++;
		}
		if (current_stage == 0) {
			int value = 0;
			bool has_bot = false;
			bool has_player = false;
			for (int i = 0; i < BOARD_SIZE*BOARD_SIZE; ++i) {
				if (!has_bot && board[i] == BOT_PIECE) has_bot = true;
				if (!has_player && board[i] == PLAYER_PIECE) has_player = true;
				value += board[i];
			}
			if (!has_player) value = BOARD_SIZE*BOARD_SIZE;
			else if (!has_player) value = -BOARD_SIZE*BOARD_SIZE;
			return value*FINAL_POWER;
		}
		double result = 0;
		// board struct
//#pragma omp parallel for
		for (int i = 0; i < var_lists.size(); ++i) {
			double _result = 0;
			vector< vector<int> > var_list = var_lists[i];
			for (int j = 0; j < var_list.size(); ++j) {
				int id = 0;
				for (int k = 0; k < var_list[j].size(); ++k) {
					id *= 3;
					id += board[var_list[j][k]] + 1;
				}
				_result += (double)struct_vars[i][id] / (struct_times[i][id] == 0 ? 1 : struct_times[i][id]);
			}
#pragma omp atomic
			result += _result;
		}
		// move struct
		if (current_stage != 60) {
			int move_power_1 = next_possible().size();
			int move_power_2 = enemy_possible().size();
			int bot_move_power, enemy_move_power;
			if (is_on_bot) {
				bot_move_power = move_power_1;
				enemy_move_power = move_power_2;
			}
			else {
				bot_move_power = move_power_2;
				enemy_move_power = move_power_1;
			}
			int move_id = (current_stage) * 60 + bot_move_power;
			result += (double)move_var[move_id] / (move_times[move_id] == 0 ? 1 : move_times[move_id]);
			move_id = current_stage * 60 + enemy_move_power;
			result -= (double)move_var[move_id] / (move_times[move_id] == 0 ? 1 : move_times[move_id]);
		}
		return result;
	}
	void all_reverse() {
		for (int i = 0; i < BOARD_SIZE*BOARD_SIZE; ++i) {
			if (board[i] == BOT_PIECE) board[i] = PLAYER_PIECE;
			else if (board[i] == PLAYER_PIECE) board[i] = BOT_PIECE;
		}
	}
};

void update_struct(int power, Board* target, short index = -1) {
	// struct update
	//#pragma omp parallel for
	for (int i = 0; i < var_lists.size(); ++i) {
		vector< vector<int> > var_list = var_lists[i];
		for (int j = 0; j < var_list.size(); ++j) {
			int id = 0;
			int _id = 0;
			bool included = false;
			for (int k = 0; k < var_list[j].size(); ++k) {
				if (var_list[j][k] == index) {
					included = true;
				}
				id *= 3;
				_id *= 3;
				id += target->board[var_list[j][k]] + 1;
				_id += target->board[var_list[j][k]] * -1 + 1;
			}
			if (index == -1 || included) {				
#pragma omp critical
				{
					struct_vars[i][id] += power;
					struct_vars[i][_id] -= power;
					struct_times[i][id] ++;
					struct_times[i][_id] ++;
				}
			}
		}
	}
	// move update;
	int current_stage = 0;
	for (int i = 0; i < BOARD_SIZE*BOARD_SIZE; ++i) {
		if (target->board[i] == 0) current_stage++;
	}
	int bot_move, enemy_move;
	int move_power_1 = target->next_possible().size();
	int move_power_2 = target->enemy_possible().size();
	if (target->is_on_bot) {
		bot_move = move_power_1;
		enemy_move = move_power_2;
	}
	else {
		bot_move = move_power_2;
		enemy_move = move_power_1;
	}
	int bot_id = current_stage * 60 + bot_move;
	int enemy_id = current_stage * 60 + enemy_move;
#pragma omp critical
	{
		move_var[bot_id] += power;
		move_var[enemy_id] -= power;
		move_times[bot_id]++;
		move_times[enemy_id]++;
	}
}

void struct_write() {
	for (int i = 0; i < struct_vars.size(); ++i) {
		ofstream write_file;
		string filename = "struct/";
		filename += to_string(i);
		filename += ".txt";
		write_file.open(filename);
		for (int j = 0; j < struct_vars[i].size(); ++j) {
			write_file << struct_vars[i][j] << endl;
		}
		write_file.close();
	}
	for (int i = 0; i < struct_times.size(); ++i) {
		ofstream write_file;
		string filename = "struct/t_";
		filename += to_string(i);
		filename += ".txt";
		write_file.open(filename);
		for (int j = 0; j < struct_times[i].size(); ++j) {
			write_file << struct_times[i][j] << endl;
		}
		write_file.close();
	}
	ofstream write_file("struct/move.txt");
	for (int i = 0; i < 3600; ++i) {
		write_file << move_var[i] << endl;
	}
	write_file.close();
	write_file.open("struct/t_move.txt");
	for (int i = 0; i < 3600; ++i) {
		write_file << move_times[i] << endl;
	}
	write_file.close();
}

int pvc() {
	while (true) {
		vector<short> record;
		Hereditary* initial_her = get_hereditary();
		int mode = -1;
		while (mode == -1) {
			cout << "请选择先后(0=先手, 1=后手):";
			cin >> mode;
			if (mode < 0 || mode > 1) {
				cout << "Illegal input!" << endl;
				if (cin.fail()) {
					cin.sync();
					cin.clear();
					cin.ignore();
				}
				mode = -1;
			}
		}
		string your = (mode) ? WHITE_PIECE : BLACK_PIECE;
		your = "(" + your + ")";
		Board* current = new Board(initial_her,mode == 1);
		bool use_struct = false;
		int _ip = -1;
		while (_ip == -1) {
			cout << "请选择评估方式(0=全局, 1=模块):";
			cin >> _ip;
			if (_ip < 0 || _ip > 1) {
				cout << "Illegal input!" << endl;
				if (cin.fail()) {
					cin.sync();
					cin.clear();
					cin.ignore();
				}
				_ip = -1;
			}
		}
		use_struct = _ip == 1;
		vector<Board*> board_records;
		vector<short> bot_moves;
		while (true) {
			cout << endl;
			cout << "--------------------------------------" << endl;
			current->print(!current->is_on_bot);
			vector<short> list = current->next_possible();
			// no new step?
			if (list.size() == 0) {
				if (current->enemy_possible().size()==0) {
					break;
				}
				else {
					cout << "无子可下，需要弃权。" << endl;
					if (!current->is_on_bot) {
						system("pause");
					}
					record.push_back(-1);
					current->is_on_bot = !current->is_on_bot;
					continue;
				}
			}
			if (!use_struct) {
				current->calculate_current_value(true);
			}
			else {
				cout << "总评估值：" << current->calculate_struct() << "。" << endl;
			}
			
			if (current->is_on_bot) {
				cout << "电脑思考中..." << endl;
				short this_record = current->bot_on_ideal_step(true, use_struct,true);
				record.push_back(this_record);
				bot_moves.push_back(this_record);
			}
			else {
				bool rollback = false;
				int step;
				string ip;
				while (true) { 
					cout << "轮到你" << your << "下棋，请输入对应落位的字母，输入Z回滚：";
					cin >> ip;
					if (ip.size() > 1) {
						cout << "Illgeal!" << endl;
						if (cin.fail()) {
							cin.sync();
							cin.clear();
							cin.ignore();
						}
						continue;
					}
					else {
						char real_ip = ip[0];
						if (real_ip >= 'a' && real_ip <= 'z') {
							step = real_ip - 'a';
						}
						else if (real_ip >= 'A' && real_ip <= 'Z') {
							step = real_ip - 'A';
						}
						else {
							cout << "Illegal!" << endl;
							continue;
						}
						if (step == 25) {
							if (board_records.size() <= 0) {
								cout << "无法回滚！" << endl;
								continue;
							}
							rollback = true;
						}
						else if (step >= list.size()) {
							cout << "Illgeal!" << endl;
							continue;
						}
					}
					break;
				}
				if (rollback) {
					cout << "回滚成功！" << endl;
					delete current;
					current = board_records[board_records.size() - 1];
					board_records.pop_back();
					record.pop_back();
					record.pop_back();
					bot_moves.pop_back();
				}
				else {
					Board* b_copy = new Board(current);
					board_records.push_back(b_copy);
					current->new_step(list[step]);
					record.push_back(list[step]);
				}
			}
		}
		double value = current->get_self_value(false).second;
		cout << "你的最终得分：" << -value / FINAL_POWER << endl;
		cout << "棋谱：" << endl;
		for (int i = 0; i < record.size(); ++i) {
			short this_one = record[i];
			if (this_one == -1) cout << "- ,";
			else {
				char first = (this_one%BOARD_SIZE) + 'a';
				char second = (this_one / BOARD_SIZE) + '1';
				cout << first << second << ",";
			}
			if (i % 10 == 9) cout << endl;
		}
		for (int i = 0; i < board_records.size(); ++i) {
			update_struct(value / FINAL_POWER, board_records[i], bot_moves[i]);
			delete board_records[i];
		}
		struct_write();
		cout << endl;
		mode = -1;
		while (mode == -1) {
			cout << "是否重新开始？(0=否，1=是)";
			cin >> mode;
			if (mode < 0 || mode > 1) {
				cout << "Illegal!" << endl;
				if (cin.fail()) {
					cin.sync();
					cin.clear();
					cin.ignore();
				}
				mode = -1;
			}
		}
		if (mode == 0) break;
		cout << endl;
	}
	return 0;
}

void play_with_her(Hereditary* her_first, Hereditary* her_second, bool debug = false, bool first_struct = false, bool second_struct = false) {
	her_first->total_play += 2;
	her_second->total_play += 2;
	Board* first_board = new Board(her_first, true);
	Board* second_board = new Board(her_second, false);
	bool is_on_first = true;
	// play
	vector<Board*> first_records;
	vector<short> first_moves;
	vector<Board*> second_records;
	vector<short> second_moves;
	while (true) {
		if (debug) {
			cout << "--------------------------------------" << endl;
			first_board->print();
		}
		Board* current_board = (is_on_first) ? first_board : second_board;
		Board* next_board = (!is_on_first) ? first_board : second_board;
		vector<short> current_steps = current_board->next_possible();
		if (current_steps.size() != 0) {
			if (debug) {
				string piece = (is_on_first) ? BLACK_PIECE : WHITE_PIECE;
				cout << "当前执棋：" << piece << endl;
			}
			short step = current_board->bot_on_ideal_step(debug, (is_on_first && first_struct) || (!is_on_first && second_struct), true);
			next_board->new_step(step);
			Board* copy = new Board(current_board);
			if (is_on_first) {
				first_records.push_back(copy);
				first_moves.push_back(step);
			}
			else {
				second_records.push_back(copy);
				second_moves.push_back(step);
			}
		}
		else {
			// end?
			vector<short> next_steps = current_board->enemy_possible();
			if (next_steps.size() == 0) {
				break;
			}
			else {
				// no space to play
				// switch
				current_board->is_on_bot = !current_board->is_on_bot;
				next_board->is_on_bot = !next_board->is_on_bot;
				if (debug) {
					cout << "无子可下！" << endl;
				}
			}
		}
		is_on_first = !is_on_first;
	}
	// calculate score
	double first_score = first_board->get_self_value(first_struct).second / FINAL_POWER;
	double second_score = second_board->get_self_value(second_struct).second / FINAL_POWER;
	delete first_board;
	delete second_board;
	if (first_score * second_score > 0) {
		cout << "Error score!" << endl;
		system("pause");
		throw;
	}
	for (int i = 0; i < first_records.size(); ++i) {
		int step = first_moves[i];
		Board* board = first_records[i];
		update_struct(first_score, board, step);
		delete board;
	}
	for (int i = 0; i < second_records.size(); ++i) {
		int step = second_moves[i];
		Board* board = second_records[i];
		update_struct(second_score, board, step);
		delete board;
	}
	if (first_score > 0) {
		if (debug) {
			cout << "本局结束，黑子胜。比分：" << first_score << endl;
		}
		her_first->total_win += 2;
	}
	else if (second_score > 0) {
		if (debug) {
			cout << "本局结束，白子胜。比分：" << second_score << endl;
		}
		her_second->total_win += 2;
	}
	else {
		if (debug) {
			cout << "平局，" << endl;
		}
		her_first->total_win++;
		her_second->total_win ++;
	}
}

int hereditary() {
	int max_her = omp_get_max_threads() * HEREDITART_PER_THREAD;
	cout << "Create " << max_her << " hereditarys." << endl;
	stringstream ss;
	ofstream ofile;
	int file_index = 0;
	while (true) {
		ss << "wins_";
		ss << file_index++;
		ss << ".csv";
		string filename = ss.str();
		ss.clear();
		ss.str("");
		ifstream ifile(filename);
		bool isExist = ifile.is_open();
		ifile.close();
		if (!isExist) {
			ofile.open(filename);
			break;
		}
	}
	if (!ofile.is_open()) {
		cout << "Unable to write data!" << endl;
		ofile.close();
		return 0;
	}
	ofile << "Times,type,move_initial_power,move_diff_power,";
	ofile << "corner_power,edge_power,near_corner_power,near_edge_power,";
	ofile << "stable_power,fake_stable_power,unstable_power,";
	ofile << "non_current_move_power,importance_power,win_rate,total_battle" << endl;
	Hereditary* initial_her = get_hereditary();
	Hereditary* initial_her_2 = get_hereditary("Hereditary_t1.txt");

	// initial
	vector<Hereditary*> current_hereditary;
	current_hereditary.push_back(initial_her);
	current_hereditary.push_back(initial_her_2);
	for (int i = 1; i < max_her/2; ++i) {
		Hereditary* copy = new Hereditary(initial_her);
		copy->huge_variation();
		current_hereditary.push_back(copy);
	}
	for (int i = 1; i < max_her / 2; ++i) {
		Hereditary* copy = new Hereditary(initial_her_2);
		copy->huge_variation();
		current_hereditary.push_back(copy);
	}
	random_shuffle(current_hereditary.begin(), current_hereditary.end());
	// begin
	int loop_times = 0;
	while (loop_times++ < 10000) {
		cout << endl << "Running the " << loop_times << " times.." << endl;
		sort(current_hereditary.begin(), current_hereditary.end(), hereditary_cmp);
		// play with each other
		int rand_spector = rand() % (max_her / HEREDITART_PER_THREAD);
#pragma omp parallel for
		for (int i = 0; i < (max_her/ HEREDITART_PER_THREAD); ++i) {
			play_with_her(current_hereditary[i * 3], current_hereditary[i * 3 + 1], (i == rand_spector));
			play_with_her(current_hereditary[i * 3 + 1], current_hereditary[i * 3], (i == rand_spector));
			play_with_her(current_hereditary[i * 3 + 1], current_hereditary[i * 3 + 2], (i == rand_spector));
			play_with_her(current_hereditary[i * 3 + 2], current_hereditary[i * 3 + 1], (i == rand_spector));
			play_with_her(current_hereditary[i * 3], current_hereditary[i * 3 + 2], (i == rand_spector));
			play_with_her(current_hereditary[i * 3 + 2], current_hereditary[i * 3], (i == rand_spector));
		}
		// count
		sort(current_hereditary.begin(), current_hereditary.end(), hereditary_cmp);
		Hereditary* best_one = current_hereditary[0];
		ofile << loop_times << "," << best_one->type << ",";
		ofile << best_one->move_initial_power << "," << best_one->move_diff_power << ",";
		ofile << best_one->corner_power << "," << best_one->edge_power << "," << best_one->near_corner_power << "," << best_one->near_edge_power << ",";
		ofile << best_one->stable_power << "," << best_one->fake_stable_power << "," << best_one->unstable_power << ",";
		ofile << best_one->non_current_move_power << "," << best_one->importance_power << "," << best_one->win_rate() << "," << best_one->total_play << endl;
		// drop
		delete current_hereditary[max_her - 2];
		delete current_hereditary[max_her - 1];
		current_hereditary.pop_back();
		current_hereditary.pop_back();
		Hereditary* small_one = new Hereditary(best_one);
		Hereditary* huge_one = new Hereditary(best_one);
		small_one->small_variation();
		huge_one->huge_variation();
		current_hereditary.push_back(small_one);
		current_hereditary.push_back(huge_one);
		// update old ones
		for (int i = 0; i < current_hereditary.size(); ++i) {
			Hereditary* this_one = current_hereditary[i];
			if (this_one->total_play >= 40) {
				this_one->total_play = 0;
				this_one->total_win = 0;
			}
		}
	}
	
	ofile.close();
	system("pause");
	return 0;
}

int cvc() {
	cout << "1号电脑从Hereditary.txt中读取数据。" << endl;
	Hereditary* her_1 = new Hereditary();
	int mode = -1;
	while (true) {
		string filename;
		cout << "请输入2号电脑需要读取的数据所在的文件：";
		cin >> filename;
		Hereditary* her_2 = new Hereditary(filename);
		play_with_her(her_1, her_2, true, false, true);
		play_with_her(her_2, her_1, true, true, false);
		struct_write();
		cout << endl << "结果：\n1号电脑胜：" << her_1->total_win << endl << "2号电脑胜：" << her_2->total_win << endl;
		mode = -1;
		while (mode == -1) {
			cout << "是否重新开始？(0=否，1=是)";
			cin >> mode;
			if (mode < 0 || mode > 1) {
				cout << "Illegal!" << endl;
				if (cin.fail()) {
					cin.sync();
					cin.clear();
					cin.ignore();
				}
				mode = -1;
			}
		}
		her_1->total_play = 0;
		her_1->total_win = 0;
		delete her_2;
		if (mode == 0) break;
		cout << endl;
	}
	delete her_1;
	return 0;
}

int struct_train(Board* board, bool debug = true) {
	int times = 0;
	while (times++<10000) {
		vector<pair<Board*,short> > records;
		Board* run_board = new Board(board);
		while (true) {
			if (debug) {
				cout << "--------------------------------------" << endl;
				run_board->print();
				cout << "当前执子：" << ((run_board->is_bot_first ^ run_board->is_on_bot) ? WHITE_PIECE : BLACK_PIECE)  << endl;
				cout << "当前期望值：" << (run_board->calculate_struct()) << endl;
			}
			vector<short> current_steps = run_board->next_possible();
			if (current_steps.size() == 0) {
				// end?
				vector<short> next_steps = run_board->enemy_possible();
				if (next_steps.size() == 0) {
					break;
				}
				else {
					// no space to play
					// switch
					run_board->is_bot_first = !run_board->is_bot_first;
					run_board->all_reverse();
					if (debug) {
						cout << "无子可下！" << endl;
					}
					continue;
				}
			}
			short nextstep = run_board->bot_on_ideal_step(debug,true,true);
			Board* record = new Board(run_board);
			records.push_back(pair<Board*,short>(record,nextstep));
			run_board->all_reverse();
			run_board->is_bot_first = !run_board->is_bot_first;
			run_board->is_on_bot = !run_board->is_on_bot;
		}
		// true=white, false=black
		double value = run_board->get_self_value(false).second;
		bool winner = (run_board->is_bot_first ^ (value > 0));
		for (int i = 0; i < records.size(); ++i) {
			Board* record = records[i].first;
			// true=black, false=white
			bool current_color = (record->is_bot_first ^ record->is_on_bot);
			int isselfwin = (winner ^ current_color) ? 1 : -1;
			isselfwin *= abs(value);
			update_struct(isselfwin, record, records[i].second);
			delete record;
		}
		struct_write();
		delete run_board;
	}
	return 0;
}

int struct_train_main() {
	int thread_size = omp_get_max_threads() / 2;
	int times = 0;
	while (times++ < 10000) {
		int rand_spector = rand() % (thread_size);
		Board* run_board = new Board(true);
#pragma omp parallel for
		for (int i = 0; i < thread_size; ++i) {
			struct_train(run_board, i == rand_spector);
		}
		struct_write();
		delete run_board;
	}
	return 0;
}

int main(int argc, char *argv[]) {
	srand(time(NULL));
	int mode = -1;
	if (argc > 1) {
		mode = atoi(argv[1]);
	}
	while (mode == -1) {
		cout << "请选择模式(0=训练, 1=人机，2=电脑对战,3):";
		cin >> mode;
		if (mode < 0 || mode > 3) {
			cout << "Illegal input!" << endl;
			if (cin.fail()) {
				cin.sync();
				cin.clear();
				cin.ignore();
			}
			mode = -1;
		}
	}
	if (mode == 0) {
		hereditary();
	}
	else if (mode ==1) {
		pvc();
	}
	else if (mode == 2) {
		cvc(); 
	}
	else {
		struct_train_main();
	}
	return 0;
}