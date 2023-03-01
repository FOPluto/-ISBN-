/**
 * @brief test
 * @author 可莉不知道哦
*/

// debug模式
#define DEBUG
#define DEBUG_LINE
#define DEBUG_GRAY_GRAGH

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


string test_image_path = "/home/fo_pluto/opencv_home/demo.jpg";

typedef pair<int, pair<int, int>> PIII;

typedef pair<int, int> PII;

void FloodFill(Mat& pic)//水漫操作
{
	int dx[] = { -1,0,1,-1,1,-1,0,1 };
	int dy[] = { 1,1,1,0,0,-1,-1,-1 };
	queue<PII> q;
	for (int i = 0; i < pic.cols; i++)//上
		for (int j = 0; j < 1; j++)
			if (pic.at<uchar>(j, i) != 0) q.push({ j,i });
	for (int i = 0; i < pic.cols; i++)//下
		for (int j = pic.rows - 1; j >= pic.rows - 1; j--)
			if (pic.at<uchar>(j, i) != 0) q.push({ j,i });
	for (int i = 0; i < pic.rows; i++)//左
		for (int j = 0; j < 1; j++)
			if (pic.at<uchar>(i, j) != 0) q.push({ i,j });
	for (int i = 0; i < pic.rows; i++)//右
			for (int j = pic.cols - 1; j >= pic.cols - 1; j--)
				if (pic.at<uchar>(i, j) != 0) q.push({ i,j });
	while (!q.empty())
	{
		PII t = q.front(); q.pop();
		int x = t.first, y = t.second;
		for (int i = 0; i < 8; i++)
		{
			int nx = x + dx[i];
			int ny = y + dy[i];
			if (nx < 0 || ny < 0 || nx >= pic.rows || ny >= pic.cols) continue;
			if (pic.at<uchar>(nx, ny) != 0)
			{
				pic.at<uchar>(nx, ny) = 0;
				q.push({ nx,ny });
			}
		}
	}
}

void ImgRectify(Mat& pic, Mat& BinaryFlat, Mat& Img, Mat& ImgFlat)//图像矫正, 明天再看看
{
	Mat pic_edge;
	Sobel(pic, pic_edge, -1, 0, 1, 5);
	//霍夫直线检测（第5个参数是阈值，阈值越大，检测精度越高）
	vector<Vec2f> Line;
	HoughLines(pic_edge, Line, 1, CV_PI / 180, 180, 0, 0);
	//计算偏转角度
	double Angle = 0;
	int LineCnt = 0;
	for (int i = 0; i < Line.size(); i++)
	{
		if (Line[i][1] < 1.2 || Line[i][1]>1.8) continue;
		Angle += Line[i][1];
		LineCnt++;
	}
	if (LineCnt == 0) Angle = CV_PI / 2;
	else Angle /= LineCnt;
	Angle = 180 * Angle / CV_PI - 90;
	Mat pic_tmp = getRotationMatrix2D(Point(pic.cols / 2, pic.rows / 2), Angle, 1);
	warpAffine(pic, BinaryFlat, pic_tmp, pic.size());
	warpAffine(Img, ImgFlat, pic_tmp, Img.size());
	FloodFill(BinaryFlat);
}



