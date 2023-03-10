#include "detect.h"
double detectSolution::CalcImg(Mat inputImg) 
{
    double nums = 0;
    int temp = 0;
    for (int i = 0; i < inputImg.rows; i++) 
    {
        for (int j = 0; j < inputImg.cols; j++) 
        {
            if (inputImg.ptr(i)[j] != 0) 
            {
                nums += inputImg.ptr(i)[j];
                temp++;
            }
        }
    }
    return nums / (inputImg.rows * inputImg.cols);
}

int SortMid(int val[])
{
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 8 - i; j++)
            if (val[j] > val[j + 1])
            {
                int tmp = val[j];
                val[j] = val[j + 1];
                val[j + 1] = tmp;
            }
    return val[4];
}


void ImgDenoise(Mat& pic, Mat& ImgClear)//中值滤波去噪
{
    int dx[] = { 0,-1,0,1,-1,1,-1,0,1 };
    int dy[] = { 0,1,1,1,0,0,-1,-1,-1 };
    ImgClear = Mat(pic.rows, pic.cols, CV_8UC1);
    int val[10], mid;
    for (int i = 0; i < pic.rows; i++)//跳过边缘不处理
        for (int j = 0; j < pic.cols; j++)
        {
            if (i == 0 || j == 0 || i == pic.rows - 1 || j == pic.cols - 1)
            {
                ImgClear.at<uchar>(i, j) = pic.at<uchar>(i, j);
                continue;
            }
            for (int k = 0; k < 9; k++)
                val[k] = pic.at<uchar>(i + dx[k], j + dy[k]);
            mid = SortMid(val);
            ImgClear.at<uchar>(i, j) = mid;
        }
}

void detectSolution::get_average_light(Mat _src) 
{
    double sum = 0;
    for (int i = 0; i < _src.rows; i++) 
    {
        for (int j = 0; j < _src.cols; j++) 
        {
            sum += _src.ptr(i)[j];
        }
    }
    this->average = sum / (_src.rows * _src.cols);
}



// pair比较函数
bool MCompare(pair<int, double>a, pair<int, double>b) 
{ 
    return a.second < b.second; 
}

