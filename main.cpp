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

#define BOARD_SIZE 6
#define FINAL_POWER 100
#define HEREDITART_PER_THREAD 3

#define SEARCH_MAX_DEPTH 8 // search depth
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

	Hereditary() {
		ifstream ifile;
		ifile.open("Hereditary.txt");
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
		total_win = 0;
		total_play = 0;
	}

	// 0.8~1.25
	void small_variation() {
		variation(0.8, 1.25);
	}

	// 0.5~2
	void huge_variation() {
		variation(0.5, 2);
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
	}

	double win_rate() {
		if (total_play == 0) {
			throw;
		}
		return (double)total_win / (double)total_play;
	}
};

bool hereditary_cmp(Hereditary* h1, Hereditary* h2) {
	return h1->win_rate() > h2->win_rate();
};

Hereditary* get_hereditary() {
	Hereditary* initial_her = new Hereditary();
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
	vector<short> except_steps;

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
		onboard(upleft, upleft) = black;
		onboard(upleft + 1, upleft + 1) = black;
		onboard(upleft, upleft + 1) = white;
		onboard(upleft + 1, upleft) = white;
		search_depth = -1;
		herediatry = her;
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
	// x,y's range: 0~5
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
		string bot_piece = (is_bot_first) ? "●" : "○";
		string player_piece = (is_bot_first) ? "○" : "●";
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
	short bot_on_idel_step(bool debug = true) {
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
		pair<int, double> expection = get_self_value();
		short next_step_id = next_steps[expection.first];
		if (debug) {
			cout << "电脑认为能够达到的期望值：" << expection.second << endl;
			cout << "电脑下子：(" << 1 + next_step_id % BOARD_SIZE << ", " << 1 + next_step_id / BOARD_SIZE << ")。" << endl;
		}
		new_step(next_step_id);
		return next_step_id;
	}
	// <from which children(index of vector), value>
	pair<int,double> get_self_value() {
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
			double value = calculate_current_value();
			return pair<int, double>(-1, value);
		}		
		for (int i = 0; i < next_steps.size(); ++i) {
			Board* next_board = new Board(this);
			next_board->new_step(next_steps[i]);
			next_board->search_depth--;
			pair<int,double> next_value = next_board->get_self_value();
			delete next_board;
			if ((result.first != -2) && 
				((result.first == -1) || 
				(result.second < next_value.second && is_on_bot) ||
					(result.second > next_value.second && !is_on_bot)) )
			{
				result.first = i;
				result.second = next_value.second;
				// alpha update
				if ((alpha < result.second) && is_on_bot) {
					alpha = result.second;
				}
				// beta update
				else if ((beta > result.second) && !is_on_bot) {
					beta = result.second;
				}
				// cut
			}
			if (alpha > beta) {
				result.first = -2;
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
					if (piece_check == 1) {
						// self
						check_x += find_dir[i][0];
						check_y += find_dir[i][1];
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
				check_x = _x - find_dir[i][0];
				check_y = _y - find_dir[i][1];
				while (check_x >= 0 && check_x < BOARD_SIZE && check_y >= 0 && check_y < BOARD_SIZE) {
					short piece_check = board[check_y * BOARD_SIZE + check_x] * this_piece;
					if (piece_check == 1) {
						// self
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
		double total_power = player_move_power + bot_move_power;
		double power = MOVE_INITIAL_POWER * (1 - pow(not_empty_count, 3) / pow(BOARD_SIZE, 6));
		bot_move_power = pow(1 + (MOVE_DIFF_POWER - 1)*(bot_move_power / total_power), power);
		player_move_power = pow(1 + (MOVE_DIFF_POWER - 1)*(player_move_power / total_power), power);
		double _temp = bot_move_power + player_move_power;
		double result = bot_value * (bot_move_power / _temp) + player_value * (player_move_power / _temp);
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
};

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
		string your = (mode) ? "(白子○)" : "(黑子●)";
		Board* current = new Board(initial_her,mode == 1);
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
			current->calculate_current_value(true);
			if (current->is_on_bot) {
				cout << "电脑思考中..." << endl;
				short this_record = current->bot_on_idel_step();
				record.push_back(this_record);
			}
			else {
				int step;
				string ip;
				while (true) { 
					cout << "轮到你" << your << "下棋，请输入对应落位的字母：";
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
						if (step >= list.size()) {
							cout << "Illgeal!" << endl;
							continue;
						}
					}
					break;
				}
				current->new_step(list[step]);
				record.push_back(list[step]);
			}

		}
		double value = current->get_self_value().second;
		cout << "你的最终得分：" << -value / FINAL_POWER << endl;
		cout << "棋谱：";
		for (int i = 0; i < record.size(); ++i) {
			short this_one = record[i];
			if (this_one == -1) cout << "-,";
			else {
				char first = (this_one%BOARD_SIZE) + 'a';
				char second = (this_one / BOARD_SIZE) + '0';
				cout << first << second << ",";
			}
			if (i % 10 == 9) cout << endl;
		}
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

void play_with_her(Hereditary* her_first, Hereditary* her_second, bool debug = false) {
	her_first->total_play += 2;
	her_second->total_play += 2;
	Board* first_board = new Board(her_first, true);
	Board* second_board = new Board(her_second, false);
	bool is_on_first = true;
	// play
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
				string piece = (is_on_first) ? "●" : "○";
				cout << "当前执棋：" << piece << endl;
			}
			short step = current_board->bot_on_idel_step(debug);
			next_board->new_step(step);
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
	double first_score = first_board->get_self_value().second / FINAL_POWER;
	double second_score = second_board->get_self_value().second / FINAL_POWER;
	delete first_board;
	delete second_board;
	if (first_score * second_score > 0) {
		cout << "Error score!" << endl;
		system("pause");
		throw;
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
	stringstream ss;
	ss << "wins_";
	ss << (rand() % 10000);
	ss << ".csv";
	string filename = ss.str();
	ss.clear();
	ss.str("");
	ofstream ofile(filename);
	if (!ofile.is_open()) {
		cout << "Unable to write data!" << endl;
		ofile.close();
		return 0;
	}
	ofile << "Times,move_initial_power,move_diff_power,";
	ofile << "corner_power,edge_power,near_corner_power,near_edge_power,";
	ofile << "stable_power,fake_stable_power,unstable_power,";
	ofile << "non_current_move_power,importance_power,win_rate,total_battle" << endl;
	Hereditary* initial_her = get_hereditary();

	// initial
	vector<Hereditary*> current_hereditary;
	current_hereditary.push_back(initial_her);
	for (int i = 1; i < max_her; ++i) {
		Hereditary* copy = new Hereditary(initial_her);
		copy->huge_variation();
		current_hereditary.push_back(copy);
	}
	// begin
	int loop_times = 0;
	Hereditary* last_best = NULL;
	while (loop_times++ < 10000) {
		cout << endl << "Running the " << loop_times << "times.." << endl;
		random_shuffle(current_hereditary.begin(), current_hereditary.end());
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
		if (best_one != last_best) {
			last_best = best_one;
		}
		ofile << loop_times << ",";
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
			if (this_one->total_play >= 60) {
				this_one->total_play = 0;
				this_one->total_win = 0;
			}
		}
	}
	
	ofile.close();
	system("pause");
	return 0;
}

int main() {
	srand(time(NULL));
	int mode = -1;
	while (mode == -1) {
		cout << "请选择模式(0=训练, 1=人机):";
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
	if (mode == 0) {
		hereditary();
	}
	else {
		pvc();
	}
	
	return 0;
}