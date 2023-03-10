#include "testSolution.h"

int main() {
    string test_path = "/home/fo_pluto/opencv_home/2023_test/*";

    string sample_path = "/home/fo_pluto/opencv_home/datasets/*";

    testSolution* test = new testSolution(sample_path);

    auto res = test->test(test_path);

    //cout << endl;
    //cout << res.first << " " << res.second << endl;
    cout << "string P:  " << res.first * 100 << "%" << endl;
    cout << "words P:  " << res.second * 100 << "%" << endl;

    return 0;
}