//模板匹配的主要函数
char detectSolution::CheckImg(Mat inputImg, int idx) 
{
    // cout << "size: " << inputImg.rows * inputImg.cols << endl;
    // 如果图片过小，就直接返回空格
    int S = inputImg.rows * inputImg.cols;
    if(S <= 630 && (double)inputImg.rows / inputImg.cols < 1) return ' ';
    //读取模板图片
    vector<String> sampleImgFN;
    glob(sampleImgPath, sampleImgFN, false);
    int sampleImgNums = sampleImgFN.size();

    pair<int, double>* nums = new pair<int, double>[sampleImgNums];//first 记录模板的索引号，second 记录两图像之差
    for (int i = 0; i < sampleImgNums; i++) 
    {
        Mat numImg = imread(sampleImgFN[i], 0);
        Mat delImg, demo_del_image;
        resize(inputImg, inputImg, Size(numImg.cols, numImg.rows));
        absdiff(numImg, inputImg, delImg);

        double res;
        // 尝试使用库函数中的模板匹配
        matchTemplate(inputImg, numImg, demo_del_image, CV_TM_SQDIFF_NORMED);


        nums[i].first = i;
        // nums[i].second = CalcImg(delImg);
        nums[i].second = CalcImg(delImg);
    }

    sort(nums, nums + sampleImgNums, MCompare);//选择差值最小的模板
    int ans_idx = 0;
    for (int i = 0; i < sampleImgNums; ++i) {
        if (idx > 5) 
        {   // 数字，就不用判断字母了
            // 找到最靠前的数字
            while (nums[ans_idx].first / 3 >= 10 || nums[ans_idx].first == 14) ans_idx++;
        }
        else if (idx < 2) 
        {   // 字母
            // 找到最靠前的字母
            while (nums[ans_idx].first / 3 <= 9 && nums[ans_idx].first != 14) ans_idx++;
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
        for (int j = 0; j < 1 || (ROI_range.start < ROI_range.end && j < ROI_range.start); j++)
            if (pic.at<uchar>(j, i) != 0) q.push({ j,i });
    for (int i = 0; i < pic.cols; i++)//下
        for (int j = pic.rows - 1; j >= pic.rows - 1; j--)
            if (pic.at<uchar>(j, i) != 0) q.push({ j,i });
    for (int i = 0; i < pic.rows; i++)//左
        for (int j = 0; j < 1 || (ROI_range_x.start < ROI_range_x.end && j < ROI_range_x.start); j++)
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
            // 如果越过字符所在行，直接跳过循环
            bool f1 = false, f2 = false;
            if(ROI_range.start < ROI_range.end && (nx >= ROI_range.start && nx <= ROI_range.end)) {
                f1 = true;
            }
            if(ROI_range_x.start < ROI_range_x.end && (ny >= ROI_range_x.start && ny <= ROI_range_x.end)) {
                f2 = true;
            }
            if(f1 && f2) continue;
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
    // 画出直线

#ifdef DEBUG_LINES
    //依次画出每条线段
    for (size_t i = 0; i < Line.size(); i++)
    {
        float rho = Line[i][0];
        float theta = Line[i][1];
        Point pt1, pt2;
        //cout << theta << endl;
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        pt1.x = cvRound(x0 + 1000 * (-b));
        pt1.y = cvRound(y0 + 1000 * (a));
        pt2.x = cvRound(x0 - 1000 * (-b));
        pt2.y = cvRound(y0 - 1000 * (a));
        if((pt1.y >= this->res_image.rows * 0.50 && pt2.y >= this->res_image.rows * 0.50)) continue;
        //只选角度最小的作为旋转角度
        //sum += theta;

        line(src_copy_image, pt1, pt2, Scalar(0, 255, 0), 1, LINE_AA); //Scalar函数用于调节线段颜色
    }
#endif

    //计算偏转角度
    double Angle = 0;
    int LineCnt = 0;
    for (int i = 0; i < Line.size(); i++)
    {
        float rho = Line[i][0];
        float theta = Line[i][1];
        Point pt1, pt2;
        //cout << theta << endl;
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        double y1 = cvRound(y0 + 1000 * (a));
        double y2 = cvRound(y0 - 1000 * (a));
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




// 预处理模式三
Mat detectSolution::get_res_image3(Mat& src_image, int type){
    
    cv::copyMakeBorder(src_image, src_image, 15, 5, 5, 0, BORDER_CONSTANT, Scalar(179, 164, 161)); //161,164,179
    // 复制Mat
    this->src_image.copyTo(src_copy_image);

    // 灰度化处理
    cvtColor(src_image, gray_image, COLOR_RGB2GRAY);

    // 滤波用的核
    Mat dilate_image,erode_image;
    Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));

    // 膨胀腐蚀
    dilate(gray_image, erode_image, element);
    erode(erode_image, erode_image, element);

    // 自己实现的中值滤波
    Mat gaussian_image, bilateral_image;
    ImgDenoise(erode_image, gaussian_image);

#ifdef DEBUG_BLUR
   imshow("gaussian_image", gaussian_image);
   // imshow("bilateral_image", bilateral_image);
   waitKey(0);
#endif

    // 获取平均亮度
    this->get_average_light(gaussian_image);

    // 使用大津法进行二值化处理
    threshold(gaussian_image, threshold_image, 0, 255, type | THRESH_OTSU);

    // 二值化调试

#ifdef DEBUG_THRESHOLD
   imshow("threshold_image", threshold_image);
   waitKey(0);
#endif

    Mat res_image;
    ImgRectify(threshold_image, res_image);

    return res_image;
}




Mat detectSolution::get_res_image2(Mat& src_image, int type){
    // 复制Mat
    this->src_image.copyTo(src_copy_image);

    // 灰度化处理
    cvtColor(src_image, gray_image, COLOR_RGB2GRAY);

    // 滤波用的核
    Mat dilate_image,erode_image;
    Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));

    // 膨胀腐蚀
    dilate(gray_image, erode_image, element);
    erode(erode_image, erode_image, element);

    // 自己实现的中值滤波
    Mat gaussian_image, bilateral_image;
    ImgDenoise(erode_image, gaussian_image);

#ifdef DEBUG_BLUR
   imshow("gaussian_image", gaussian_image);
   // imshow("bilateral_image", bilateral_image);
   waitKey(0);
#endif

    // 获取平均亮度
    this->get_average_light(gaussian_image);

    // 使用大津法进行二值化处理
    threshold(gaussian_image, threshold_image, 0, 255, type | THRESH_OTSU);

    // 二值化调试
#ifdef DEBUG_THRESHOLD
   imshow("threshold_image", threshold_image);
   waitKey(0);
#endif

    Mat res_image;
    ImgRectify(threshold_image, res_image);

    return res_image;
}






