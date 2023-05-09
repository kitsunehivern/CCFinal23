#include <bits/stdc++.h>

using namespace std;

std::mt19937 mt_rng(std::chrono::steady_clock::now().time_since_epoch().count());

template <typename _Tp>
_Tp rand(_Tp l, _Tp r) {
	assert(l <= r);
	return std::uniform_int_distribution <_Tp> (l, r) (mt_rng);
}

int main() {
    ofstream fout;
    fout.open("initial_map.inp");

    int n = 2 * rand <int> (5, 10) + 1;
    int m = 2 * rand <int> (5, 10) + 1;
    int k = rand <int> (5, 20);
    int p = 4;

    fout << n << ' ' << m << ' ' << k << " " << p << "\n";

    char s[21][21];

    int tmp = rand <int> (n * m / 10, n * m / 5);

    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            s[i][j] = '.';


    while (tmp) {
        int u = rand <int> (0, n - 1);
        int v = rand <int> (0, m - 1);

        if (s[u][v] != '#') {
            s[u][v] = '#';
            tmp--;
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++)
            fout << s[i][j];
        fout << "\n";
    }

    fout.close();
}
