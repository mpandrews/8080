#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	FILE* file = fopen(argv[1], "r");
	fseek(file, 0x80, SEEK_SET);
	char* output = malloc(3072);
	int i	     = 0;
	output[i++]  = '{';
	while (fread(&output[i], 1, 1, file))
	{
		output[i++] += '0';
		output[i++] = ',';
		output[i++] = ' ';
	}
	output[i++] = '}';
	output[i]   = 0;
	printf("%s\n", output);
}
