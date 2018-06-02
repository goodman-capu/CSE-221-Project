#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <math.h>
using namespace std;
static string base_dir = "/Users/zhanyuan/Documents/GitHub/CSE221_Project/Project/";
int main()
{
	for (int i = 2; i <= 512; i *= 2)
	{
		cout << i <<endl;
		size_t file_size = pow(2, 20 * i);
		string file_name = base_dir + "File_" + to_string(i) + "MB";

	    FILE *fptr = fopen(file_name.data(), "w");
		char *content = (char *)malloc(file_size);
		memset(content, 0, file_size);
		fwrite(content, sizeof(char), file_size, fptr);
		free(content);
		fclose(fptr);
		cout << file_name << " written" << endl;
	}
	
}
        