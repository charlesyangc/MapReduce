#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <map>
#include <iterator>

// Relative frequencies of the first letters of a word in the English language
// source: https://en.wikipedia.org/wiki/Letter_frequency
//std::map<char, float> FreqFirstLetter = {
//				{ 'a', 0.11682 }, { 'b', 0.04434 }, { 'c', 0.05238 }, { 'd', 0.03174 }, { 'e', 0.02799 },
//				{ 'f', 0.04027 }, { 'g', 0.01642 }, { 'h', 0.04200 }, { 'i', 0.07294 }, { 'j', 0.00511 },
//				{ 'k', 0.00456 }, { 'l', 0.02415 }, { 'm', 0.03826 }, { 'n', 0.02284 }, { 'o', 0.07631 },
//				{ 'p', 0.04319 }, { 'q', 0.00222 }, { 'r', 0.02826 }, { 's', 0.06686 }, { 't', 0.15978 },
//				{ 'u', 0.01183 }, { 'v', 0.00824 }, { 'w', 0.05497 }, { 'x', 0.00045 }, { 'y', 0.00763 },
//				{ 'z', 0.00045 } };
std::map<char, float> FreqFirstLetter = {
				{ 'a', 0.11682 }, { 'b', 0.1612 }, { 'c', 0.2135 }, { 'd', 0.2453 }, { 'e', 0.2733 },
				{ 'f', 0.3135 }, { 'g', 0.3300 }, { 'h', 0.3720 }, { 'i', 0.4449 }, { 'j', 0.4500 },
				{ 'k', 0.4546 }, { 'l', 0.4787 }, { 'm', 0.5170 }, { 'n', 0.5398 }, { 'o', 0.6161 },
				{ 'p', 0.6593 }, { 'q', 0.6615 }, { 'r', 0.6898 }, { 's', 0.7567 }, { 't', 0.9164 },
				{ 'u', 0.9283 }, { 'v', 0.9365 }, { 'w', 0.9915 }, { 'x', 0.9919 }, { 'y', 0.9996 },
				{ 'z', 1.0000 } };

// Map word to reducer based on its first letter, return the index of reducer 0 : num_reducer-1
int HashMap(std::string& word, int num_reducer) {

	int AssignedReducer = 0;

	try {
		char FirstLetter = word.at(0);
		AssignedReducer = (int)( FreqFirstLetter.at(FirstLetter) * num_reducer );
		std::cout << word << ", first letter is: " << FirstLetter << " with freq = " << FreqFirstLetter.at(FirstLetter) << std::endl;
	}
	catch (...) { // assign other cases to the last reducer
		AssignedReducer = num_reducer - 1;
	}

	if (AssignedReducer == num_reducer)
		return (AssignedReducer - 1);
	else
		return AssignedReducer;
}

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
		// Convert upper case to lower case
		std::transform(word.begin(), word.end(), word.begin(), ::tolower);

		// Count the number of each word locally
		++WordCount[word];

		std::cout << word << ": to reducer " << HashMap(word, 8) << std::endl;
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