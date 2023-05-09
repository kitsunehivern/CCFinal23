#include <iostream>
#include <fstream>
#include <math.h>
#include <algorithm>
#include <tuple>
#include <vector>
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
	int n, m, k, p;
	fin >> n >> m >> k >> p;

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

	sf::RenderWindow window(sf::VideoMode((m + 1) * 50,  (n + 1) * 50), "CCFinal23");
	window.setFramerateLimit(60);

	sf::Font font;
	assert(font.loadFromFile("Font/consolas.ttf"));

	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(30);
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
	const int time_stop = 1 * 60;
	
	int timer = 0, t = -1;
	while (window.isOpen()) {
		if (timer % time_stop == 0) {
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

				command = "move ";
				command.push_back(i + 'A');
				command += "\\bot.exe";
				system(command.c_str());

				command = "move ";
				command.push_back(i + 'A');
				command += "\\state.dat";
				system(command.c_str());

				command = "bot.exe";
				system(command.c_str());

				command = "move bot.exe ";
				command.push_back(i + 'A');
				system(command.c_str());

				command = "move state.dat ";
				command.push_back(i + 'A');
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
						alive[i] = false;
						continue;
					}
				}

				if (!inside(x, y) || map[x][y] == '#' || ('a' <= map[x][y] && map[x][y] <= 'd')) {
					if (t == 0) {
						do {
							x = rand <int>(0, n - 1);
							y = rand <int>(0, m - 1);
						} while (map[x][y] == '#');

						new_move[i] = std::make_pair(x, y);
					} else {
						alive[i] = false;
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
		}

		window.clear();

		sf::RectangleShape rect(sf::Vector2f(50, 50));
		rect.setFillColor(sf::Color::White);
		rect.setOutlineThickness(2);
		rect.setOutlineColor(sf::Color::Black);
		rect.setPosition(0, 0);
		window.draw(rect);

		text.setString(std::to_string(t));
		text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2, text.getLocalBounds().top + text.getLocalBounds().height / 2);
		text.setPosition(25, 25);
		window.draw(text);

		for (int i = 0; i < n; i++) {
			rect.setFillColor(sf::Color::White);
			rect.setOutlineThickness(2);
			rect.setOutlineColor(sf::Color::Black);
			rect.setPosition(0, (i + 1) * 50);
			window.draw(rect);

			text.setString(std::to_string(i));
			text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2, text.getLocalBounds().top + text.getLocalBounds().height / 2);
			text.setPosition(25, (i + 1) * 50 + 25);
			window.draw(text);
		}

		for (int j = 0; j < m; j++) {
			rect.setFillColor(sf::Color::White);
			rect.setOutlineThickness(2);
			rect.setOutlineColor(sf::Color::Black);
			rect.setPosition((j + 1) * 50, 0);
			window.draw(rect);

			text.setString(std::to_string(j));
			text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2, text.getLocalBounds().top + text.getLocalBounds().height / 2);
			text.setPosition((j + 1) * 50 + 25, 25);
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
				rect.setPosition((j + 1) * 50, (i + 1) * 50);
				window.draw(rect);
			}
		}

		for (int i = 0; i < p; i++) {
			text.setString(std::string(1, i + (alive[i] ? 'A' : 'a')));
			text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2, text.getLocalBounds().top + text.getLocalBounds().height / 2);
			text.setPosition((last_move[i].second + 1) * 50 + 25, (last_move[i].first + 1) * 50 + 25);
			window.draw(text);
		}

		window.display();

		timer++;

		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
				
				for (int i = 0; i < p; i++) {
					command = "del ";
					command.push_back(i + 'A');
					command += "\\state.dat";
					system(command.c_str());
				}

				command = "del map.inp";
				system(command.c_str());

				command = "del move.out";
				system(command.c_str());

				return 0;
			}
		}
	}

	return 0;
}