#include <iostream>
#include <random>
using namespace std;
mt19937 gen(random_device{}());
double exponentialRandom(double mean) {
    // Return random number from exponential distribution
    exponential_distribution<> dist(1.0 / mean);
    return dist(gen);
}
int main() {
    for (int i = 0; i < 20; i++) {
        cout << exponentialRandom(100.0 / (0.6)) << " " << exponentialRandom(100.0 / (1.0/30)) << endl;
    }
}