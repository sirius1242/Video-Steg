#include "steg.hpp"

using namespace cv;;

gsl_wavelet *w;
gsl_wavelet_workspace *work;

Mat steg(Mat img, std::string key)
{
	int type=img.type();

	w = gsl_wavelet_alloc(gsl_wavelet_daubechies, 4);
	work = gsl_wavelet_workspace_alloc(BLOCK_SIZE*BLOCK_SIZE);
	img.convertTo(img, CV_64F, 1.0);
	int bncol = img.cols/BLOCK_SIZE;
	int bnrow = img.rows/BLOCK_SIZE;
	for(int i=0; i<bnrow; i++)
	{
		for(int j=0;j<bncol; j++)
		{
			//Mat Block = Mat::zeros(BLOCK_SIZE, BLOCK_SIZE, CV_8UC3);
			Rect roi(i*BLOCK_SIZE, j*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
			Mat_<double> Block; //= img(roi);
			img(roi).copyTo(Block);
			Mat_<double> S, U, VT;
			//haar_2d(Block, LEVEL, img.type());
			//Block.convertTo(Block, CV_64F, 1.0);
			//std::cout << Block << std::endl;
			Block = Block.reshape(0,1);
			double *data = Block.ptr<double>();
			gsl_wavelet2d_transform_forward(w, data, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, work);
			Block = Block.reshape(0, BLOCK_SIZE);
			SVDecomp(Block, S, U, VT, SVD::FULL_UV);
			if(key.size()>(i*bncol+j)/8)
			{
				//std::cout << (key[(i*BLOCK_SIZE+j)/8] & (0x80 >> ((i*BLOCK_SIZE+j)%8))) << std::endl;
				//std::cout << S.at<double>(0) << std::endl;
				/*
				if (key[(i*BLOCK_SIZE+j)/8] & (0x80 >> ((i*BLOCK_SIZE+j)%8)))
				{
					//std::cout << (key[(i*BLOCK_SIZE+j)/8] & (0x80 >> ((i*BLOCK_SIZE+j)%8))) << std::endl;
					if(((int)(S.at<double>(0))>>SHIFT)%2==0)
						S.at<double>(0)+=1<<SHIFT;
				}
				else if (((int)(S.at<double>(0)))%2)
					S.at<double>(0)-=1<<SHIFT;
				*/
				int tmp = (int)S.at<double>(0);
				if (key[(i*bncol+j)/8] & (0x80 >> ((i*bncol+j)%8)))
					S.at<double>(0) = (floor(tmp/Q)+(int)floor(tmp/Q+1)%2)*Q;
				else
					S.at<double>(0) = (floor(tmp/Q)+(int)floor(tmp/Q)%2)*Q;
				//std::cout << S.at<double>(0) << ":" << ((key[(i*BLOCK_SIZE+j)/8] & (0x80 >> ((i*BLOCK_SIZE+j)%8)))?"1":"0") << std::endl;
				//std::cout << S.at<double>(0) << std::endl;
			}
			Block = U*Mat::diag(S)*VT;
			Block = Block.reshape(0,1);
			gsl_wavelet2d_transform_inverse(w, data, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, work);
			Block = Block.reshape(0, BLOCK_SIZE);
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
	return img;
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
			Rect roi(i*BLOCK_SIZE, j*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
			Mat_<double> Block;// = img(roi);
			//std::cout << Block << std::endl;
			img(roi).copyTo(Block);
			Mat_<double> S, U, VT;
			//haar_2d(Block, LEVEL, img.type());
			Block.convertTo(Block, CV_64F, 1.0);
			double *data = Block.ptr<double>();
			Block = Block.reshape(0,1);
			gsl_wavelet2d_transform_forward(w, data, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, work);
			Block = Block.reshape(0, BLOCK_SIZE);
			SVDecomp(Block, S, U, VT, SVD::FULL_UV);
			//std::cout << (key[(i*BLOCK_SIZE+j)/8] & (0x80 >> ((i*BLOCK_SIZE+j)%8))) << std::endl;
			//std::cout << (int)S.at<float>(0,0)%2 << std::endl;
			/*
			if(((int)(S.at<double>(0))>>SHIFT)%2)
			{
				tmp += (0x80 >> ((i*BLOCK_SIZE+j)%8));
				//std::cout << (0x80 >> ((i*BLOCK_SIZE+j)%8)) << std::endl;
			}
			*/
			double Si = S.at<double>(0);
			//std::cout << S.at<double>(0) << ":" << ((int)floor((Si/Q)+0.5)%2) << std::endl;
			if((int)floor((Si/Q)+0.5)%2)
				tmp += (0x80 >> ((i*BLOCK_SIZE+j)%8));
			if((i*BLOCK_SIZE+j)%8==7)
			{
				//std::cout << "tmp" << (int)tmp << std::endl;
				res+=tmp;
				tmp = 0;
			}
		}
	}
	gsl_wavelet_free(w);
	gsl_wavelet_workspace_free(work);
	return res;

}