// 图像预处理操作
Mat detectSolution::get_res_image(Mat& src_image, int type) {
    // 复制Mat
    this->src_image.copyTo(src_copy_image);

    // 灰度化处理
    cvtColor(src_image, gray_image, COLOR_RGB2GRAY);

    Mat dilate_image,erode_image;
    Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));

    dilate(gray_image, erode_image, element);
    erode(erode_image, erode_image, element);


    // 高斯滤波处理
    Mat gaussian_image, bilateral_image;

    ImgDenoise(erode_image, gaussian_image);
#ifdef DEBUG_BLUR

   imshow("gaussian_image", gaussian_image);
   // imshow("bilateral_image", bilateral_image);
   waitKey(0);

#endif

    // 获取平均亮度
    this->get_average_light(gaussian_image);

    // 使用自适应法进行二值化处理
    adaptiveThreshold(gaussian_image, threshold_image, 255, ADAPTIVE_THRESH_MEAN_C, type, 159, 18);

    // 二值化调试
#ifdef DEBUG_THRESHOLD
   imshow("threshold_image", threshold_image);
   waitKey(0);
#endif

    Mat res_image;
    ImgRectify(threshold_image, res_image);

    return res_image;
}

// resize_stand函数，将图片resize成为较小尺寸，减少计算量
void detectSolution::resize_stand() {
    double width = 1200;
    double height = width * (double)src_image.rows / src_image.cols;
    resize(src_image, src_image, cv::Size(width, height));
}


void detectSolution::find_ROI() {
    // 先将上次的工作清除
    ans.clear();
    points.clear();
    rows_element.clear();
    for (int i = 0; i < res_image.rows; i++) {
        int sum = 0;
        uchar* ff = res_image.ptr(i);
        for (int j = 0; j < res_image.cols; j++) {
            if (*(ff + j) >= 100) sum++;
        }
        rows_element.push_back(sum);
        points.push_back(Point(sum / 2, i));
        if (i) line(src_copy_image, points[max(0, i - 1)], points[i], Scalar(255, 0, 0), 2);
    }

    int idx = -1;
    for (int i = 0; i < rows_element.size() / 2; i++) {
        if (rows_element[i] >= 35) {
            PIII item = { ++idx, {i, 0} };
            ans.push_back(item);
            int idx = i;
            while (rows_element[idx] >= 35){ 
                idx ++;
            }
            ans[item.first].second.second = idx;
            i = ++idx;
        }
    }
    int _begin = ans[max(0, (int)ans.size() - 2)].second.first, _end = ans[max(0, (int)ans.size() - 2)].second.second;
    // 如果没有提取到导致开始小于结尾，或者太大，直接返回

    priority_queue<double> heap; // 最大堆
    if(_end - _begin >= 400 || _end - _begin <= 40){
        ans.clear();
        idx = -1;
        for (int i = 0; i < rows_element.size() / 2; i++) {
            if (rows_element[i] >= 35 && (!heap.size() || heap.top() - rows_element[i] <= 300)) {
                heap.push(rows_element[i]);
                PIII item = { ++idx, {i, 0} };
                ans.push_back(item);
                int idx = i;
                while (rows_element[idx] >= 35 && heap.top() - rows_element[idx] <= 300){ 
                    heap.push(rows_element[idx ++]);
                }
                ans[item.first].second.second = idx;
                i = ++idx;
            } else {
                while(heap.size()) heap.pop();
            }
        }
        _begin = ans[max(0, (int)ans.size() - 2)].second.first, _end = ans[max(0, (int)ans.size() - 2)].second.second;
    }


    if (_begin >= _end || _begin > 114514) return;

    // 保存兴趣框位置
    this->ROI_range = Range(_begin, _end);

    // 提取兴趣框
    this->ROI_image = res_image(Range(_begin, _end), Range::all());

    num_area.clear();
    num_position.clear();
    for (int i = 0; i < ROI_image.cols; i++) {
        int num = 0;
        for (int j = 0; j < ROI_image.rows; j++) {
            uchar* ch = ROI_image.ptr(j);
            if (*(ch + i) >= 103) num++;
        }
        num_area.push_back(num);
    }

    for (int i = 0; i < num_area.size(); i++) {
        if (num_area[i] >= 2) {
            PII item = { max(i - 1, 0), 0 };
            int idx = i;
            while (num_area[idx]) idx++;
            item.second = idx;
            i = idx;
            num_position.push_back(item);
        }
    }
    // 保存x方向的边
    this->ROI_range_x.start = num_position[0].first;
    this->ROI_range_x.end = num_position[num_position.size() - 1].second;
}



