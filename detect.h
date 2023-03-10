/**
 * @author FoPluto
*/
#ifndef DETECT
#define DETECT

#include <opencv2/opencv.hpp>
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

#define DEBUG_ALL

#ifdef DEBUG_ALL

//#define DEBUG

#define DEBUG_THRESHOLD          // 二值化调试
#define DEBUG_RES                // 预处理调试
// #define DEBUG_BLUR               // 中值滤波
// #define DEBUG_ER_DE              // 膨胀腐蚀调试
// #define DEBUG_FOOLD              // 漫水调试
// #define DEBUG_ROI                // ROI框调试
// #define DEBUG_ITEM               // 字符框调试
// #define DEBUG_LINES              // 线调试
#endif

typedef pair<int, pair<int, int>> PIII;

typedef pair<int, int> PII;



class detectSolution {
private:
    // 样本图片的路径
    string sampleImgPath;

    Mat src_image;               // 输入图片
    Mat src_copy_image;          // 拷贝输入图片
    Mat gray_image;
    Mat threshold_image;         // 处理之后的二值化图片
    Mat rotated_image;           // 旋转之后的图像
    Mat res_image;               // 处理完成的图像，提取兴趣框时用
    Mat ROI_image;               // 提取兴趣框

    vector<int> rows_element;    // 行ROI感兴趣区域
    vector<Point> points;        // 用于表示：每一行的像素值大于100像素点的数量
    vector<PIII> ans;            // 存储答案的容器
    vector<int> num_area;        // 数字的ROI感兴趣区域，即字符分割
    vector<PII> num_position;    // 数字的位置信息，用于字符分割
    vector<Mat> num_ROI_rect;    // 数字的Mat，截取数字的结果

    std::string res_str;         // 最终的答案

    double ChNum;                // 字符准确度

    double StrNum;               // 字符串准确度

    double average;              // 保存平均亮度

    Range ROI_range;             // y字符兴趣框位置

    Range ROI_range_x;           // x字符兴趣框位置
private:

    // 旋转操作
    void ImgRectify(Mat& pic, Mat& BinaryFlat);

    // 获取平均亮度
    void get_average_light(Mat _src);

    // 计算图像像素点值的平均值，用于分类
    double CalcImg(Mat inputImg);

    // 模板匹配的主要函数
    char CheckImg(Mat inputImg, int idx);

    // 水浸操作
    void FloodFill(Mat& pic);

    // 预处理函数, 返回值赋值给res_image，使用自适应阈值
    Mat get_res_image(Mat& src_image, int type);

    // 预处理函数, 返回值赋值给res_image，使用大津法
    Mat get_res_image2(Mat& src_image, int type);

    // resize_stand函数，将图片resize成为较小尺寸，减少计算量
    void resize_stand();

    // find_ROI函数，用于寻找感兴趣区域
    void find_ROI();

public:

    // 构造函数,初始化两个参数
    detectSolution(string sample_path) { this->sampleImgPath = sample_path; this->ROI_range.end = 0, this->ROI_range.start = 0;}

    // 获取结果
    string get_res();

    // 获取字符准确度
    double getChNum();

    // 获取字符串准确度
    double getStrNum();

    // fit函数，用于整体识别的接口
    int fit(string src_path, int model);

};

#endif