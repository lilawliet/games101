#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0],                          // 圆心移动到原点
                 0, 1, 0, -eye_pos[1], 
                 0, 0, 1, -eye_pos[2], 
                 0, 0, 0, 1;
                 
    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f z_rotate;
    float radian = rotation_angle/180.0*MY_PI;

    z_rotate <<     cos(radian) , (-1)*sin(radian)  ,  0,  0,       // 以z轴为旋转轴旋转某角度
                    sin(radian) , cos(radian)       ,  0,  0,
                    0           , 0                 ,  1,  0,
                    0           , 0                 ,  0,  1;
    
    model = model * z_rotate;

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f persp2ortho = Eigen::Matrix4f::Identity();      // 将透视投影转换为正交投影到矩阵 （摆正到视平面）
    float n = -1 * zNear;
    float f = -1 * zFar;
    
    persp2ortho <<      zNear   , 0     , 0             , 0                 ,
                        0       , zNear , 0             , 0                 ,
                        0       , 0     , zNear + zFar  , (-1)*zNear*zFar   ,
                        0       , 0     , 1             , 0                 ;
                
    float halfEyeAngelRadian = eye_fov / 180.0*MY_PI;       // y轴 视角/视锥 
    float t = zNear * std::tan(halfEyeAngelRadian/2.0);     // y轴 可视最高点 （屏幕中心点为0)
    float r = t * aspect_ratio;                             // x轴 可视最右侧
    float l = (-1) * r;                                     // x轴 可视最左侧
    float b = (-1) * t;                                     // y轴 可视最低点

    std::cout << l << r << std::endl;
  
    Eigen::Matrix4f ortho1 = Eigen::Matrix4f::Identity();  // 进行一定的缩放使之成为一个标准到长度为2的正方体 （标准化/单位化）
    ortho1 <<   2/(r-l) , 0         , 0             , 0 ,
                0       , 2/(t-b)   , 0             , 0 ,
                0       , 0         , 2/(zNear-zFar), 0 ,
                0       , 0         , 0             , 1 ;

    Eigen::Matrix4f ortho2 = Eigen::Matrix4f::Identity();   // 把长方体到中心移动到原点 （摆正到圆心）
    ortho2 <<   1 , 0 , 0 , (-1)*(r+l)/2        ,
                0 , 1 , 0 , (-1)*(t+b)/2        ,
                0 , 0 , 1 , (-1)*(zNear+zFar)/2 ,
                0 , 0 , 0 , 1                   ; 
    
    Eigen::Matrix4f matrix_ortho = ortho1 * ortho2;
    projection = matrix_ortho * persp2ortho;
    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
