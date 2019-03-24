#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <map>
#include <iterator>

int main()
{
	// Store word count in map type
	std::map<std::string, int> WordCount;

	std::ifstream file;
	file.open((std::string)"./RawText/" + (std::string)"600.txt.utf-8.txt");
	if (!file.is_open()) return 0;

	std::string word;
	while (file >> word)
	{
		// remove punctuation
		//auto isPunct = [](char c) { return std::ispunct(static_cast<unsigned char>(c)); }
		word.erase(std::remove_if(word.begin(), word.end(), [](unsigned char c) { return std::ispunct(c);} ), word.end());

		// Count the number of each word locally
		++WordCount[word];

		std::cout << word << '\n';
	}

	// Print the word count from map type
	std::map<std::string, int>::iterator itr;
	std::cout << "\nThe map WordCount is : \n";
	std::cout << "\tWord\tCount\n";
	for (itr = WordCount.begin(); itr != WordCount.end(); ++itr) {
		std::cout << '\t' << itr->first
			<< '\t' << itr->second << '\n';
	}

	file.close();
}