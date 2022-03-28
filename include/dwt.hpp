//https://github.com/Cuda-Chen/haar2d/blob/master/dwt.hpp
#ifndef DWT_HPP
#define DWT_HPP

#include <opencv2/opencv.hpp>

void haar_2d(cv::Mat &src, int level, int type);
void haar_2d_inverse(cv::Mat &src, int level, int type);

#endif /* dwt.hpp  */
