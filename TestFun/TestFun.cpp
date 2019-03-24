#include <iostream>
#include <string>
#include <fstream>

int main()
{
	std::ifstream file;
	file.open((std::string)"./RawText/" + (std::string)"600.txt.utf-8.txt");
	if (!file.is_open()) return 0;

	std::string word;
	while (file >> word)
	{
		std::cout << word << '\n';
	}

	file.close();
}