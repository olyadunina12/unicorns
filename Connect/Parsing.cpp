#include "Parsing.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <string>

bool isDelimiter(char c, const std::string& delimiter)
{
    return delimiter.find(c) != delimiter.npos;
}

std::vector<std::string> fileRead(const std::string& fileName)
{
    std::vector<std::string> text;
    std::string word;
    std::fstream newDoc;
    newDoc.open(fileName, std::ios::in);
    if (newDoc.is_open())
    {
        while (std::getline(newDoc, word))
        {
            text.push_back(word);
        }
    }
    return text;
}

std::vector<std::string> split(const std::string& fullString, const std::string& delimiter)
{
    std::vector<std::string> text;
    std::string word;
    for (int i = 0; i < fullString.size(); i++)
    {
        if (isDelimiter(fullString[i], delimiter))
        {
            if (!word.empty())
            {
                text.push_back(word);
                word = "";
            }
        }
        else
        {
            word.push_back(fullString[i]);
        }
    }
    if (!word.empty())
        text.push_back(word);
    return text;
}

bool endsWith(const std::string& fullString, const std::string& subString)
{
    if (subString.size() > fullString.size())
        return false;

    int i = 0;
    for (size_t j = fullString.size() - subString.size(); j < fullString.size(); j++)
    {
        if (subString[i] != fullString[j])
        {
            return false;
        }
        i++;
    }
    return true;
}