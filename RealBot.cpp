#include<iostream>
#include<fstream>
#include<vector>
#include<ctime>
using namespace std;

struct Bot {
	int x, y;
	char name;

	Bot() : x(-1), y(-1), name('E') {}
	Bot(int x, int y, char name) : x(x), y(y), name(name) {}
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
		return (p % k == 0);

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

void move(ofstream& fo, ofstream& dat_out) {
	int backward_move = 0;

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

		//avoid going back if half of the map is still around
		if (X == lastX && Y == lastY && (min(n, m) - get_border_srink() <= min(n, m) / 2)) {
			backward_move = i;
			continue;
		}

		if (check_range(Bot(X, Y, myBot.name)) && 
			((nxt_mov == -1) || 
			(get_dist_from_border(X, Y) > get_dist_from_border(myBot.x + nX[nxt_mov], myBot.y + nY[nxt_mov]))
			))
		{
			nxt_mov = i;
		}
	}
	
	//cover the backward case in here already
	if (nxt_mov != -1) {
		fo << myBot.x + nX[nxt_mov] << ' ' << myBot.y + nY[nxt_mov] << '\n';
		dat_out << nxt_mov << '\n';
		dat_out << myBot.x << ' ' << myBot.y << '\n';
		return;
	}

	//stand still
	dat_out << 4 << '\n';
	dat_out << myBot.x << ' ' << myBot.y << '\n';
	fo << myBot.x << ' ' << myBot.x;
}

int rand_offset(int base) { //offset from center
	bool neg = rand() % 2;
	int val = 100;
	while (base / 2 + (((neg == 1) ? -1 : 1) * val) > 1 && base / 2 + (((neg == 1) ? -1 : 1) * val) < base - 1) {
		val = rand() % 4;
	}
	return val;
}

int main() {
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
		//cout << "Succ to read yo shit\n";
		if ((p % area_length == 0) || (lastMov == 4)) {
			if (lastMov == 4) {
				lastMov = 0;
			}
			else {
				lastMov = (lastMov + 1) % 4; //just in case it stuck in one direction for too long
			}
		}

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
		srand(time(NULL));
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