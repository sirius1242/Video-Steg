#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <gsl/gsl_wavelet2d.h>

#include "ecc.hpp"

#define BLOCK_SIZE 8
#define Q 32

cv::Mat steg(cv::Mat img, int width, std::string key, int keysize);
std::string solve(cv::Mat img, int width);
