#pragma once

#include <sstream>
#include <string>

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
void base64_encode(std::stringstream& sstream, unsigned char const* bytes_to_encode, unsigned int in_len);
std::string base64_decode(std::string const& encoded_string);