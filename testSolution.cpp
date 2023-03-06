#include "testSolution.h"

/// @brief 测试，得到模型的准确度和准确率
pair<double, double> testSolution::test(string test_path) 
{
    vector<String> testImgFN;
    glob(test_path, testImgFN, false);
    int c_num = 0, c_p_num = 0;  //字符
    int s_num = 0, s_p_num = 0;  //字符串
    for (int i = 0; i < testImgFN.size(); i++) 
    {
        detectSolution* detect_item = new detectSolution(this->sample_path);
        string testItem = testImgFN[i];
        detect_item->fit(testItem);

        int idx = testItem.find("ISBN ", 0);
        testItem = testItem.substr(idx + 5, testItem.length() - idx - 9);
        int idx_copy = idx;

        string ans = detect_item->get_res();

        if (ans == testItem)
        {   
            s_p_num++;
        }
        s_num++;

        for (int j = 0; j < testItem.length() && j < ans.length(); j++) 
        {
            if (ans[j] == testItem[j]) c_p_num++;
        }
        c_num += testItem.length();
        cout << detect_item->get_res() << setw(10);
        
        if (ans == testItem)
        {
            cout << "right, ";
            cout << "right_num: " << s_p_num  <<"   ";
            cout << "right_rate: " << s_p_num << "/" << s_num << "=" << (double)s_p_num / s_num * 100<<setprecision(4) << "%"  << "   ";
            cout << "accuracy: " << c_p_num<<"/"<< c_num<<"="<<(double)c_p_num / c_num * 100 << "%"  << endl;
            cout << endl;
        }
        else {
            cout << "fail, ";
            cout << "fail_num: " << s_p_num << "   ";
            cout << "right_rate: " << s_p_num << "/" << s_num << "=" << (double)s_p_num / s_num * 100 << setprecision(4) << "%" << "   ";
            cout << "accuracy: " << c_p_num << "/" << c_num << "=" << (double)c_p_num / c_num * 100 << "%" << endl;
            cout << endl;
        }




        
        delete detect_item;
    }
    return { (double)s_p_num / s_num, (double)c_p_num / c_num };
    //return { c_p_num,c_num };
}