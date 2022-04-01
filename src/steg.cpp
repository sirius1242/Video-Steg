#include "steg.hpp"

using namespace cv;;

gsl_wavelet *w;
gsl_wavelet_workspace *work;

void steg(Mat img, std::string key)
{
	int type=img.type();

	w = gsl_wavelet_alloc(gsl_wavelet_daubechies, 4);
	work = gsl_wavelet_workspace_alloc(BLOCK_SIZE*BLOCK_SIZE);
	img.convertTo(img, CV_64F, 1.0);
	for(int i=0; i<img.rows/BLOCK_SIZE; i++)
	{
		for(int j=0;j<img.cols/BLOCK_SIZE; j++)
		{
			//Mat Block = Mat::zeros(BLOCK_SIZE, BLOCK_SIZE, CV_8UC3);
			//std::cout << Block << std::endl;
			Rect roi(i*BLOCK_SIZE, j*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
			Mat_<double> Block = img(roi);
			//img(roi).copyTo(Block);
			Mat_<double> S, U, VT;
			//haar_2d(Block, LEVEL, img.type());
			//Block.convertTo(Block, CV_64F, 1.0);
			double *data = Block.ptr<double>();
			gsl_wavelet2d_transform_forward(w, data, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, work);
			SVDecomp(Block, S, U, VT, SVD::FULL_UV);
			if(key.size()>(i*BLOCK_SIZE+j)/8)
			{
				//std::cout << (key[(i*BLOCK_SIZE+j)/8] & (0x80 >> ((i*BLOCK_SIZE+j)%8))) << std::endl;
				//std::cout << S.at<double>(0) << std::endl;
				if (key[(i*BLOCK_SIZE+j)/8] & (0x80 >> ((i*BLOCK_SIZE+j)%8)))
				{
					//std::cout << (key[(i*BLOCK_SIZE+j)/8] & (0x80 >> ((i*BLOCK_SIZE+j)%8))) << std::endl;
					if(((int)(S.at<double>(0)))%2==0)
						S.at<double>(0)+=1;
				}
				else if (((int)(S.at<double>(0)))%2)
					S.at<double>(0)-=1;
				//std::cout << S.at<double>(0) << std::endl;
			}
			//std::cout << S.at<float>(0, 0) << std::endl;
			Block = U*Mat::diag(S)*VT;
			gsl_wavelet2d_transform_inverse(w, data, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, work);
			//Block.convertTo(Block, img.type(), 1);
			//haar_2d_inverse(Block, LEVEL, img.type());
			//std::cout << Block << std::endl;
			Block.copyTo(img(roi));
			//std::cout << i << ":" << j << std::endl;
		}
	}
	gsl_wavelet_free(w);
	gsl_wavelet_workspace_free(work);
	img.convertTo(img, type);
}

std::string solve(Mat img)
{
	std::string res;
	char tmp = 0;

	w = gsl_wavelet_alloc(gsl_wavelet_daubechies, 4);
	work = gsl_wavelet_workspace_alloc(BLOCK_SIZE*BLOCK_SIZE);
	img.convertTo(img, CV_64F, 1.0);
	for(int i=0; i<img.rows/BLOCK_SIZE; i++)
	{
		for(int j=0;j<img.cols/BLOCK_SIZE; j++)
		{
			//Mat Block = Mat::zeros(BLOCK_SIZE, BLOCK_SIZE, CV_8UC3);
			//std::cout << Block << std::endl;
			Rect roi(i*BLOCK_SIZE, j*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
			Mat_<double> Block = img(roi);
			//img(roi).copyTo(Block);
			Mat_<double> S, U, VT;
			//haar_2d(Block, LEVEL, img.type());
			Block.convertTo(Block, CV_64F, 1.0);
			double *data = Block.ptr<double>();
			gsl_wavelet2d_transform_forward(w, data, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, work);
			SVDecomp(Block, S, U, VT, SVD::FULL_UV);
			//std::cout << (key[(i*BLOCK_SIZE+j)/8] & (0x80 >> ((i*BLOCK_SIZE+j)%8))) << std::endl;
			std::cout << S.at<double>(0,0) << std::endl;
			//std::cout << (int)S.at<float>(0,0)%2 << std::endl;
			if(((int)(S.at<double>(0)))%2)
			{
				tmp += (0x80 >> ((i*BLOCK_SIZE+j)%8));
				//std::cout << (0x80 >> ((i*BLOCK_SIZE+j)%8)) << std::endl;
			}
			if((i*BLOCK_SIZE+j)%8==7)
			{
				//std::cout << (int)tmp << std::endl;
				res+=tmp;
				tmp = 0;
			}
		}
	}
	gsl_wavelet_free(w);
	gsl_wavelet_workspace_free(work);
	return res;

}
