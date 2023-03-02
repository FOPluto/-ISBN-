#include "testSolution.h"

/// @brief 测试，得到模型的准确度和准确率
pair<double, double> testSolution::test(string test_path){
    vector<String> testImgFN;
    glob(test_path, testImgFN, false);
    int c_num = 0, c_p_num = 0;  //字符
    int s_num = 0, s_p_num = 0;  //字符串
    for(int i = 0;i < testImgFN.size();i++){
        detectSolution* detect_item = new detectSolution(this->sample_path);
        string testItem = testImgFN[i];
        detect_item->fit(testItem);
        string ac = "";
        int idx = testItem.find("ISBN ", 0);
        while(testItem[idx] != '.'){
            ac += testItem[idx ++];
        }
        string ans = detect_item->get_res();

        if(ans == ac) s_p_num ++;
        s_num ++;
        for(int j = 0;j < testItem.length() && j < ans.length();j++){
            if(ans[j] == testItem[j]) c_p_num ++;
        }
        c_num += testItem.length();
        cout << detect_item->get_res() << endl;
        delete detect_item;
    }
    return {(double)c_p_num / c_num, (double)s_p_num / s_num};
}