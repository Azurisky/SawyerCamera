#include <cv_bridge/cv_bridge.h>
#include "opencv2/opencv.hpp"
#include <iostream>
#include <string>
#include "ros/ros.h"
#include "std_msgs/String.h" 
#include "sensor_msgs/Image.h"
#include "intera_core_msgs/IOComponentCommand.h"

ros::Publisher image_pub;

void imageCallback(const sensor_msgs::ImageConstPtr& image)
{
    cv_bridge::CvImagePtr cv_img;
    cv::Mat frame;
    try {
        cv_img = cv_bridge::toCvCopy(image, sensor_msgs::image_encodings::BGR8);
        frame = cv_img->image;
        std::cout << frame.rows << " " << frame.cols << std::endl;
        // Turn all pixels to black
        // for (int i = 0; i < frame.rows; i++) {
        //     for (int j = 0; j < frame.cols; j++) {
        //         frame.at<cv::Vec3b>(i, j)[0] = 0; //uchar -> mono
        //         frame.at<cv::Vec3b>(i, j)[1] = 0;
        //         frame.at<cv::Vec3b>(i, j)[2] = 0;
        //     }
        // }

        // Canny Edge Detection
        // cv::cvtColor(frame, frame, CV_BGR2GRAY);
        // cv::Canny(frame, frame, 50, 100);
    }
    catch (cv_bridge::Exception& e) {
        ROS_ERROR("cv_bridge exception: %s", e.what());
    }
    cv::imshow("cap", frame);
    cv::waitKey(1); // 1 millisecond
}

void setStartStreaming(std::string camera_name, bool enable, double _timeout)
{
    ros::Rate loop_rate(100);
    ros::Time start = ros::Time::now();
    while (ros::ok() && (ros::Time::now() - start).toSec() < _timeout)
    {
        intera_core_msgs::IOComponentCommand msg;
        msg.time = ros::Time::now();
        msg.op = "set";
        if(enable)
        {
            msg.args = "{\"signals\": {\"camera_streaming\": {\"data\": [true], \"format\": {\"type\": \"bool\"}}}}";
        }
        else
        {
            msg.args = "{\"signals\": {\"camera_streaming\": {\"data\": [false], \"format\": {\"type\": \"bool\"}}}}";
        }
        image_pub.publish(msg);    
        loop_rate.sleep(); 
    }
    // image_pub.shutdown();
}

void setCameraResolution(std::string camera_name, int height, int width, double _timeout)
{   
    ros::Rate loop_rate(100);
    ros::Time start = ros::Time::now();
    while (ros::ok() && (ros::Time::now() - start).toSec() < _timeout)
    {   
        intera_core_msgs::IOComponentCommand msg;
        msg.time = ros::Time::now();
        msg.op = "set";
        msg.args = "{\"signals\": {\"resolution\": {\"data\": [{\"height\": " + std::to_string(height) + ",\"width\": " + std::to_string(width) + "}], \"format\": {\"type\": \"object\"}}}}";
        image_pub.publish(msg);   
        loop_rate.sleep(); 
    }
    // image_pub.shutdown();
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "videoBridge");
    ros::NodeHandle n;
    std::string camera_name = "head_camera";
    ros::Subscriber sub = n.subscribe("/io/internal_camera/" + camera_name + "/image_raw", 10, imageCallback);
    image_pub = n.advertise<intera_core_msgs::IOComponentCommand>("/io/internal_camera/" + camera_name + "/command", 10);
    
    // close cameras
    setStartStreaming(camera_name, false, 0.5);
    
    // modify the resolution for head camera
    setCameraResolution(camera_name, 800, 1280, 0.5);

    // initialize cameras
    setStartStreaming(camera_name, true, 0.5);

    ros::spin();
    return 0;
}