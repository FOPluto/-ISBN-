#include "testSolution.h"

int main(){
    string test_path = "/home/fo_pluto/opencv_home/for_test";
    
    string sample_path = "/home/fo_pluto/opencv_home/datasets";

    testSolution * test = new testSolution(sample_path);

    auto res = test->test(test_path);

    cout << res.first << " " << res.second << endl;
    return 0;
}