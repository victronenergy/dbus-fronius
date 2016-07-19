#include "app.h"

int main(int argc, char *argv[])
{
	App a(argc, argv);

	int r = a.parseOptions();
	if (r != 0)
		return r;

	return a.exec();
}
