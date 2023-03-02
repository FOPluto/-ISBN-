#include "detect.h"





double detectSolution::CalcImg(Mat inputImg) {
    double nums = 0;
    int temp = 0;
    for (int i = 0; i < inputImg.rows; i++) {
        for (int j = 0; j < inputImg.cols; j++) {
            if (inputImg.at<uchar>(i, j) != 0) {
                nums += inputImg.at<uchar>(i, j);
                temp ++;
            }
        }
    }
    return nums / (inputImg.rows * inputImg.cols);
}






//模板匹配的主要函数
char detectSolution::CheckImg(Mat inputImg) {
    //读取模板图片
    vector<String> sampleImgFN;
    glob(sampleImgPath, sampleImgFN, false);
    int sampleImgNums = sampleImgFN.size();

    pair<int, double>*nums = new pair<int, double>[sampleImgNums];//first 记录模板的索引号，second 记录两图像之差
    for (int i = 0; i < sampleImgNums; i++) {
        nums[i].first = i;
        Mat numImg = imread(sampleImgFN[i], 0);
        Mat delImg;
        resize(inputImg, inputImg, Size(numImg.cols, numImg.rows));
        absdiff(numImg, inputImg, delImg);

        nums[i].second = CalcImg(delImg);
    }

    sort(nums, nums + sampleImgNums, Comp);//选择差值最小的模板

    int index = nums[0].first / 3;
    switch (index) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        return index + '0';
    case 10:
        return 'B';
    case 11:
        return 'I';
    case 12:
        return 'N';
    case 13:
        return 'S';
    case 14:
        return 'X';
    case 15:
        return '-';
    default:
        return ' ';
    }
}





//水漫操作
void detectSolution::FloodFill(Mat& pic)//水漫操作
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




// 图像预处理操作
Mat detectSolution::get_res_image(Mat& src_image, int type){
    // 复制Mat
    this->src_image.copyTo(src_copy_image);

    // 灰度化处理
    Mat gray_image;
    cvtColor(src_image, gray_image, COLOR_RGB2GRAY);

    imshow("gray_image", gray_image);

    Mat erode_dilate_image;
    Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
    dilate(gray_image, erode_dilate_image, element);
    erode(erode_dilate_image, erode_dilate_image, element);

    imshow("erode_dilate_image", erode_dilate_image);

    // 高斯滤波处理
    Mat gaussian_image;
    GaussianBlur(erode_dilate_image, gaussian_image, Size(1, 1), 0, 0, BORDER_DEFAULT);

    imshow("gaussian_image", gaussian_image);

    // 使用大津法进行二值化处理
    threshold(gaussian_image, threshold_image, 0, 255, type | THRESH_OTSU);

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

    // 获取到输出图片的大小，这里宽度在原来基础上要加上一点
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
    return res_image;
}




// resize_stand函数，将图片resize成为较小尺寸，减少计算量
void detectSolution::resize_stand(){
    double width = 1600;
    double height = width * (double)src_image.rows / src_image.cols;
    resize(src_image, src_image, cv::Size(width, height));
}