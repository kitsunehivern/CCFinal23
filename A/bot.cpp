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
};

int n, m, k, p; //board thinggy

//Moving direction
const int nX[] = { -1, 0, 1, 0, 0 };
const int nY[] = { 0, 1, 0, -1, 0 };

//Team bot
Bot myBot;
int lastMov;
int lastX, lastY;

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

bool checkShrink(const Bot& bot) {
	int border = get_border_srink();
	if (bot.x == border || bot.x == (n - border - 1) ||
		bot.y == border || bot.y == (m - border - 1))
		return (p % k == 0);

	return true;
}

bool checkRange(const Bot& bot) {
	bool check = true;
	int border = get_border_srink();
	check = check && (bot.x >= get_border_srink()) && (bot.x < (n - get_border_srink())); //x edges
	check = check && (bot.y >= get_border_srink()) && (bot.y < (m - get_border_srink())); //y edges
	check = check && (board[bot.x][bot.y] != '#'); //ban tiles
	for (char i = 'a'; i <= 'd'; i++) {
		check = check && (board[bot.x][bot.y] != i); //tiles that have been captured, got shrinked
	}
	check = check && checkShrink(bot); //die because of shrinking
	return check;
}

void move(ofstream& fo, ofstream& dat_out) {
	int mov = 0;
	if (lastMov >= 0 && checkRange(Bot(myBot.x + nX[lastMov], myBot.y + nY[lastMov], myBot.name))) {
		dat_out << lastMov << '\n';
		dat_out << myBot.x << ' ' << myBot.y << '\n';
		fo << myBot.x + nX[lastMov] << ' ' << myBot.y + nY[lastMov];
		return;
	}
	for (int i = (lastMov + 1) % 4; i != lastMov; i = (i + 1) % 4) {
		if (myBot.x + nX[i] == lastX && myBot.y + nY[i] == lastY) { mov = i; continue; } //avoid going back
		if (checkRange(Bot(myBot.x + nX[i], myBot.y + nY[i], myBot.name))) {
			dat_out << i << '\n';
			dat_out << myBot.x << ' ' << myBot.y << '\n';
			fo << myBot.x + nX[i] << ' ' << myBot.y + nY[i];
			return;
		}
	}

	if (checkRange(Bot(lastX, lastY, myBot.name))) { //last resort
		dat_out << mov << '\n';
		dat_out << myBot.x << ' ' << myBot.y << '\n';
		fo << lastX << ' ' << lastY;
		return;
	}

	//stand still
	dat_out << 4 << '\n';
	dat_out << myBot.x << ' ' << myBot.y << '\n';
	fo << myBot.x << ' ' << myBot.x;
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
		if ((p % 2 == 0) || (lastMov == 4)) {
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
		int x = rand <int> (0, n - 1), y = rand <int> (0, m - 1);
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
