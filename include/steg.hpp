#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

int steg(cv::Mat img, std::string key);
std::string solve(cv::Mat img);
