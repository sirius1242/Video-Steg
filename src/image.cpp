#include <iostream>
#include <fstream>

//#include <opencv2/core.hpp>
//#include <opencv2/imgcodecs.hpp>

#include "steg.hpp"

using namespace cv;

int main(int argc, char* argv[])
{
	/*
	if (argc < 3)
	{
		std::cerr << "Wrong command format!" << std::endl;
		return 1;
	}
	*/
	std::string image_path = samples::findFile(argv[1]);
	Mat img = imread(image_path, IMREAD_COLOR);

	if (img.empty())
	{
		std::cerr << "Could not read the image: " << image_path << std::endl;
		return 1;
	}

	Mat res;
	//res = img.clone();
	cvtColor(img, res, COLOR_RGB2GRAY);
	if(argc > 2)
	{
		std::ifstream fkey(argv[2]);
		std::string key((std::istreambuf_iterator<char>(fkey)),
						std::istreambuf_iterator<char>());
		//std::cout << hamming_encode(key) << std::endl;
		key = hamming_encode(key);
		res = steg(res, res.cols, key, key.size()+1); // make steg function write \0 after key message
		if(argc >= 3)
		{
			imwrite(argv[3], res);
			std::cout << "steged image saved to: " << argv[3] << std::endl;
		}
	}
	else
	{
		std::cerr << "decoding" << std::endl;
		std::string key = solve(res, res.cols);
		key = hamming_decode(key);
		std::cout << key << std::endl;
	}
	return 0;
}
