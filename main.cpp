#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <algorithm>
using namespace std;

#define BOT_PIECE 1
#define PLAYER_PIECE -1
#define EMPTY_PIECE 0

#define BOARD_SIZE 8
#define FINAL_POWER 10000

#define SEARCH_MAX_DEPTH 7 // search depth
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

template<typename T> int find_in_vector(vector<T> v, T f) {
	for (int i = 0; i < v.size(); ++i)
		if (v[i] == f) return i;
	return -1;
}

struct Board {
	double alpha;
	double beta;
	short board[BOARD_SIZE * BOARD_SIZE];
	bool is_bot_first;
	bool is_on_bot;
	Board* last_board;
	int search_depth;
	int unmoveable_count;
	vector<short> except_steps;

	Board(bool bot_first = false) {
		alpha = SHRT_MIN;
		beta = SHRT_MAX;
		is_on_bot = bot_first;
		is_bot_first = bot_first;
		last_board = NULL;
		for (int i = 0; i < BOARD_SIZE*BOARD_SIZE; ++i) board[i]=EMPTY_PIECE;
		short black = bot_first ? BOT_PIECE : PLAYER_PIECE;
		short white = bot_first ? PLAYER_PIECE : BOT_PIECE;
		short upleft = BOARD_SIZE / 2 - 1;
		onboard(upleft, upleft) = black;
		onboard(upleft+1, upleft+1) = black;
		onboard(upleft, upleft+1) = white;
		onboard(upleft+1, upleft) = white;
		search_depth = -1;
		unmoveable_count = 0;
	}
	Board(Board* b) {
		alpha = b->alpha;
		beta = b->beta;
		memcpy_s(board, BOARD_SIZE * BOARD_SIZE * sizeof(short), b->board, BOARD_SIZE * BOARD_SIZE * sizeof(short));
		is_on_bot = b->is_on_bot;
		is_bot_first = b->is_bot_first;
		last_board = b;
		search_depth = b->search_depth;
		unmoveable_count = b->unmoveable_count;
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
	void bot_on_idel_step() {
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
		cout << "电脑认为能够达到的期望值：" << expection.second << endl;
		int next_step_id = next_steps[expection.first];
		cout << "电脑下子：(" << 1+next_step_id % BOARD_SIZE << ", " << 1 + next_step_id / BOARD_SIZE << ")。" << endl;
		new_step(next_step_id);
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
				(is_on_bot && result.second < next_value.second) ||
					(!is_on_bot && result.second > next_value.second)) )
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
			double importance = 0.4;
			if ((_x == 0 || _x == BOARD_SIZE - 1) && (_y == 0 || _y == BOARD_SIZE - 1)) {
				importance *= 5;
			}
			else  {
				int to_edge_x = min(abs(_x - 1), abs(_x - (BOARD_SIZE-1)));
				int to_edge_y = min(abs(_y - 1), abs(_y - (BOARD_SIZE - 1)));
				// near corner
				if (to_edge_x < 2 && to_edge_y < 2) {
					importance *= 0.1;
				}
				// near the edge
				else if (to_edge_x < 2 || to_edge_y < 2) {
					importance *= 0.5;
				}
				// at the edge
				else if (_x == 0 || _x == BOARD_SIZE - 1 || _y == 0 || _y == BOARD_SIZE - 1) {
					importance *= 3;
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
				importance *= 0.1;
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
				importance *= 5;
			}
			else if (is_fake_stable) {
				importance *= 2;
			}
			if (this_piece == BOT_PIECE) {
				bot_value += this_piece * pow(1.5, importance);
			}
			else {
				player_value += this_piece * pow(1.5, importance);
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
			player_move_power *= 0.9;
		}
		else {
			player_move_power = next_possible().size(); 
			is_on_bot = !is_on_bot;
			bot_move_power = next_possible().size();
			is_on_bot = !is_on_bot;
			bot_move_power *= 0.9;
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
		else {
			return result;
		}
	}
};

int main() {
	while (true) {
		int mode = -1;
		while (mode == -1) {
			cout << "请选择先后(0=先手, 1=后手):";
			cin >> mode;
			if (mode < 0 || mode > 1) {
				cout << "Illegal!" << endl;
				mode = -1;
			}
		}
		string your = (mode) ? "(白子○)" : "(黑子●)";
		Board* current = new Board(mode == 1);
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
					current->is_on_bot = !current->is_on_bot;
					continue;
				}
			}
			current->calculate_current_value(true);
			if (current->is_on_bot) {
				cout << "电脑思考中..." << endl;
				current->bot_on_idel_step();
			}
			else {
				int step;
				string ip;
				while (true) { 
					cout << "轮到你" << your << "下棋，请输入对应落位的字母：";
					cin >> ip;
					if (ip.size() > 1) {
						cout << "Illgeal!" << endl;
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
			}

		}
		double value = current->get_self_value().second;
		cout << "你的最终得分：" << -value / FINAL_POWER << endl;
		mode = -1;
		while (mode == -1) {
			cout << "是否重新开始？(0=否，1=是)";
			cin >> mode;
			if (mode < 0 || mode > 1) {
				cout << "Illegal!" << endl;
				mode = -1;
			}
		}
		if (mode == 0) break;
		cout << endl;
	}
	return 0;
}