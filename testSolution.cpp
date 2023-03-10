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
        string testItem = testImgFN[i]; // 取出来每个路径字符串
        int idx = testItem.find("ISBN", 0);
        int num_space = 0;
        while(testItem[idx] < '0' || testItem[idx] > '9') {
            idx ++; num_space ++;
        }
        string path = testItem;
        testItem = testItem.substr(idx, testItem.length() - idx - 4);
        
        // 实际的答案
        string res = "";
        // 只取数字
        for(int j = 0;j < testItem.size();j++){
            if(testItem[j] <= '9' && testItem[j] >= '0' || testItem[j] == 'X'){
                res += testItem[j];
            }
        }

        string res_ans = "";
        int ans_num = -1;
        detectSolution* detect_item = new detectSolution(this->sample_path);
        
        for(int k = 0;k < 2;k++) {
            detect_item->fit(path, k);

            string ans = detect_item->get_res(), ans_temp = "";
            // 我们得到的结果
            // 只取数字
            for(int j = 0;j < ans.length();j++){
                if((ans[j] <= '9' && ans[j] >= '0') || ans[j] == 'X'){
                    ans_temp += ans[j];
                }
            }
            // 计算正确数量
            int num_temp = 0;
            for(int oi = 0;oi < res.length() && oi < ans_temp.length();oi++){
                if(res[oi] == ans_temp[oi]) num_temp ++;
            }
            if(ans_num < num_temp){
                res_ans = ans_temp;
                ans_num = num_temp; // 更新正确数量
            }
            
        }
        if (res == res_ans){   
            s_p_num++;
        }
        s_num++;

        for (int j = 0; j < res.length() && j < res_ans.length(); j++) {
            if (res_ans[j] == res[j]) c_p_num++;
        }
        cout << res << "  *  ";
        c_num += res.length();
        cout << res_ans << setw(10);
        
        if (res_ans == res)
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