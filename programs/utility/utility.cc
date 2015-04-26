#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <vector>

using namespace std;

int main(int argc, char **argv)
{
	if(argc < 4)
	{
		cout<<"usage error"<<endl;
		return 1;
	}

	vector<unsigned int> va;  // address of PAL function
	vector<string> vf;        // name of PAL function
	
	unsigned int tmp_addr;
	string tmp_func;
	int c;

	ifstream fconf(argv[2]);  // configuration file	
	c = 0;
	while(fconf >> tmp_func)
	{
		vf.push_back(tmp_func);
		va.push_back(0xffffffff);
		c++;
	}
	fconf.close();

	// unsigned int is 32-bit
	ifstream fsym(argv[1]);
	while(fsym >> std::hex >> tmp_addr)
	{
		fsym >> tmp_func;
		for(int i=0; i<c; i++)
		{
			if(vf[i].compare(tmp_func) == 0)
			{
				va[i] = tmp_addr;
				break;
			}
		}
	}

	fsym.close();

	// append info to PAL
	unsigned int idx;
	FILE *fp = fopen(argv[3], "a");
	if(fp == NULL)
	{
		printf("Failed to open file\n");
		return 1;
	}

	for(idx=0; idx<c; idx++)
	{
		unsigned int tmp = va[idx];
		fwrite(&tmp, sizeof(unsigned int), 1, fp);
	}

	fclose(fp);

	return 0;
}
