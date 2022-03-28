#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include "dwt.hpp"
#define BLOCK_SIZE 8

int steg(cv::Mat img, std::string key);
std::string solve(cv::Mat img);
