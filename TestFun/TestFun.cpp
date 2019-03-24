#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>

int main()
{
	std::ifstream file;
	file.open((std::string)"./RawText/" + (std::string)"600.txt.utf-8.txt");
	if (!file.is_open()) return 0;

	std::string phrase;
	while (file >> phrase)
	{
		// remove punctuation
		//auto isPunct = [](char c) { return std::ispunct(static_cast<unsigned char>(c)); }
		phrase.erase(std::remove_if(phrase.begin(), phrase.end(), [](unsigned char c) { return std::ispunct(c);} ), phrase.end());

		std::cout << phrase << '\n';
	}

	file.close();
}