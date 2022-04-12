#include "ecc.hpp"

std::string hamming_encode(std::string src) // (12,8) hamming code
{
	std::string res;
	unsigned char tmp[3]={0,0,0}; // for write into result string;
	int base=0; // for judge write posision
	for(auto &c: src)
	{
		int x=0; //result for xor
		for(int i=7;i>=0;i--)
		{
			int p=tab[i];// poisition for current bit
			if(c&1<<i)
			{
				x ^= p;
				tmp[(p+base-1)/8]+=1<<(p+base-1)%8;
			}
		}
		//std::cout << x << std::endl;
		for(int j=0;j<4;j++)
		{
			int p=1<<j;
			if(bin[x][j])
				tmp[(p+base-1)/8]+=1<<(p+base-1)%8;// wrote the error checking bit
		}
		if(base)
		{
			for(int i=0;i<3;i++)
			{
				res+=tmp[i];
				tmp[i]=0;
			}
			base=0;
		}
		else
			base=12;
	}
	if(!base)
	{
		res+=tmp[0];
		res+=tmp[1];
	}
	return res;
}

std::string hamming_decode(std::string src)
{
	unsigned char tmp[3] = {0, 0, 0};
	std::string res;
	for(int i=0;i<src.size();i+=3)
	{
		tmp[0] = src[i];
		tmp[1] = src[i+1];
		if(src.size()-i<2)
			tmp[2] = 0;
		else
			tmp[2] = src[i+2];
		for(int j=0;j<2;j++)
		{
			int base=j*12;
			int x = 0; // xor result
			int p;
			int k;
			unsigned char cache=0; // char cache for result
			int b_err = 0; // the bit that error occured
			for(k=0;k<8;k++) // recover the message without checking error
			{
				p = tab[k];
				if(tmp[(p+base-1)/8] & 1<<(p+base-1)%8)
				{
					x ^= p;
					cache += 1<<k;
				}
			}
			for(k=0;k<4;k++) // compare error checking bit
			{
				p=1<<k;
				if(tmp[(p+base-1)/8]&1<<(p+base-1)%8)
				{
					if(!bin[x][k])
						b_err += p;
				}
				else if(bin[x][k])
					b_err += p;
			}
			if(b_err&&(k=err_tab[b_err-1])>=0) // if two or more error checking bit is wrong
			{
				if(b_err>12)
					std::cout << "error correct failed!" << std::endl;
				else
				{
					std::cout << "wrong bit position: " << b_err << ":" << k << std::endl;
					if(tmp[(b_err+base-1)/8] & 1<<(b_err+base-1)%8) // reverse the error bit
						cache -= 1<<k;
					else
						cache += 1<<k;
				}
			}
			res += cache;
			//std::cout << i << ":" << cache << std::endl;
		}
	}
	return res;
}
