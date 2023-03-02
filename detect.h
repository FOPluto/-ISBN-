/**
 * @author FoPluto
*/
#ifndef DETECT
#define DETECT

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

using namespace std;
using namespace cv;

#define ERROR -1
#define DONE 0

// 宏定义：如果不注释就是打开对应DEBUG模式
#define DEBUG

#ifdef DEBUG

// 设置模式




#endif

typedef pair<int, pair<int, int>> PIII;

typedef pair<int, int> PII;


// pair比较函数
bool Comp(pair<int, double>a, pair<int, double>b) {return a.second < b.second;}

class detectSolution{
    private:
    // 样本图片的路径
    string sampleImgPath;

    Mat src_image;     // 输入图片
    Mat src_copy_image;    // 拷贝输入图片
    Mat threshold_image;   //处理之后的二值化图片
    Mat rotated_image;   // 旋转之后的图像
    Mat res_image;    // 处理完成的图像，提取兴趣框时用
    Mat ROI_image;   // 提取兴趣框

    vector<int> rows_element;   // 行ROI感兴趣区域
    vector<Point> points;  // 用于表示：每一行的像素值大于100像素点的数量
    vector<PIII> ans;   // 存储答案的容器
    vector<int> num_area;   // 数字的ROI感兴趣区域，即字符分割
    vector<PII> num_position;   // 数字的位置信息，用于字符分割
    vector<Mat> num_ROI_rect;   // 数字的Mat，截取数字的结果

    std::string res_str;  // 最终的答案

    double ChNum;   // 字符准确度

    double StrNum;   // 字符串准确度

    private:


    // 计算图像像素点值的平均值，用于分类
    double CalcImg(Mat inputImg);

    // 模板匹配的主要函数
    char CheckImg(Mat inputImg);

    // 水浸操作
    void FloodFill(Mat& pic);

    // 预处理函数, 返回值赋值给res_image
    Mat get_res_image(Mat& src_image, int type);

    // resize_stand函数，将图片resize成为较小尺寸，减少计算量
    void resize_stand();

    // find_ROI函数，用于寻找感兴趣区域
    void find_ROI();

    public:

    // 获取结果
    string get_res();

    // 获取字符准确度
    double getChNum();

    // 获取字符串准确度
    double getStrNum();

    // fit函数，用于整体识别的接口
    int fit(string src_path);

};

#endif