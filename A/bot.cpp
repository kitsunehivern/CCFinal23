#include<iostream>
#include<fstream>
#include<vector>
#include<ctime>
#include<queue>
using namespace std;

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
	int border = get_border_srink();
	return (x - border) + (n - border - x) + (y - border) + (m - border - y);
}

int get_dist_from_cur(int x, int y) {
	return abs(x - myBot.x) + abs(y - myBot.y);
}

//find the nearest tiles that haven't been captured/captured by others player that's also save to move to
//(outside of the 9 tiles surround player)
Bot bfs_move() {
	vector<Bot> pot_mov;
	vector<vector<bool>> pass_tiles;
	pass_tiles.resize(n);
	for (int i = 0; i < n; i++) {
		pass_tiles[i].resize(m);
	}
	queue<Bot> nxt;
	nxt.push(myBot);

	while (!nxt.empty()) {
		Bot cur = nxt.front();
		nxt.pop();

		for (int i = 0; i < 4; i++) {
			int X = cur.x + nX[i];
			int Y = cur.y + nY[i];

			if (check_range(Bot(X, Y, myBot.name)) && !pass_tiles[X][Y]) {
				pass_tiles[X][Y] = true;

				if (board[X][Y] != 'A' + (myBot.name - 'A')) {
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
		if (get_dist_from_border(tmp.x, tmp.y) < get_dist_from_border(res.x, res.y)) {
			res = tmp;
			continue;
		}
		//aggresive option
		if (get_dist_from_cur(tmp.x, tmp.y) < get_dist_from_cur(res.x, res.y)) {
			res = tmp;
			continue;
		}

		//enemy tiles
		if (board[tmp.x][tmp.y] >= 'A' && board[tmp.x][tmp.y] <= 'D') {
			res = tmp;
		}
	}

	return res;
}

void move(ofstream& fo, ofstream& dat_out) {
	int backward_move = -1;

	//prioritize not captured tile
	if (lastMov >= 0 && lastMov <= 3 &&
		check_range(Bot(myBot.x + nX[lastMov], myBot.y + nY[lastMov], myBot.name)) &&
		(board[myBot.x + nX[lastMov]][myBot.y + nY[lastMov]] != 'A' + (myBot.name - 'A')))
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
	
	for (int i = (lastMov + 1) % 4; i != lastMov; i = (i + 1) % 4) {
		int X = myBot.x + nX[i];
		int Y = myBot.y + nY[i];

		if (check_range(Bot(X, Y, myBot.name)) && board[X][Y] != 'A' - (myBot.name - 'A')) {
			fo << X << ' ' << Y << '\n';
			dat_out << i << '\n';
			dat_out << myBot.x << ' ' << myBot.y << '\n';
			return;
		}
	}

	//prioritize gtfo of the border
	int nxt_mov = -1;
	for (int i = 0; i < 4; i++) {
		int X = myBot.x + nX[i];
		int Y = myBot.y + nY[i];

		//prioritize furthest from border
		if (check_range(Bot(X, Y, myBot.name)) && 
			((nxt_mov == -1) || 
			(get_dist_from_border(X, Y) > get_dist_from_border(myBot.x + nX[nxt_mov], myBot.y + nY[nxt_mov]))
			))
		{

			//avoid going back if half of the map is still around
			if (X == lastX && Y == lastY) {
				backward_move = i;
				continue;
			}

			nxt_mov = i;
		}
	}
	
	//haven't cover the backward case in here
	if (nxt_mov != -1) {
		fo << myBot.x + nX[nxt_mov] << ' ' << myBot.y + nY[nxt_mov] << '\n';
		dat_out << nxt_mov << '\n';
		dat_out << myBot.x << ' ' << myBot.y << '\n';
		return;
	}

	//desperate move no.1
	Bot res = bfs_move();
	if (res.x != -1) {
		int mov = 3;
		for (int i = 3; i >= 0; i--) {
			
			int X = myBot.x + nX[i];
			int Y = myBot.y + nY[i];
 
			if (check_range(Bot(X, Y, myBot.name)) && 
				get_dist_from_cur(res.x - nX[i], res.y - nY[i]) < 
				get_dist_from_cur(res.x - nX[mov], res.y - nY[mov])
				) {
				mov = i;
			}
		}
		fo << myBot.x + nX[mov] << ' ' << myBot.y + nY[mov] << '\n';
		dat_out << mov << '\n';
		dat_out << myBot.x << ' ' << myBot.y << '\n';
		return;
	}

	//backward case
	if ((min(n, m) - get_border_srink() <= min(n, m) / 2) && backward_move != -1) {
		fo << myBot.x + nX[backward_move] << ' ' << myBot.y + nY[backward_move] << '\n';
		dat_out << nxt_mov << '\n';
		dat_out << myBot.x << ' ' << myBot.y << '\n';
		return;
	}

	//het cuu
	dat_out << 4 << '\n';
	dat_out << myBot.x << ' ' << myBot.y << '\n';
	fo << myBot.x << ' ' << myBot.x;
}

int rand_offset(int base) { //offset from center
	bool neg = rand() % 2;
	int val = 100;
	while (base / 2 + (((neg == 1) ? -1 : 1) * val) <= 1 || base / 2 + (((neg == 1) ? -1 : 1) * val) >= base - 1) {
		val = rand() % 4;
	}
	return base / 2 + ((neg == 1)? -1 : 1) * val;
}

int main() {
	srand(time(NULL));
	ifstream fi;
	fi.open("MAP.INP");

	ofstream fo;
	fo.open("move.out");
	fi >> n >> m >> k >> p;

	ifstream dat_input;
	dat_input.open("state.dat");
	if (!(dat_input >> lastMov)) {
		//cout << "Failed to read yo shit\n";
		lastMov = -1;
		lastX = lastY = 0;
	}
	else {
		area_length = 3;
		//cout << "Succ to read yo shit\n";
		if ((p % area_length == 0) || (lastMov == 4)) {
			if (lastMov == 4) {
				lastMov = 0;
			}
			else {
				lastMov = (lastMov + 1) % 4; //just in case it stuck in one direction for too long
			}
		}

		//panic mode
		if (area_length > 2 && (min(n, m) - get_border_srink() <= min(n, m) / 3)) {
			area_length = 2;
		}
		
		dat_input >> lastX >> lastY;
	}
	dat_input.close();

	ofstream dat_output;
	dat_output.open("state.dat", ofstream::out | ofstream::trunc);

	int x, y;
	char name;
	fi >> x >> y >> name;
	myBot = Bot(x, y, name);

	fi >> num_target;
	for (int i = 0; i < num_target; i++) {
		fi >> x >> y >> name;
		tar.push_back(Bot(x, y, name));
	}

	board.resize(n);
	for (int i = 0; i < n; i++) {
		board[i].resize(m);
		for (int j = 0; j < m; j++) {
			fi >> board[i][j];
		}
	}

	if (myBot.x < 0) {
		do {
			x = rand_offset(n); y = rand_offset(m);
		} while (!check_range(Bot(x, y, name)));
		fo << x << ' ' << y << '\n';
		dat_output << 4 << '\n';
		dat_output << x << ' ' << y << '\n';
	}
	else {
		move(fo, dat_output);
	}

	fi.close();
	fo.close();
	dat_output.close();
	return 0;
}