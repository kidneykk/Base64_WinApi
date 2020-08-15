#include "base64.h"

base64::base64() {
	// maps initialization
	int i = 0;
	for (char ch = 'A'; ch <= 'Z'; ++ch) { alph64[i] = ch; alph64_inv[ch] = i++; }
	for (char ch = 'a'; ch <= 'z'; ++ch) { alph64[i] = ch; alph64_inv[ch] = i++; }
	for (char ch = '0'; ch <= '9'; ++ch) { alph64[i] = ch; alph64_inv[ch] = i++; }
	alph64[i] = '+'; alph64_inv['+'] = i++;
	alph64[i] = '/'; alph64_inv['/'] = i;
}

void base64::setReadBlockSize(unsigned int size) {
	_blockSize = size;
}

std::string base64::encode(const std::string& input_str) {
	if (input_str.empty()) return "";

	using u8 = unsigned char;

	char ch = 0;	// current 6-bit char
	char rem = 0;	// 'remainder'

	std::string encoded;
	for (int i = 0; i < input_str.size(); ++i) {
		switch (i % 3) {
		case 0:
			if (i > 0) encoded += alph64[rem];
			ch = ((u8)input_str[i] >> 2);
			rem = ((u8)input_str[i] << 4) & 0x30;		// 00xx0000
			break;

		case 1:
			ch = ((u8)input_str[i] >> 4) | rem;
			rem = ((u8)input_str[i] << 2) & 0x3C;		// 00xxxx00
			break;

		case 2:
			ch = ((u8)input_str[i] >> 6) | rem;
			rem = (u8)input_str[i] & 0x3F;				// 00xxxxxx
			break;
		}
		encoded += alph64[ch];
	}
	encoded += alph64[rem];
		 if (input_str.size() % 3 == 1) encoded += "==";
	else if (input_str.size() % 3 == 2) encoded += '=';

	return encoded;
}

std::string base64::decode(const std::string& encoded) {
	if (encoded.empty()) return "";

	std::string decoded;
	int k = 0;
	char ch;	// current char
	char rem;	// 'remainder' - part of next char
	for (int i = 0; i < encoded.size() - 1; ++i) {
		switch (k % 3) {
		case 0:
			ch = alph64_inv[encoded[i]] << 2;
			rem = alph64_inv[encoded[i + 1]] >> 4;			// 0000 00xx
			break;

		case 1:
			ch = alph64_inv[encoded[i]] << 4;
			rem = alph64_inv[encoded[i + 1]] >> 2;			// 0000 xxxx
			break;

		case 2:
			ch = alph64_inv[encoded[i]] << 6;
			rem = alph64_inv[encoded[i + 1]];				// 00xx xxxx
			i++;
			break;
		}
		decoded += static_cast<char>(ch | rem);
		k++;
	}

	return decoded;
}

void base64::encode_file(const std::string& filename) {
	std::ifstream fin(filename.c_str(), std::ios::binary);
	if (!fin.is_open()) {
		std::cerr << "Error: could not open file \"" << filename << "\"\n";
		return;
	}

	// output filename routine
	std::string en_filename(filename);
	std::size_t extPos = en_filename.find('.');	// try to find file extension ('.')
	if (extPos != std::string::npos)			// if extension exists
		en_filename.insert(extPos, "_enc");		// insert "_enc" before '.'
	else										// otherwise
		en_filename.append("_enc");				// add "_enc" to the end

	std::ofstream fout(en_filename.c_str(), std::ios::binary);
	std::string _block(_blockSize, ' ');		// chunk of input data

	// encoding
	while (!fin.eof()) {
		fin.read(&_block[0], _blockSize);
		_block.resize(fin.gcount()); // bad? maybe use another structure, or create specific method . . .
		_block = encode(_block);	 // encode received block
		fout.write(_block.c_str(), _block.size());	// write encoded block to output file
	}

	fin.close();
	fout.close();
}

void base64::decode_file(const std::string& filename) {
	std::ifstream fin(filename.c_str());
	if (!fin.is_open()) {
		std::cerr << "Error: could not open file \"" << filename << "\"\n";
		return;
	}

	// output filename routine
	std::string dec_filename(filename);
	std::size_t extPos = dec_filename.find('.'); // try to find file extension ('.')
	if (extPos != std::string::npos)			 // if extension exists
		dec_filename.insert(extPos, "_dec");	 // insert "_decc" before '.'
	else										 // otherwise
		dec_filename.append("_dec");			 // add "_decc" to the end

	std::ofstream fout(dec_filename.c_str());
	
	// read data from input file to string
	std::string _block((std::istreambuf_iterator<char>(fin)),
						std::istreambuf_iterator<char>());

	// decoding
	_block = decode(_block);

	// write decoded string to output file
	fout.write(_block.c_str(), _block.size());

	fin.close();
	fout.close();
}