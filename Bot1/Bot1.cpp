#include<iostream>
#include<fstream>
#include<vector>
using namespace std;

struct Bot {
	int x, y;
	char name;

	Bot(): x(-1), y(-1), name('E') {}
	Bot(int x, int y, char name) : x(x), y(y), name(name) {}
};

int n, m, k, p; //board thinggy

//Moving direction
const int nX[] = { -1, 1, 0, 0 };
const int nY[] = { 0, 0, -1, 1 }; 

//Team bot
Bot myBot;
int lastMov;

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
	if (bot.x != get_border_srink() || bot.x != (n - get_border_srink() - 1) ||
		bot.y != get_border_srink() || bot.y != (m - get_border_srink() - 1))
		return true;

	return (p % k != 0);
}

bool checkRange(const Bot& bot) {
	bool check = true;
	int border = get_border_srink();
	check = check && (bot.x >= get_border_srink()) && (bot.x < (n - get_border_srink())); //x edges
	check = check && (bot.y >= get_border_srink()) && (bot.y < (m - get_border_srink())); //y edges
	check = check && (board[bot.x][bot.y] != '#'); //ban tiles
	for (char i = 'a'; i <= 'd'; i++) {
		check = check && (board[bot.x][bot.y] == i); //tiles that have been captured, got shrinked
	}
	check = check && checkShrink(bot); //die because of shrinking 
	return check;
}

void move(ofstream& fo, ofstream& dat_out) {
	if (lastMov >= 0 && checkRange(Bot(myBot.x + nX[lastMov], myBot.y + nY[lastMov], myBot.name))) {
		dat_out << lastMov;
		fo << myBot.x + nX[lastMov] << ' ' << myBot.y + nY[lastMov];
	}
	for (int i = 0; i < 4; i++) {
		if (checkRange(Bot(myBot.x + nX[i], myBot.y + nY[i], myBot.name))) {
			dat_out << i;
			fo << myBot.x + nX[i] << ' ' << myBot.y + nY[i];
			return;
		}
	}
}


int main() {
	ifstream fi;
	fi.open("MAP.INP");

	ofstream fo;
	fo.open("MAP.OUT");
	fi >> n >> m >> k >> p;

	ifstream dat_input;
	dat_input.open("STATE.DAT");
	dat_input >> lastMov;
	dat_input.close();

	ofstream dat_output;
	dat_output.open("STATE.DAT");

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
		fo << (rand() % n) << ' ' << (rand() % m) << '\n';
		dat_output << -1 << '\n';
	}
	else {
		move(fo, dat_output);
	}

	fi.close();
	fo.close();
	dat_output.close();
	return 0;
}