#include "config.h"

int main()
{
	Config::Document doc;
	auto r = doc.load("test.txt");
	if (!r)
		return 0;

	doc.save("save.txt");

	printf("success\n");
	system("pause");
	return 0;
}