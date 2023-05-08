#include<bits/stdc++.h>
using namespace std;

string to_string(bool __b);
string to_string(string __s);
string to_string(const char* __s);
string to_string(char __c);
template <typename _Tp, typename _Up>
string to_string(pair <_Tp, _Up> __p);
template <size_t _Ts>
string to_string(bitset <_Ts> __b);
template <typename _Tp>
string to_string(_Tp __c);

string to_string(bool __b) {
    return (__b) ? "true" : "false";
}

string to_string(string __s) {
    return "\"" + __s + "\"";
}

string to_string(const char* __s) {
    return "\"" + string(__s) + "\"";
}

string to_string(char __c) {
    return "'" + string(1, __c) + "'";
}

template <typename _Tp, typename _Up>
string to_string(pair <_Tp, _Up> __p) {
    return "(" + to_string(__p.first) + ", " + to_string(__p.second) + ")";
}

template <size_t _Ts>
string to_string(bitset <_Ts> __b) {
    return __b.to_string();
}

template <typename _Tp>
string to_string(_Tp __c) {
    string res = "{";
    for (auto it = __c.begin(); it != __c.end(); it++) {
        res += ((it == __c.begin()) ? "" : ", ") + to_string(*it);
    }
    res += "}";
    return res;
}

void error(vector <string> &__v) {
    cerr << "\n";
}

template <typename _Tp, typename... Args>
void error(vector <string> &__v, _Tp __x, Args... args) {
    cerr << "[" << __v.back() << " = " << to_string(__x) << "] ";
    __v.pop_back();
    error(__v, args...);
}

#ifndef ONLINE_JUDGE
#define debug(args...) {                         \
    string __t = #args, __s; __t.push_back(','); \
    int __a = 0; bool __b = true;                \
    vector <string> __v;                         \
    for (auto __c : __t) {                       \
        if (__c == ',' && !__a && __b) {         \
            for (int __i = 2; __i--; ) {         \
                while (__s.back() == ' ') {      \
                    __s.pop_back();              \
                }                                \
                reverse(__s.begin(), __s.end()); \
            }                                    \
            __v.push_back(__s);                  \
            __s.clear();                         \
        } else {                                 \
            if (__c == '"') {                    \
                __b ^= true;                     \
            } else if (__c == '(') {             \
                __a += __b;                      \
            } else if (__c == ')') {             \
                __a -= __b;                      \
            }                                    \
            __s.push_back(__c);                  \
        }                                        \
    }                                            \
    reverse(__v.begin(), __v.end());             \
    error(__v, args);                            \
}
#else
#define debug(args...) 42
#endif

std::mt19937 mt_rng(std::chrono::steady_clock::now().time_since_epoch().count());

template <typename _Tp>
_Tp rand(_Tp l, _Tp r) {
	assert(l <= r);
	return std::uniform_int_distribution <_Tp> (l, r) (mt_rng);
}


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
		check = check && (board[bot.x][bot.y] != i); //tiles that have been captured, got shrinked
	}
	check = check && checkShrink(bot); //die because of shrinking
	return check;
}

void move(ofstream& fo, ofstream& dat_out) {
	if (lastMov >= 0 && checkRange(Bot(myBot.x + nX[lastMov], myBot.y + nY[lastMov], myBot.name))) {
		dat_out << lastMov;
		fo << myBot.x + nX[lastMov] << ' ' << myBot.y + nY[lastMov];
		return;
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
	fi.open("map.inp");

	ofstream fo;
	fo.open("move.out");
	fi >> n >> m >> k >> p;

	ifstream dat_input;
	dat_input.open("state.dat");
	if (!dat_input >> lastMov) {
        lastMov = -1;
	}

	dat_input.close();

	ofstream dat_output;
	dat_output.open("state.dat");

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
		fo << rand <int> (0, n) << ' ' << rand <int> (1, m) << '\n';
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
