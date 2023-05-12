#include <iostream>
#include <fstream>
#include <math.h>
#include <algorithm>
#include <tuple>
#include <vector>
#include <queue>
#include <string>
#include <assert.h>
#include <random>
#include <chrono>

#define NOMINMAX
#include <windows.h>

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

std::mt19937 mt_rng(std::chrono::steady_clock::now().time_since_epoch().count());

template <typename _Tp>
_Tp rand(_Tp l, _Tp r) {
	assert(l <= r);
	return std::uniform_int_distribution <_Tp> (l, r) (mt_rng);
}

int main() {
	std::ifstream fin;
	std::ofstream fout;
	std::string command;

	// read inital map
	fin.open("initial_map.inp");
	assert(fin.good());
	int n, m, k;
	fin >> n >> m >> k;

	std::vector <std::vector <int> > map(n, std::vector <int> (m));
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			char c;
			fin >> c;

			if (c == '.') {
				map[i][j] = 5;
			} else {
				map[i][j] = 0;
			}
		}
	}
	
	fin.close();

	std::cout << "Enter number of players: ";
	int p; std::cin >> p;

	std::vector <std::string> player_name(p);
	for (int i = 0; i < p; i++) {
		std::cout << "Enter name for player " << char(i + 'A') << ": ";
		std::cin >> player_name[i];
	}

	sf::RenderWindow window(sf::VideoMode((m + 1) * 40,  (n + 1) * 40), "CCFinal23", sf::Style::Close);
	window.setFramerateLimit(60);

	sf::Font font;
	assert(font.loadFromFile("Font/consolas.ttf"));

	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(20);
	text.setFillColor(sf::Color::Black);

	auto inside = [&](int x, int y) -> bool {
		return 0 <= x && x <= n - 1 && 0 <= y && y <= m - 1;
	};

	std::vector <sf::Color> unblocked_color = {
		sf::Color(255,   0,   0),
		sf::Color(255, 255,   0),
		sf::Color(  0, 255,   0),
		sf::Color(  0,   0, 255),
		sf::Color(255, 255, 255),
	};

	std::vector <sf::Color> blocked_color = {
		sf::Color(234, 153, 153),
		sf::Color(255, 229, 153),
		sf::Color(182, 215, 168),
		sf::Color(164, 194, 244),
		sf::Color(153, 153, 153)
	};
	
	std::vector <bool> alive(p, true);
	std::vector <std::pair <int, int> > last_move(p, std::make_pair(-1, -1));
	const int time_stop = 1;
	
	int timer = 0, t = -1;
	while (window.isOpen()) {
		if (std::count(alive.begin(), alive.end(), true) && timer % time_stop == 0) {
			t++;
			std::cout << "======================\n";
			std::cout << "TURN " << t << "\n";
			std::cout << "======================\n";
			std::vector <std::pair <int, int> > new_move = last_move;
			for (int i = 0; i < p; i++) {
				std::cout << "++++++++++\n";
				std::cout << "PLAYER " << char(i + 'A') << "\n";
				std::cout << "++++++++++\n";

				if (!alive[i]) {
					continue;
				}

				fout.open("map.inp");
				assert(fout.good());

				fout << n << " " << m << " " << k << " " << t << "\n";
				fout << last_move[i].first << " " << last_move[i].second << " " << char(i + 'A') << "\n";
				fout << std::count(alive.begin(), alive.end(), true) - 1 << "\n";
				for (int j = 0; j < p; j++) {
					if (j == i || !alive[j]) {
						continue;
					}

					fout << last_move[j].first << " " << last_move[j].second << " " << char(j + 'A') << "\n";
				}

				for (int i = 0; i < n; i++) {
					for (int j = 0; j < m; j++) {
						if (map[i][j] == 5) {
							fout << ".";
						} else if (map[i][j] == 0 || map[i][j] == -5) {
							fout << "#";
						} else if (map[i][j] < 0) {
							fout << char(-map[i][j] - 1 + 'a');
						} else {
							fout << char(map[i][j] - 1 + 'A');
						}
					}
					fout << "\n";
				}

				fout.close();

				command = "move " + player_name[i];
				command += "\\bot.exe";
				system(command.c_str());

				command = "move " + player_name[i];
				command += "\\state.dat";
				system(command.c_str());

				auto start = std::chrono::high_resolution_clock::now();

				command = "bot.exe";
				system(command.c_str());

				auto finish = std::chrono::high_resolution_clock::now();
				std::cerr << "Time used: " << std::chrono::duration_cast <std::chrono::milliseconds> (finish - start).count() << " (ms)\n";

				command = "move bot.exe " + player_name[i];
				system(command.c_str());

				command = "move state.dat " + player_name[i];
				system(command.c_str());

				fin.open("move.out");
				assert(fin.good());

				int x, y;
				if (!(fin >> x >> y)) {
					alive[i] = false;
					fin.close();
					break;
				}

				std::cerr << "RUNNING " << x << " " << y << "\n";
				fin.close();

				if (last_move[i].first != -1) {
					//assert(abs(x - last_move[i].first) + abs(y - last_move[i].second) <= 1);
					if (abs(x - last_move[i].first) + abs(y - last_move[i].second) > 1) {
						continue;
					}
				}

				if (!inside(x, y) || map[x][y] == 0 || map[x][y] == -5 || (-4 <= map[x][y] && map[x][y] <= -1)) {
					if (t == 0) {
						do {
							x = rand <int>(0, n - 1);
							y = rand <int>(0, m - 1);
						} while (map[x][y] == 0);

						new_move[i] = std::make_pair(x, y);
					} else {
						continue;
					}
				} else {
					new_move[i] = std::make_pair(x, y);
				}
			}

			last_move = new_move;

			for (int i = 0; i < p; i++) {
				if (!alive[i]) {
					continue;
				}

				bool check = false;
				for (int j = 0; j < p; j++) {
					if (j == i || !alive[j]) {
						continue;
					}

					if (last_move[i] == last_move[j]) {
						check = true;
						break;
					}
				}

				if (!check) {
					map[last_move[i].first][last_move[i].second] = i + 1;
				}
			}

			auto bfs = [&](int s, int t, int c) -> bool {
				int nX[] = {-1, 1, 0, 0};
				int nY[] = {0, 0, -1, 1};

				std::queue<std::pair <int, int> > q;
				std::vector <std::vector <bool> > vis(n, std::vector <bool> (m));
				q.push({ s, t });
				vis[s][t] = false;
				while (!q.empty()) {
					int u = q.front().first;
					int v = q.front().second;

					if (u - 1 < 0 || u + 1 >= n || v - 1 < 0 || v + 1 >= m) {
						return false;
					}

					q.pop();
					for (int i = 0; i < 4; i++) {
						int x = u + nX[i];
						int y = v + nY[i];

						if (!vis[x][y] && inside(x, y) && abs(map[x][y]) != c) {
							vis[x][y] = true;
							q.push({ x, y });
						}
					}
				}

				return true;
			};

			for (int z = 0; z < p; z++) {
				std::vector <std::pair <int, int> > pending;
				for (int i = 0; i < n; i++) {
					for (int j = 0; j < m; j++) {
						if (map[i][j] > 0 && map[i][j] != z + 1 && bfs(i, j, z + 1)) {
							pending.push_back({i, j});
						}
					}

				}

				for (auto i : pending) {
					for (int zz = 0; zz < p; zz++) {
						if (zz == z) {
							continue;
						}

						if (i == last_move[zz]) {
							alive[zz] = false;
						}
					}

					map[i.first][i.second] = z + 1;
				}
			}

			if (t > 1 && (t - 1) % k == 0) {
				int l = (t - 1) / k;

				for (int i = 0; i < n; i++) {
					for (int j = 0; j < m; j++) {
						if (i == l - 1 || j == l - 1 || i == n - l || j == m - l) {
							map[i][j] = -1 * std::abs(map[i][j]);
						}
					}
				}

				for (int i = 0; i < p; i++) {
					if (alive[i]) {
						if (map[last_move[i].first][last_move[i].second] <= 0) {
							alive[i] = false;
						}
					}
				}
			}

			for (int z = 0; z < p; z++) {
				int cnt = 0;
				for (int i = 0; i < n; i++) {
					for (int j = 0; j < m; j++) {
						cnt += (abs(map[i][j]) == z + 1);
					}
				}

				std::cout << "Player " << char(z + 'A') << ": " << cnt << "\n";
			}
		}

		window.clear();

		sf::RectangleShape rect(sf::Vector2f(40, 40));
		rect.setFillColor(sf::Color::White);
		rect.setOutlineThickness(2);
		rect.setOutlineColor(sf::Color::Black);
		rect.setPosition(0, 0);
		window.draw(rect);

		text.setString(std::to_string(t));
		text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2, text.getLocalBounds().top + text.getLocalBounds().height / 2);
		text.setPosition(20, 20);
		window.draw(text);

		for (int i = 0; i < n; i++) {
			rect.setFillColor(sf::Color::White);
			rect.setOutlineThickness(2);
			rect.setOutlineColor(sf::Color::Black);
			rect.setPosition(0, (i + 1) * 40);
			window.draw(rect);

			text.setString(std::to_string(i));
			text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2, text.getLocalBounds().top + text.getLocalBounds().height / 2);
			text.setPosition(20, (i + 1) * 40 + 20);
			window.draw(text);
		}

		for (int j = 0; j < m; j++) {
			rect.setFillColor(sf::Color::White);
			rect.setOutlineThickness(2);
			rect.setOutlineColor(sf::Color::Black);
			rect.setPosition((j + 1) * 40, 0);
			window.draw(rect);

			text.setString(std::to_string(j));
			text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2, text.getLocalBounds().top + text.getLocalBounds().height / 2);
			text.setPosition((j + 1) * 40 + 20, 20);
			window.draw(text);
		}

		for (int i = 0; i < n; i++) {
			for (int j = 0; j < m; j++) {
				if (map[i][j] == 0) {
					rect.setFillColor(sf::Color::Black);
				} else if (map[i][j] < 0) {
					rect.setFillColor(blocked_color[-map[i][j] - 1]);
				} else {
					rect.setFillColor(unblocked_color[map[i][j] - 1]);
				}

				rect.setOutlineThickness(2);
				rect.setOutlineColor(sf::Color::Black);
				rect.setPosition((j + 1) * 40, (i + 1) * 40);
				window.draw(rect);
			}
		}

		for (int i = 0; i < p; i++) {
			text.setString(std::string(1, i + (alive[i] ? 'A' : 'a')));
			text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2, text.getLocalBounds().top + text.getLocalBounds().height / 2);
			text.setPosition((last_move[i].second + 1) * 40 + 20, (last_move[i].first + 1) * 40 + 20);
			window.draw(text);
		}

		window.display();

		if (std::count(alive.begin(), alive.end(), true)) {
			timer++;
		}

		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}
	}

	for (int i = 0; i < p; i++) {
		command = "del " + player_name[i];
		command += "\\state.dat";
		system(command.c_str());
	}

	command = "del map.inp";
	system(command.c_str());

	command = "del move.out";
	system(command.c_str());


	return 0;
}