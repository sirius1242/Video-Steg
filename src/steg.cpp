#include "steg.hpp"

using namespace cv;;

gsl_wavelet *w;
gsl_wavelet_workspace *work;

Mat steg(Mat img, std::string key, int keysize)
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
			Rect roi(j*BLOCK_SIZE, i*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
			Mat_<double> Block; //= img(roi);
			img(roi).copyTo(Block);
			Mat_<double> S, U, VT;
			Block = Block.reshape(0,1); // convert to row-major array for wavelet transform
			double *data = Block.ptr<double>();
			gsl_wavelet2d_transform_forward(w, data, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, work);
			Block = Block.reshape(0, BLOCK_SIZE); // convert back for SVDecomp
			SVDecomp(Block, S, U, VT, SVD::FULL_UV);
			if(keysize>(i*bncol+j)/8)
			{
				int tmp = (int)S.at<double>(0);
				if (key[(i*bncol+j)/8] & (0x80 >> ((i*bncol+j)%8))) // QIM for robust
					S.at<double>(0) = (floor(tmp/Q)+(int)floor(tmp/Q+1)%2)*Q;
				else
					S.at<double>(0) = (floor(tmp/Q)+(int)floor(tmp/Q)%2)*Q;
				//std::cout << S.at<double>(0) << std::endl;
			}
			Block = U*Mat::diag(S)*VT;
			Block = Block.reshape(0,1);
			gsl_wavelet2d_transform_inverse(w, data, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, work);
			Block = Block.reshape(0, BLOCK_SIZE);
			Block.copyTo(img(roi));
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
	int bncol = img.cols/BLOCK_SIZE;
	int bnrow = img.rows/BLOCK_SIZE;
	for(int i=0; i<bnrow; i++)
	{
		for(int j=0;j<bncol; j++)
		{
			Rect roi(j*BLOCK_SIZE, i*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
			Mat_<double> Block;// = img(roi);
			img(roi).copyTo(Block);
			Mat_<double> S, U, VT;
			Block.convertTo(Block, CV_64F, 1.0);
			double *data = Block.ptr<double>();
			Block = Block.reshape(0,1); // convert to row-major array for wavelet transform
			gsl_wavelet2d_transform_forward(w, data, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, work);
			Block = Block.reshape(0, BLOCK_SIZE);
			SVDecomp(Block, S, U, VT, SVD::FULL_UV);
			double Si = S.at<double>(0);
			//std::cout << S.at<double>(0) << ":" << ((int)floor((Si/Q)+0.5)%2) << std::endl;
			if((int)floor((Si/Q)+0.5)%2) // read every bit hidden in blocks
				tmp += (0x80 >> ((i*bncol+j)%8));
			if((i*bncol+j)%8==7)
			{
				res+=tmp;
				if(!tmp)
				{
					gsl_wavelet_free(w);
					gsl_wavelet_workspace_free(work);
					return res;
				}
				tmp = 0;
			}
		}
	}
	gsl_wavelet_free(w);
	gsl_wavelet_workspace_free(work);
	return res;

}
