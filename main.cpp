#include <iostream>
#include <algorithm>
#include "cilk/cilk.h"
#include <ctime>
#include <chrono>

//#define int long long

using namespace std;

//const int BLOCK = 32;

//void p_for(int times, int counter, void (*f)()) {
void p_for(int times, int counter, function<void(void)> f) {
    if (times < 2) {
        f();
        return;
    }
    int times1 = times / 2;
    cilk_scope {
            cilk_spawn p_for(times1, counter, f);
            p_for(times - times1, counter + times1, f);
    };
}

//void p_for(int times, int counter, void (*f)(int i)) {
void p_for(int times, int counter, function<void(int)> f) {
    if (times < 2) {
        f(counter);
        return;
    }
    int times1 = times / 2;
    cilk_scope {
            cilk_spawn p_for(times1, counter, f);
            p_for(times - times1, counter + times1, f);
    };
}


//vector<int> scan(int* l, int* r) {
//    int p = 1;
//    int deg = 0;
//    while (p < r - l) {
//        deg++;
//        p *= 2;
//    }
////    int* arr = new int[p];
//    vector<int> arr = vector<int>(p, 0);
//    memcpy(arr.data(), l, (r - l) * sizeof(int));
//    for (int i = 2; i <= p; i *= 2) {
//        p_for(p / i, 0, [&](int j) {
//            arr[j*i+i-1] = arr[j*i+i/2-1] + arr[j*i+i-1];
//        });
//    }
//    arr[p-1] = 0;
//    for (int i = p; i > 1; i /= 2) {
//        p_for(p / i, 0, [&](int j) {
//            int sum = arr[j*i+i-1] + arr[j*i+i/2-1];
//            arr[j*i+i/2-1] = arr[j*i+i-1];
//            arr[j*i+i-1] = sum;
//        });
//    }
//    return arr;
//}

void in_place_scan(int* l, int* r) {
    int p = 1;
    int deg = 0;
    while (p < r - l) {
        deg++;
        p *= 2;
    }
//    int* arr = new int[p];
    vector<int> arr = vector<int>(p, 0);
    memcpy(arr.data(), l, (r - l) * sizeof(int));
    for (int i = 2; i <= p; i *= 2) {
        p_for(p / i, 0, [&](int j) {
            arr[j*i+i-1] = arr[j*i+i/2-1] + arr[j*i+i-1];
        });
    }
    arr[p-1] = 0;
    for (int i = p; i > 1; i /= 2) {
        p_for(p / i, 0, [&](int j) {
            int sum = arr[j*i+i-1] + arr[j*i+i/2-1];
            arr[j*i+i/2-1] = arr[j*i+i-1];
            arr[j*i+i-1] = sum;
        });
    }
    memcpy(l, arr.data(), (r - l) * sizeof(int));
}

vector<int> filter(vector<int> src, function<bool(int)> f) {
    vector<int> result = vector<int>();
    for (int i : src) {
        if (f(i))
            result.push_back(i);
    }
    return result;
}

vector<int> a;

void bfs(vector<vector<int>> gr) {
    a = vector<int>(gr.size(), -1);
    vector<vector<int>> f = vector<vector<int>>(gr.size(), vector<int>());
    f[0] = vector<int>(1, 0);
    a[0] = 0;
    for (int i = 0; i < gr.size() && !f[i].empty(); i++) {
        vector<int> deg = vector<int>(f[i].size(), 0);
//        p_for(f[i]->size(), 0, [&](int j) {
//            deg[j] = gr[(*f[i])[j]].size();
//        });
        for(int j = 0; j < f[i].size(); j++) {
            deg[j] = gr[f[i][j]].size();
        }
        in_place_scan(deg.data(), deg.data() + deg.size());
        f[i+1] = vector<int>(deg[f[i].size()-1] + gr[f[i][f[i].size()-1]].size(), -1);
        p_for(f[i].size(), 0, [&](int j) {
            for (int vi = 0; vi < gr[f[i][j]].size(); vi++) {
                int v = gr[f[i][j]][vi];
//                if (i == 0) {
//                    cout << v << "\n";
//                }
                if (a[v] == -1) {
                    a[v] = i+1;
                    f[i+1][deg[j]+vi] = v;
                }
            }
        });
        f[i+1] = filter(f[i+1], [](int x) { return x >= 0; });
//        if (i == 8) {
//            for (int j = 0; j < f[i+1].size(); j++)
//                cout << f[i+1][j] << " ";
//            cout << "\n";
//        }
    }
//    return a;
}

signed main() {
//    p_for(4, 0, []() { cout << "10 "; });
    p_for(4, 0, [](int a) { cout << a << " "; });
    cout << "\n";

    vector<int> arr = {1, 2, 3, 4, 5, 6, 7, 8};
    in_place_scan(arr.data(), arr.data() + arr.size());
    for (int i = 0; i < arr.size(); i++)
        cout << arr[i] << " ";
    cout << "\n\n";

    int n = 300;
    int n2 = n * n;
    int n3 = n * n * n;
    vector<vector<int>> gr = vector<vector<int>>(n3, vector<int>());
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                if (i > 0) gr[i*n2 + j*n + k].push_back((i-1)*n2 + j*n + k);
                if (i < n-1) gr[i*n2 + j*n + k].push_back((i+1)*n2 + j*n + k);
                if (j > 0) gr[i*n2 + j*n + k].push_back(i*n2 + (j-1)*n + k);
                if (j < n-1) gr[i*n2 + j*n + k].push_back(i*n2 + (j+1)*n + k);
                if (k > 0) gr[i*n2 + j*n + k].push_back(i*n2 + j*n + k-1);
                if (k < n-1) gr[i*n2 + j*n + k].push_back(i*n2 + j*n + k+1);
            }
        }
    }

//    for (int i = 0; i < gr[0].size(); i++)
//        cout << gr[0][i] << " ";
//    cout << "\n";

    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
    bfs(gr);
    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

    cout << "Done in " << (end - start).count() / 1000000000.0 << " seconds\n";

//    cout << a[0] << " " << a[100] << " " << a[200] << " " << a[300] << " " << a[500] << " " << a[900] << "\n";
//    cout << a[0] << " " << a[9] << " " << a[10] << " " << a[19] << " " << a[29] << " " << a[39] << " " << a[49]
//    << " " << a[99]
//    << " " << a[998] << " " << a[999] << "\n";

    return 0;
}
