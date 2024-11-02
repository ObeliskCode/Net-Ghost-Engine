#include "Run.h"

int main(int argc, char **argv)
{
	if (argc > 1)
	{
		std::cout << argv[1] << std::endl;
	}

	return run();
}