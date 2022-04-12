#include <iostream>
#include <queue>

//const int tab[8]={3,5,6,7,9,10,11,12};
const int tab[8]={12, 11, 10, 9, 7, 6, 5, 3};
const int bin[16][4]={{0,0,0,0},{0,0,0,1},{0,0,1,0},{0,0,1,1},
				   {0,1,0,0},{0,1,0,1},{0,1,1,0},{0,1,1,1},
				   {1,0,0,0},{1,0,0,1},{1,0,1,0},{1,0,1,1},
				   {1,1,0,0},{1,1,0,1},{1,1,1,0},{1,1,1,1}};
const int err_tab[12]={-1, -1, 0, -1, 1, 2, 3, -1, 4, 5, 6, 7};

std::string hamming_encode(std::string);
std::string hamming_decode(std::string);
