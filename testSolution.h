#ifndef TEST_SOLUTION
#define TEST_SOLUTION
#include "detect.h"

class testSolution {
private:
    string sample_path;

    vector<pair<string, string>> ans;

public:
    testSolution(string sample_path) { this->sample_path = sample_path; }
    pair<double, double> test(string path);
};


#endif
