#pragma once
#include <vector>
#include <string>

std::vector<std::string> fileRead(const std::string& fileName);
std::vector<std::string> split(const std::string& fullString, const std::string& delimiter);
bool endsWith(const std::string& fullString, const std::string& subString);