/**
 * @authors 可莉不知道哦
 * @brief 本文件写的是detectSolution对外部的接口
 * @details fit test
*/

/// 外部接口
int detectSolution::fit(string src_path, int model) {
    // 输入图像
    this->src_image = imread(src_path);

    // 加入白边
    // cv::copyMakeBorder(src_image, src_image, 10, 0, 0, 0, BORDER_CONSTANT, Scalar(0, 0, 0)); // 158,159,178

    if (!this->src_image.data) {
        cout << "src_image_empty!" << endl;
        return ERROR;
    }

    // 图像标准化，并进行预处理
    this->resize_stand();



    // 模式选择
    if(model == 0){ // 自适应
        this->res_image = this->get_res_image(this->src_image, THRESH_BINARY_INV);
        // 计算图像的平均像素值
        double _sum = CalcImg(this->res_image);
        // 如果平均像素值过小，那么换一个参数重新进行预处理
        if (_sum <= 14) this->res_image = get_res_image(this->src_image, THRESH_BINARY);
    } else if(model == 1) { // 大津法
        this->res_image = this->get_res_image2(this->src_image, THRESH_BINARY_INV);
        // 计算图像的平均像素值
        double _sum = CalcImg(this->res_image);
        // 如果平均像素值过小，那么换一个参数重新进行预处理
        if (_sum <= 14) this->res_image = get_res_image2(this->src_image, THRESH_BINARY);
    } else {
        this->res_image = this->get_res_image3(this->src_image, THRESH_BINARY_INV);
        // 计算图像的平均像素值
        double _sum = CalcImg(this->res_image);
        // 如果平均像素值过小，那么换一个参数重新进行预处理
        if (_sum <= 14) this->res_image = get_res_image3(this->src_image, THRESH_BINARY);
    }


    // 寻找ROI区域,保存ROI区域
    this->find_ROI();

    // 再次旋转
    ImgRectify(threshold_image, res_image);

    // 再次寻找ROI
    this->find_ROI();

#ifdef DEBUG_ROI

    imshow("ROI_image", ROI_image);
    waitKey(0);

#endif

#ifdef DEBUG_LINES

    imshow("lines_image", this->src_copy_image);
    waitKey(0);

#endif

#ifdef DEBUG_RES

    imshow("res", this->res_image);
    waitKey(0);
#endif

    // 准备获取ans
    vector<char> ans_;
    this->res_str = "";
    int idx_image = 0;

    // 开始模板匹配
    for (int i = 0; i < num_position.size(); i++) {
        Mat item_image = ROI_image(Range::all(), Range(num_position[i].first, num_position[i].second));

        int head = 0;
        for(int j = 0;j < item_image.rows;j++){
            auto ff = item_image.ptr(j);
            int num = 0;
            for(int k = 0;k < item_image.cols;k++){
                num += ff[k];
            }
            if(num){
                head = j;
                break;
            }
        }

        int end = 0;
        for(int j = item_image.rows - 1;j >= 0;j --){
            auto ff = item_image.ptr(j);
            int num = 0;
            for(int k = 0;k < item_image.cols;k++){
                num += ff[k];
            }
            if(num){
                end = j;
                break;
            }
        }

        item_image = item_image(Range(head, end), Range::all());

        #ifdef DEBUG_ITEM
        imwrite("demo_wobuhaoshuo/" + to_string(i) + ".jpg", item_image);
        imshow("item_image", item_image);
        waitKey(0);
        #endif
        
        num_ROI_rect.push_back(item_image);
        char ch = CheckImg(item_image, i + 1); // 传入image和第几个

        ans_.push_back(ch);
        // 前期工作代码
        // imwrite("D:\\ISBN\\demo\\" + to_string(idx_image ++) + ".jpg", item_image); // 存储自制模板
    }
    bool flag = false;

    // 整理结果
    for (int i = 0; i < ans_.size() && res_str.length() < 17; i++) 
    {
        if (flag) {
            res_str += ans_[i];
        }
        else if ((ans_[i] == 'N' || ans_[i + 1] == '9') && i >= 2) 
        {
            flag = true;
        } else if((ans_[i - 1] == 'N' && ans_[i] == '9') && i < 2){
            res_str += ans_[i];
            flag = true;
        }
    }
    return DONE;
}


string detectSolution::get_res() 
{
    return this->res_str;
}

double detectSolution::getChNum() 
{
    return this->ChNum;
}

double detectSolution::getStrNum() 
{
    return this->StrNum;
}