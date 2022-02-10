#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    // if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) 
    if (event == cv::EVENT_LBUTTONDOWN)
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    // TODO: Implement de Casteljau's algorithm
    if (control_points.size() > 2){
        
        cv::Point2f point = control_points[0];
        std::vector<cv::Point2f> temp;

        for(int i = 0; i < control_points.size() - 1; i++){
            temp.push_back(control_points[i] + t * (control_points[i+1] - control_points[i]));
        }

        // std::cout<<temp.size()<<std::endl;
        return recursive_bezier(temp, t);
    }
    
    return control_points[0] + t * (control_points[1] - control_points[0]);
}

//计算阶乘
/*
注意：0！= 1
*/
int factorial(int n)
{
    int fc=1;
    for(int i=1;i<=n;++i) fc *= i;
    return fc;
}

//计算组合数
/*从n个不同元素中取出m(m≤n)个元素的所有组合的个数，叫做n个不同元素中取出m个元素的组合数。用符号c(n,m) 表示。
组合数公式：c(n,m)=n!/(m! * (n-m)!)
性质：c(n,m) = c(n,m-n)
递推公式：c(n,m) = c(n-1,m-1) + c(n-1,m)
*/
int combo(int n,int m)
{
    int com=factorial(n)/(factorial(m)*factorial(n-m));
    return com;
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
    
    for (double t = 0.0; t <= 1.0; t += 0.001) {

        // 算术法
        // 加法中各项的（伯恩斯坦多项式展开项）一般代数公式： a * t^b * (1-t)^c * Pn
        // 假设 n = 5
        // a    1   5   10  5   1
        // b    1   2   3   4   5
        // c    5   4   3   2   1
        int n = control_points.size() - 1;

        cv::Point2f point = std::pow((1 - t), n) * control_points[0];       // 化简后的第一项： combo(n, 0) * t^0 * (1-t)^n * point[0]
        for (int i = 1 ; i <= n; i++){
            int a = combo(n , i);
            int b = i;
            int c = n - i;
            std::cout<< a <<" * (1 - t)^ " << b << " * t^ " << c << " * Pn" << std::endl;
            point += a * std::pow(t, b) * std::pow(1-t, c) * control_points[i];
        }
        window.at<cv::Vec3b>(point.y, point.x)[1] = 255;

        // 递归法
        // auto point = recursive_bezier(control_points, t);
        // window.at<cv::Vec3b>(point.y, point.x)[1] = 255;
    }

}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 5) 
        {
            // naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);
            system("pause");
            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
