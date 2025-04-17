/*
 * Software License Agreement (Modified BSD License)
 *
 *  Copyright (c) 2013, PAL Robotics, S.L.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of PAL Robotics, S.L. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/** \author Jordi Pages <jordi.pages@pal-robotics.com> */
/** \author Migrated to ROS2 Humble */

// Project message headers
#include <pal_person_detector_opencv/msg/detections2d.hpp>
#include <pal_person_detector_opencv/msg/detection2d.hpp>


// ROS headers
#include <rclcpp/rclcpp.hpp>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <image_transport/image_transport.hpp>

// OpenCV headers
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// Std C++ headers
#include <vector>
#include <memory>

/**
 * @brief The PersonDetector class encapsulating an image subscriber and the OpenCV's CPU HOG person detector
 *
 * @example ros2 run pal_person_detector_opencv pal_person_detector_opencv --ros-args -p image:=/camera/image -p rate:=5 -p scale:=0.5
 *
 */
class PersonDetector : public rclcpp::Node
{
public:
  PersonDetector(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
  virtual ~PersonDetector();

protected:
  void imageCallback(const sensor_msgs::msg::Image::ConstSharedPtr& msg);

  void detectPersons(const cv::Mat& img,
                     std::vector<cv::Rect>& detections);

  void scaleDetections(std::vector<cv::Rect>& detections,
                       double scaleX, double scaleY) const;

  void publishDetections(const std::vector<cv::Rect>& detections) const;

  void publishDebugImage(cv::Mat& img,
                         const std::vector<cv::Rect>& detections) const;

  double image_scaling_;
  mutable cv_bridge::CvImage cv_img_debug_;

  std::unique_ptr<cv::HOGDescriptor> hog_cpu_;

  image_transport::Subscriber image_sub_;
  rclcpp::Time img_time_stamp_;

  rclcpp::Publisher<pal_person_detector_opencv::msg::Detections2d>::SharedPtr detection_pub_;
  image_transport::Publisher im_debug_pub_;
};

PersonDetector::PersonDetector(const rclcpp::NodeOptions & options)
: rclcpp::Node("person_detector", options)
{
  // Declare and get parameters
  image_scaling_ = this->declare_parameter<double>("scale", 1.0);
  double freq = this->declare_parameter<double>("rate", 10.0);
  std::string img_transport = this->declare_parameter<std::string>("transport", "raw");
  std::string topic = this->declare_parameter<std::string>("image", "/camera/image");
  
  RCLCPP_INFO(this->get_logger(), "Setting image scale factor to: %f", image_scaling_);
  RCLCPP_INFO(this->get_logger(), "Setting detector max rate to: %f", freq);
  RCLCPP_INFO(this->get_logger(), "Image type: %s", img_transport.c_str());
  RCLCPP_INFO(this->get_logger(), "Creating person detector ...");

  hog_cpu_ = std::make_unique<cv::HOGDescriptor>();
  hog_cpu_->setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

  // Setup image transport
  image_sub_ = image_transport::create_subscription(
    this, topic,
    std::bind(&PersonDetector::imageCallback, this, std::placeholders::_1),
    img_transport);

  // Create publishers
  detection_pub_ = this->create_publisher<pal_person_detector_opencv::msg::Detections2d>("detections", 1);
  im_debug_pub_ = image_transport::create_publisher(this, "debug");

  cv::namedWindow("person detections");

  RCLCPP_INFO(this->get_logger(), "Person detector initialized and spinning...");
}

PersonDetector::~PersonDetector()
{
  cv::destroyWindow("person detections");
}

void PersonDetector::imageCallback(const sensor_msgs::msg::Image::ConstSharedPtr& msg)
{
  cv_bridge::CvImageConstPtr cvImgPtr;
  cvImgPtr = cv_bridge::toCvShare(msg);

  img_time_stamp_ = msg->header.stamp;

  cv::Mat img(static_cast<int>(image_scaling_ * cvImgPtr->image.rows),
              static_cast<int>(image_scaling_ * cvImgPtr->image.cols),
              cvImgPtr->image.type());

  if (image_scaling_ == 1.0)
    cvImgPtr->image.copyTo(img);
  else
  {
    cv::resize(cvImgPtr->image, img, img.size());
  }

  std::vector<cv::Rect> detections;

  detectPersons(img, detections);

  if (image_scaling_ != 1.0)
  {
    scaleDetections(detections,
                    static_cast<double>(cvImgPtr->image.cols) / static_cast<double>(img.cols),
                    static_cast<double>(cvImgPtr->image.rows) / static_cast<double>(img.rows));
  }

  publishDetections(detections);

  cv::Mat imDebug = cvImgPtr->image.clone();
  publishDebugImage(imDebug, detections);
}

void PersonDetector::scaleDetections(std::vector<cv::Rect>& detections,
                                     double scaleX, double scaleY) const
{
  for (auto& detection : detections)
  {
    cv::Rect roi(detection);
    detection.x      = static_cast<long>(roi.x      * scaleX);
    detection.y      = static_cast<long>(roi.y      * scaleY);
    detection.width  = static_cast<long>(roi.width  * scaleX);
    detection.height = static_cast<long>(roi.height * scaleY);
  }
}

void PersonDetector::detectPersons(const cv::Mat& img,
                                   std::vector<cv::Rect>& detections)
{
  double start = static_cast<double>(cv::getTickCount());

  hog_cpu_->detectMultiScale(img,
                            detections,
                            0,                //hit threshold: decrease in order to increase number of detections but also false alarms
                            cv::Size(8,8),    //win stride
                            cv::Size(0,0),    //padding 24,16
                            1.02,             //scaling
                            1,                //final threshold
                            false);           //use mean-shift to fuse detections

  double stop = static_cast<double>(cv::getTickCount());
  RCLCPP_DEBUG(this->get_logger(), "Elapsed time in detectMultiScale: %f ms", 
               1000.0 * (stop - start) / cv::getTickFrequency());
}

void PersonDetector::publishDetections(const std::vector<cv::Rect>& detections) const
{
  auto msg = std::make_unique<pal_person_detector_opencv::msg::Detections2d>();
  pal_person_detector_opencv::msg::Detection2d detection;

  msg->header.frame_id = "";
  msg->header.stamp = img_time_stamp_;

  for (const auto& roi : detections)
  {
    detection.x = roi.x;
    detection.y = roi.y;
    detection.width = roi.width;
    detection.height = roi.height;

    msg->detections.push_back(detection);
  }

  detection_pub_->publish(std::move(msg));
}

void PersonDetector::publishDebugImage(cv::Mat& img,
                                       const std::vector<cv::Rect>& detections) const
{
  //draw detections
  for (const auto& roi : detections)
  {
    cv::rectangle(img, roi, CV_RGB(0,255,0), 2);
  }

  if (img.channels() == 3 && img.depth() == CV_8U)
    cv_img_debug_.encoding = sensor_msgs::image_encodings::BGR8;
  else if (img.channels() == 1 && img.depth() == CV_8U)
    cv_img_debug_.encoding = sensor_msgs::image_encodings::MONO8;
  else
    throw std::runtime_error("Error in Detector2dNode::publishDebug: only 24-bit BGR or 8-bit MONO images are currently supported");

  cv_img_debug_.image = img;
  auto imgMsg = std::make_unique<sensor_msgs::msg::Image>();
  imgMsg->header.stamp = img_time_stamp_;
  cv_img_debug_.toImageMsg(*imgMsg); //copy image data to ROS message

  im_debug_pub_.publish(std::move(imgMsg));
}

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<PersonDetector>());
  rclcpp::shutdown();
  return 0;
}