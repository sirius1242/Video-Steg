#include "steg.hpp"

using namespace cv;;

int steg(Mat img, std::string key)
{
	int level = 1;
	for(int i=0; i<img.rows/BLOCK_SIZE; i++)
	{
		for(int j=0;j<img.cols/BLOCK_SIZE; j++)
		{
			//Mat Block = Mat::zeros(BLOCK_SIZE, BLOCK_SIZE, CV_8UC3);
			//std::cout << Block << std::endl;
			Rect roi(i*BLOCK_SIZE, j*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
			Mat Block = img(roi);
			//img(roi).copyTo(Block);
			Mat_<float> S, U, VT;
			haar_2d(Block, level, img.type());
			Block.convertTo(Block, CV_32F, 1.0);
			SVDecomp(Block, S, U, VT, SVD::FULL_UV);
			std::cout << S.at<float>(0, 0) << std::endl;
			S.at<float>(0, 0) -= 1;
			Mat_<float> tmp = U*Mat::diag(S)*VT;
			tmp.convertTo(Block, img.type(), 1);
			haar_2d_inverse(Block, level, img.type());
			//std::cout << Block << std::endl;
			Block.copyTo(img(roi));
		}
	}
}

std::string solve(Mat img)
{

}
