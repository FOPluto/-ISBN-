/**
 * @authors 可莉不知道哦
 * @brief 本文件写的是detectSolution对外部的接口
 * @details fit test
*/

#include "detect.h"

/// 外部接口
int detectSolution::fit(string src_path){
    // 输入图像
    this->src_image = imread(src_path);
    if(!this->src_image.data) {
        cout << "src_image_empty!" << endl;
        return ERROR;
    }
    
    // 图像标准化，并进行预处理
    this->resize_stand();
    this->res_image = this->get_res_image(this->src_image, THRESH_BINARY_INV);

    // 计算图像的平均像素值
    double _sum = CalcImg(this->res_image);
    cout << "average_: " << _sum << endl;

    // 如果平均像素值过小，那么换一个参数重新进行预处理
    if(_sum <= 40) this->res_image = get_res_image(this->src_image, THRESH_BINARY);

    // 浸水操作
    this->FloodFill(this->src_image);

    // 寻找ROI区域
    this->find_ROI();

    // 准备获取ans
    vector<char> ans_;
    this->res_str = "";
    int idx_image = 0;

    // 开始模板匹配
    for(int i = 0;i < num_position.size();i++){
        Mat item_image = ROI_image(Range::all(), Range(num_position[i].first, num_position[i].second));
        num_ROI_rect.push_back(item_image);
        char ch = CheckImg(item_image);
        ans_.push_back(ch);
        //imwrite("demo/" + to_string(idx_image ++) + ".jpg", item_image); // 存储自制模板
    }
    bool flag = false;

    // 整理结果
    for(int i = 0;i < ans_.size();i++){
        if(flag){
            res_str += ans_[i];
        }else if(ans_[i] == 'N'){
            flag = true;
        }
    }
    return DONE;
}


/// @brief 获取结果
/// @return string
string detectSolution::get_res(){
    return this->res_str;
}

double detectSolution::getChNum(){
    return this->ChNum;
}

double detectSolution::getStrNum(){
    return this->StrNum;
}