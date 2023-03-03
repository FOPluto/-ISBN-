#include "detect.h"




double detectSolution::CalcImg(Mat inputImg) {
    double nums = 0;
    int temp = 0;
    for (int i = 0; i < inputImg.rows; i++) {
        for (int j = 0; j < inputImg.cols; j++) {
            if (inputImg.ptr(i)[j] != 0) {
                nums += inputImg.ptr(i)[j];
                temp ++;
            }
        }
    }
    return nums / (inputImg.rows * inputImg.cols);
}


void detectSolution::get_average_light(Mat _src){
    double sum = 0;
    for(int i = 0;i < _src.rows;i++){
        for(int j = 0;j < _src.cols;j++){
            sum += _src.ptr(i)[j];
        }
    }
    this->average =  sum / (_src.rows * _src.cols);
}



// pair比较函数
bool MCompare(pair<int, double>a, pair<int, double>b) {return a.second < b.second;}

//模板匹配的主要函数
char detectSolution::CheckImg(Mat inputImg, int idx) {
    //读取模板图片
    vector<String> sampleImgFN;
    glob(sampleImgPath, sampleImgFN, false);
    int sampleImgNums = sampleImgFN.size();

    pair<int, double>*nums = new pair<int, double>[sampleImgNums];//first 记录模板的索引号，second 记录两图像之差
    for (int i = 0; i < sampleImgNums; i++) {
        
        Mat numImg = imread(sampleImgFN[i], 0);
        Mat delImg;
        resize(inputImg, inputImg, Size(numImg.cols, numImg.rows));
        absdiff(numImg, inputImg, delImg);

        nums[i].first = i;
        nums[i].second = CalcImg(delImg);
    }

    sort(nums, nums + sampleImgNums, MCompare);//选择差值最小的模板
    int ans_idx = 0;
    for(int i = 0;i < sampleImgNums; ++i){
        if(idx > 5){ // 数字，就不用判断字母了
            // 找到最靠前的数字
            while(nums[ans_idx].first / 3 >= 10 && nums[ans_idx].first / 3 != 15) ans_idx ++;
        } else if(idx < 3){ // 字母
            // 找到最靠前的字母
            while(nums[ans_idx].first / 3 <= 9) ans_idx ++;
        }
    }

    int index = nums[ans_idx].first;

    index = index / 3;
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


// 旋转操作
void detectSolution::ImgRectify(Mat& pic, Mat& BinaryFlat)//图像矫正
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
    Size src_size = Size(pic.cols * 1.42, pic.rows);
	warpAffine(pic, BinaryFlat, pic_tmp, src_size);
	warpAffine(this->src_copy_image, this->src_copy_image, pic_tmp, src_size);
	FloodFill(BinaryFlat);
}







// 图像预处理操作
Mat detectSolution::get_res_image(Mat& src_image, int type){
    // 复制Mat
    this->src_image.copyTo(src_copy_image);

    // 灰度化处理
    cvtColor(src_image, gray_image, COLOR_RGB2GRAY);

    Mat erode_dilate_image;
    Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
    // 高斯滤波处理
    Mat gaussian_image, bilateral_image;
    GaussianBlur(gray_image, gaussian_image, Size(3, 3), 0, 0, BORDER_DEFAULT);

    // bilateralFilter(gray_image, bilateral_image,  20, 200, 20);
    
    #ifdef DEBUG_BLUR

    imshow("gaussian_image", gaussian_image);
    // imshow("bilateral_image", bilateral_image);
    waitKey(0);

    #endif

    // 获取平均亮度
    this->get_average_light(gaussian_image);

    // 使用大津法进行二值化处理
    adaptiveThreshold(gaussian_image, threshold_image, 255, ADAPTIVE_THRESH_MEAN_C, type, 159, 18);
    // 二值化调试
    #ifdef DEBUG_THRESHOLD
    imshow("threshold_image", threshold_image);
    imshow("gray_image", gray_image);
    waitKey(0);
    #endif

    Mat res_image;
    ImgRectify(threshold_image, res_image);

    return res_image;
}




// resize_stand函数，将图片resize成为较小尺寸，减少计算量
void detectSolution::resize_stand(){
    double width = 1200;
    double height = width * (double)src_image.rows / src_image.cols;
    resize(src_image, src_image, cv::Size(width, height));
}


void detectSolution::find_ROI(){
    for(int i = 0;i < res_image.rows;i++){
        int sum = 0;
        uchar* ff = res_image.ptr(i);
        for(int j = 0;j < res_image.cols;j++){
            if(*(ff + j) >= 100) sum ++;
        }
        rows_element.push_back(sum);
        points.push_back(Point(sum / 2, i));
        if(i) line(src_copy_image, points[max(0, i - 1)], points[i], Scalar(255, 0, 0), 2);
    }

    int idx = -1;
    for(int i = 0;i < rows_element.size() / 2;i++){
        if(rows_element[i] >= 42){
            PIII item = {++ idx, {i, 0}};
            ans.push_back(item);
            int idx = i;
            while(rows_element[idx] >= 42) idx ++;
            ans[item.first].second.second = idx;
            i = ++ idx;
        }
    }

    #ifdef DEBUG
    imshow("src_image", src_image);
    imshow("res_image", res_image);
    waitKey(0);
    #endif

    int _begin = ans[max(0, (int)ans.size() - 2)].second.first, _end = ans[max(0, (int)ans.size() - 2)].second.second;

    // 如果没有提取到，直接返回
    if(_begin >= _end) return;

    // 提取兴趣框
    this->ROI_image = res_image(Range(_begin, _end), Range::all());


    for(int i = 0;i < ROI_image.cols;i++){
        int num = 0;
        for(int j = 0;j < ROI_image.rows;j++){
            uchar * ch = ROI_image.ptr(j);
            if(*(ch + i) >= 103) num ++;
        }
        num_area.push_back(num);
    }

    for(int i = 0;i < num_area.size();i++){
        if(num_area[i] >= 6){
            PII item = {max(i - 1, 0), 0};
            int idx = i;
            while(num_area[idx]) idx ++;
            item.second = idx;
            i = idx;
            num_position.push_back(item);
        }
    }
}



/**
 * @authors 可莉不知道哦
 * @brief 本文件写的是detectSolution对外部的接口
 * @details fit test
*/

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
    // cout << "average_: " << _sum << endl;

    // 如果平均像素值过小，那么换一个参数重新进行预处理
    if(_sum <= 14) this->res_image = get_res_image(this->src_image, THRESH_BINARY);


    #ifdef DEBUG_RES

    imshow("res", this->res_image);
    waitKey(0);
    #endif

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
        char ch = CheckImg(item_image, i + 1); // 传入image和第几个

        ans_.push_back(ch);
        //imwrite("demo/" + to_string(idx_image ++) + ".jpg", item_image); // 存储自制模板
    }
    bool flag = false;

    // 整理结果
    for(int i = 0;i < ans_.size() && res_str.length() < 17;i++){
        if(flag){
            res_str += ans_[i];
        }else if(ans_[i] == 'N' || ans_[i + 1] == '9'){
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