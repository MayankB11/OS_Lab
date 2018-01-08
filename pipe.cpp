#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
int main(int argc, char const *argv[])
{
	/* code */
	int pip[2];
	int result;
	result = pipe(pip);	
	if (result == -1)
	{
		std::perror("pipe");
		exit(1);
	}
	return 0;
}