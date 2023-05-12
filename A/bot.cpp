#include<bits/stdc++.h>
using namespace std;

std::mt19937 mt_rng(std::chrono::steady_clock::now().time_since_epoch().count());

template <typename _Tp>
_Tp rand(_Tp l, _Tp r) {
	assert(l <= r);
	return std::uniform_int_distribution <_Tp> (l, r) (mt_rng);
}

struct Bot {
	int x, y;
	char name;

	Bot() : x(-1), y(-1), name('E') {}
	Bot(int x, int y, char name) : x(x), y(y), name(name) {}
	//in case default constructor is being cucked
	Bot(const Bot& bot) : x(bot.x), y(bot.y), name(bot.name) {}
};

int n, m, k, p; //board thinggy

//Moving direction
const int nX[] = { -1, 0, 1, 0, 0 };
const int nY[] = { 0, 1, 0, -1, 0 };

//Team bot
Bot myBot;
int lastMov;
int lastX, lastY;
int area_length = 3;

//Target surrounding area
int tarX, tarY;

//Other targets
int num_target;
vector<Bot> tar;

//Board state
vector<vector<char>> board;

int get_border_srink() {
	return p / k - (p % k == 0);
}

bool check_shrink(const Bot& bot) {
	int border = get_border_srink();
	if (bot.x == border || bot.x == (n - border - 1) ||
		bot.y == border || bot.y == (m - border - 1))
		return (p % k != 0);

	return true;
}

bool check_range(const Bot& bot) {
    if (0 > bot.x || bot.x > n - 1 || 0 > bot.y || bot.y > m - 1) {
        return false;
    }

    assert(0 <= bot.x && bot.x <= n - 1 && 0 <= bot.y && bot.y <= m - 1);
	bool check = true;
	int border = get_border_srink();
	check = check && (bot.x >= get_border_srink()) && (bot.x < (n - get_border_srink())); //x edges
	check = check && (bot.y >= get_border_srink()) && (bot.y < (m - get_border_srink())); //y edges
	check = check && (board[bot.x][bot.y] != '#'); //ban tiles
	for (char i = 'a'; i <= 'd'; i++) {
		check = check && (board[bot.x][bot.y] != i); //tiles that have been captured, got shrinked
	}
	check = check && check_shrink(bot); //die because of shrinking
	return check;
}

int get_dist_from_border(int x, int y) {
    assert(0 <= x && x <= n - 1 && 0 <= y && y <= m - 1);
	int border = get_border_srink();
	return min(x - border, n - border - x) + min(y - border, m - border - y);
}

int get_dist_from_cur(int x, int y) {
    assert(0 <= x && x <= n - 1 && 0 <= y && y <= m - 1);
	return abs(x - myBot.x) + abs(y - myBot.y);
}

int get_dist_from_point(int a, int b, int x, int y) {
    assert(0 <= x && x <= n - 1 && 0 <= y && y <= m - 1);
	return abs(x - a) + abs(y - b);
}

int count_death(int x, int y) {
    assert(0 <= x && x <= n - 1 && 0 <= y && y <= m - 1);
	int res = 0;
	int border = get_border_srink();
	for (int i = x - 2; i <= x; i++) {
		for (int j = y - 2; j <= y; j++) {
			if (!check_range(Bot(i, j, myBot.name))) {
				res++;
			}
		}
	}
	return res;
}

//find the nearest tiles that haven't been captured/captured by others player that's also save to move to
//(outside of the 9 tiles surround player)
Bot bfs_move() {
	vector<Bot> pot_mov;
	vector<vector<bool>> pass_tiles;
	pass_tiles.resize(n, vector<bool>(m, false));

	queue<Bot> nxt;
	nxt.push(myBot);
	pass_tiles[myBot.x][myBot.y] = true;

	while (!nxt.empty()) {
		Bot cur = nxt.front();
		nxt.pop();

		for (int i = 0; i < 4; i++) {
			int X = cur.x + nX[i];
			int Y = cur.y + nY[i];

			if (check_range(Bot(X, Y, myBot.name)) && !pass_tiles[X][Y]) {
				pass_tiles[X][Y] = true;

				if (board[X][Y] != myBot.name) {
					pot_mov.push_back(Bot(X, Y, myBot.name));
					continue;
				}

				nxt.push(Bot(X, Y, myBot.name));
			}
		}
	}

	Bot res = Bot();
	for (int i = 0; i < (int)pot_mov.size(); i++) {
		Bot tmp = pot_mov[i];
		if (res.x == -1) {
			res = tmp;
			continue;
		}

		//safer option first
		int border = get_border_srink();
		if ((min(res.x - border, n - res.x - border) > min(tmp.x - border, n - tmp.x - border)) &&
			(min(res.y - border, m - res.y - border) > min(tmp.y - border, m - tmp.y - border))
		 ) {
			res = tmp;
			continue;
		}
		//aggresive option
		else if (get_dist_from_cur(tmp.x, tmp.y) < get_dist_from_cur(res.x, res.y)) {
			res = tmp;
			continue;
		}
		//enemy tiles
		else if (
			((board[res.x][res.y] == '.') || board[res.x][res.y] == myBot.name) &&
			(board[tmp.x][tmp.y]) >= 'A' && (board[tmp.x][tmp.y] <= 'D') && board[tmp.x][tmp.y] != myBot.name)
		{
			res = tmp;
		}
	}

	return res;
}

