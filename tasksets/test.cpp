#include <iostream>
#include <vector>
#include <string>
#include <queue>
using namespace std;

class s {
public:
    int a;
    int b;
    string c;
    double d;

    s() {
        this->a = 1;
    }
    s(int a, int b, string c, double d) : a(a), b(b), c(c), d(d) {}
};

int main() {
    s ss;
    cout << ss.a << " " << ss.b << " " << ss.c << " " << ss.d << endl;
    vector<s> vec;
    queue<s> que;
    auto& temp = ss;
    vec.push_back(ss);
    que.push(temp);
    auto& temp2 = que.front();
    temp2.a = 3;
    cout << ss.a << " " << temp.a << " " << temp2.a << endl;
    return 0;
}