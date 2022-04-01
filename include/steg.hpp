#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <gsl/gsl_wavelet2d.h>

#include "dwt.hpp"
#define BLOCK_SIZE 8
#define LEVEL 1

void steg(cv::Mat img, std::string key);
std::string solve(cv::Mat img);