int main(){
    Mat src_image = imread(test_image_path);

    double width = 700;
    double height = width * (double)src_image.rows / src_image.cols;
    resize(src_image, src_image, cv::Size(width, height));

    Mat src_copy_image;
    src_image.copyTo(src_copy_image);

    // 灰度化处理
    Mat gray_image;
    cvtColor(src_image, gray_image, COLOR_RGB2GRAY);

    Mat erode_dilate_image;
    Mat element = getStructuringElement(MORPH_RECT, Size(1, 1));
    erode(gray_image, erode_dilate_image, element);

    // 高斯滤波处理
    Mat gaussian_image;
    GaussianBlur(erode_dilate_image, gaussian_image, Size(1, 1), 0, 0, BORDER_DEFAULT);

    // 使用大津法进行二值化处理
    Mat threshold_image;
    threshold(gaussian_image, threshold_image, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);

    // canny边缘检测
    Mat canny_image;
    Canny(threshold_image, canny_image, 50, 150, 3, false);

    vector<Vec4f> lines;

    // 获取霍夫曼直线
    HoughLinesP(canny_image, lines, 2, CV_PI / 180, 60, 100, 5);
    printf("canny_image_lines_size:  %d\n", lines.size());
    
    // 计算平均角度
    double angle = 0;
    int num = 0;
    // 遍历所有直线
    for(int i = 0;i < lines.size();i++){
        double x_item = lines[i].val[0] - lines[i].val[2];
        double y_item = lines[i].val[1] - lines[i].val[3];
        // if(abs(x_item / y_item) >= 1) continue;

        // 画条线调试一下
        Vec4f hline = lines[i];
		line(src_copy_image, Point(hline[0], hline[1]), Point(hline[2], hline[3]), Scalar(0, 255, 0), 2);
        // 利用反三角函数求出来角度加到angle中，方便求平均
        double angle_item = x_item / y_item;
        angle += atan(angle_item) * 180 / CV_PI;
        num ++;
        printf("lines[%d]: %lf\n", i, angle_item * 180 / CV_PI);
    }
    // 计算平均
    angle /= lines.size();
    printf("average_angle: %lf\n", angle);

    Point2f center = Point2f(src_image.rows / 2.0, src_image.cols / 2.0);

    Mat get_temp_mat = getRotationMatrix2D(center, -angle, 1.0);
    Mat rotated_image;
    cv::Size src_sz = threshold_image.size();
    cv::Size dst_sz(src_sz.width * 1.2, src_sz.height);
    // 对输出的图像处理
    warpAffine(threshold_image, rotated_image, get_temp_mat, dst_sz);
    // 对原始图像处理，方便调试
    warpAffine(src_copy_image, src_copy_image, get_temp_mat, dst_sz);

    puts("......");
    // 然后再在水平方向上拉直
    // 获取霍夫曼直线
    Mat canny_rotated_image;
    lines.clear();
    Canny(rotated_image, canny_rotated_image, 50, 150, 3, false);
    HoughLinesP(canny_rotated_image, lines, 2, CV_PI / 180, 60, 50, 74);
    printf("canny_image_lines_size:  %d\n", lines.size());
    
    angle = 0; num = 0;
    for(int i = 0;i < lines.size();i++){
        double x_item = lines[i].val[0] - lines[i].val[2];
        double y_item = lines[i].val[1] - lines[i].val[3];
        if(abs(y_item / x_item) >= 1) continue;
        if(lines[i].val[1] > canny_rotated_image.rows / 2 && lines[i].val[3] > canny_rotated_image.rows / 2) continue;

        // 画条线调试一下
        Vec4f hline = lines[i];
		line(src_copy_image, Point(hline[0], hline[1]), Point(hline[2], hline[3]), Scalar(0, 0, 255), 2);

        // 计算角度，并加到angle中求平均
        double angle_item = y_item / x_item;
        angle += atan(angle_item) * 180 / CV_PI;
        num ++;
        printf("lines[%d]: %lf\n", i, angle_item * 180 / CV_PI);
    }
    angle /= num;

    Mat temp_for_x = getRotationMatrix2D(center, angle, 1.0);
    Mat res_image; // 最终图像
    // 对输出的图像处理
    warpAffine(rotated_image, res_image, temp_for_x, dst_sz);
    // 对输入图像处理，方便调试
    warpAffine(src_copy_image, src_copy_image, temp_for_x, dst_sz);

    FloodFill(res_image);

    vector<int> rows_element;
    vector<Point> points;
    
    for(int i = 0;i < res_image.rows;i++){
        int sum = 0;
        uchar* ff = res_image.ptr(i);
        for(int j = 0;j < res_image.cols;j++){
            if(*(ff + j) >= 100) sum ++;
        }
        rows_element.push_back(sum);
        points.push_back(Point(sum / 2, i));
        if(i) line(src_copy_image, points[i - 1], points[i], Scalar(255, 0, 0), 2);
    }

    int idx = -1;
    vector<PIII> ans;
    for(int i = 0;i < rows_element.size() / 2;i++){
        if(rows_element[i] >= 70 && rows_element[i] <= 299){
            PIII item = {++ idx, {i, 0}};
            ans.push_back(item);
            int idx = i;
            while(rows_element[idx] >= 50) idx ++;
            ans[item.first].second.second = idx;
            i = ++ idx;
        }
    }

    int _begin = ans[ans.size() - 2].second.first, _end = ans[ans.size() - 2].second.second;

    // 提取兴趣框
    Mat ROI_image = res_image(Range(_begin, _end), Range::all());

    vector<int> num_area;

    for(int i = 0;i < ROI_image.cols;i++){
        int num = 0;
        for(int j = 0;j < ROI_image.rows;j++){
            uchar * ch = ROI_image.ptr(j);
            if(*(ch + i) >= 60) num ++;
        }
        num_area.push_back(num);
    }

    vector<PII> num_position;
    vector<Mat> num_ROI_rect;
    for(int i = 0;i < num_area.size();i++){
        if(num_area[i]){
            PII item = {i - 1, 0};
            int idx = i;
            while(num_area[idx]) idx ++;
            item.second = idx;
            i = idx;
            num_position.push_back(item);
        }
    }

    for(int i = 0;i < num_position.size();i++){
        Mat item_image = ROI_image(Range::all(), Range(num_position[i].first, num_position[i].second));
        num_ROI_rect.push_back(item_image);
        string str = "images(";
        imshow(str + to_string(i) + ")", item_image);
        waitKey(0);
    }

    // 调试代码，可以改宏定义
    

    #ifdef DEBUG_LINE // 调试直线检测
    imshow("lines_image", src_copy_image);
    #endif

    #ifdef DEBUG // 调试图像
    // imshow("res_ROI_image", ROI_image); // 最后输出的图片为res_image
    #endif
    waitKey(0);

    return 0;
}