void move(ofstream& fo, ofstream& dat_out) {
	//prioritize not captured tile
	if (lastMov >= 0 && lastMov <= 3 &&
		check_range(Bot(myBot.x + nX[lastMov], myBot.y + nY[lastMov], myBot.name)) &&
		(board[myBot.x + nX[lastMov]][myBot.y + nY[lastMov]] != myBot.name) &&
		get_dist_from_border(myBot.x + nX[lastMov], myBot.y + nY[lastMov]) >= ((m + n - (p % (k - 1)) - 2 * get_border_srink()) / 3)
	)
	{
		int X = myBot.x + nX[lastMov];
		int Y = myBot.y + nY[lastMov];
		if (check_range(Bot(X, Y, myBot.name))) {
			fo << X << ' ' << Y << '\n';
			dat_out << lastMov << '\n';
			dat_out << myBot.x << ' ' << myBot.y << '\n';
			return;
		}
	}

	//surrounding 4 tiles
	for (int i = (lastMov + 1) % 4; i != lastMov; i = (i + 1) % 4) {
		int X = myBot.x + nX[i];
		int Y = myBot.y + nY[i];

		if (check_range(Bot(X, Y, myBot.name)) && board[X][Y] != myBot.name) {
			fo << X << ' ' << Y << '\n';
			dat_out << i << '\n';
			dat_out << myBot.x << ' ' << myBot.y << '\n';
			return;
		}
	}

	//get backward move
	int backward_move = -1;
	for (int i = 0; i < 4; i++) {
		int X = myBot.x + nX[i];
		int Y = myBot.y + nY[i];

		//avoid going back if half of the map is still around
		if (check_range(Bot(X, Y, myBot.name)) && X == lastX && Y == lastY) {
			backward_move = i;
			continue;
		}
	}

	//desperate move no.1
	Bot res = bfs_move();
	if (res.x != -1) {
		int mov = -1;
		for (int i = 3; i >= 0; i--) {
			if (i == backward_move) continue; //avoid going back if possible

			int X = myBot.x + nX[i];
			int Y = myBot.y + nY[i];

			if (check_range(Bot(X, Y, myBot.name))) {
				if (mov == -1) {mov = i; continue;}
				if (get_dist_from_point(X, Y, res.x, res.y) < get_dist_from_point(myBot.x + nX[mov], myBot.y + nY[mov], res.x, res.y)) {
					mov = i;
				}
				else if (get_dist_from_point(X, Y, res.x, res.y) == get_dist_from_point(myBot.x + nX[mov], myBot.y + nY[mov], res.x, res.y))
				{
					if (count_death(X, Y) < count_death(myBot.x + nX[mov], myBot.y + nY[mov])) {
						mov = i;
					}
				}
			}

		}
		if (mov != -1) {
			fo << myBot.x + nX[mov] << ' ' << myBot.y + nY[mov] << '\n';
			dat_out << mov << '\n';
			dat_out << myBot.x << ' ' << myBot.y << '\n';
			return;
		}
	}

	//backward case
	if (backward_move != -1 && check_range(Bot(myBot.x + nX[backward_move], myBot.y + nY[backward_move], myBot.name))) {
		fo << myBot.x + nX[backward_move] << ' ' << myBot.y + nY[backward_move] << '\n';
		dat_out << backward_move << '\n';
		dat_out << myBot.x << ' ' << myBot.y << '\n';
		return;
	}

	//het cuu
	//cout << "Cau cuu hai cau\n";
	int cau_cuu_ngai_hai_cau = 4;
	dat_out << cau_cuu_ngai_hai_cau << '\n';
	dat_out << myBot.x << ' ' << myBot.y << '\n';
	fo << myBot.x + nX[cau_cuu_ngai_hai_cau] << ' ' << myBot.y + nY[cau_cuu_ngai_hai_cau];
}

int rand_offset(int base) { //offset from center
	bool neg = rand <int> (0, 1);
	int val = 100;
	while (base / 2 + (((neg == 1) ? -1 : 1) * val) <= 1 || base / 2 + (((neg == 1) ? -1 : 1) * val) >= base - 1) {
		val = rand <int> (2, 4);
	}
	return base / 2 + ((neg == 1)? -1 : 1) * val;
}

int main() {
	ifstream fi;
	fi.open("MAP.INP");

	ofstream fo("MOVE.OUT");
	fi >> n >> m >> k >> p;

	ifstream dat_input("STATE.DAT");
	if (!(dat_input >> lastMov)) {
		//cout << "Failed to read yo shit\n";
		lastMov = -1;
		lastX = lastY = 0;
	}
	else {
		area_length = 4;

		//panic mode
		if (area_length > 3 && (min(n, m) - get_border_srink() <= min(n, m) / 2)) {
			area_length = 3;
		}

		if ((p % area_length == 0) || (lastMov == 4)) {
			if (lastMov == 4) {
				lastMov = 0;
			}
			else {
				lastMov = (lastMov + 1) % 4; //just in case it stuck in one direction for too long
			}
		}

		dat_input >> lastX >> lastY;
	}
	dat_input.close();

	ofstream dat_output("STATE.DAT");

	int x, y;
	char name;
	fi >> x >> y >> name;
	myBot = Bot(x, y, name);

	fi >> num_target;
	for (int i = 0; i < num_target; i++) {
		fi >> x >> y >> name;
		tar.push_back(Bot(x, y, name));
	}

	board.resize(n, vector<char>(m, '.'));
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			fi >> board[i][j];
		}
	}
	fi.close();

	if (p == 0) {
		do {
			x = rand_offset(n); y = rand_offset(m);
		} while (!check_range(Bot(x, y, name)));
		fo << x << ' ' << y << '\n';
		dat_output << 4 << '\n';
		dat_output << x << ' ' << y << '\n';
	}
	else if (myBot.x == -1) {

	}
	else {
		move(fo, dat_output);
	}

	fo.close();
	dat_output.close();
	return 0;
}
