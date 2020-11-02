#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	FILE* file   = fopen(argv[1], "r");
	char* output = malloc(131072);
	output[0]    = '{';
	output[1]    = 0;
	unsigned char buf;
	char* next = output + 1;
	while (fread(&buf, 1, 1, file))
	{ next += sprintf(next, "0x%2.2ux, ", buf); }
	*(next - 1) = 0;
	*(next - 2) = '}';
	printf("%s\n", output);
}
