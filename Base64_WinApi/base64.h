#ifndef BASE64_H_
#define BASE64_H_

#include <unordered_map>
#include <string>
#include <fstream>
#include <iostream>

class base64 {
private:
	std::unordered_map<int, char> alph64;
	std::unordered_map<char, int> alph64_inv;

	unsigned int _blockSize{ 64U };

public:
	explicit base64();

	void setReadBlockSize(unsigned int size);

	std::string encode(const std::string& input_str);	// return encoded string
	std::string decode(const std::string& input_str);	// return decoded string

	void encode_file(const std::string& filename);
	void decode_file(const std::string& filename);
};

#endif