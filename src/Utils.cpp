#include <Utils.h>

std::vector<std::string> Utils::split(const std::string &argument,
                                             const std::string &delimiter,
                                             int limit /*=-1*/)
{
    std::vector<std::string> strings;

    int counter = 1;
    auto start = 0U;
    auto end = argument.find(delimiter);
    while (end != std::string::npos)
    {
        if (counter == limit)
        {
            strings.push_back(argument.substr(start, argument.size() - 1));
            return strings;
        }
        strings.push_back(argument.substr(start, end - start));
        start = end + delimiter.length();
        end = argument.find(delimiter, start);
        counter++;
    }
    strings.push_back(argument.substr(start, end));
    return strings;
}

std::string Utils::getFileName(const std::string &filePath)
{
    std::string tissueName = filePath.substr(
        filePath.find_last_of('/') + 1, filePath.size() - 1);
    tissueName = tissueName.substr(0, tissueName.find_first_of('.'));
    return tissueName;
}

std::string Utils::fixFilePath(const std::string &filePath)
{
    std::string newPath = filePath;
    if (newPath.back() != '/')
    {
        newPath.push_back('/');
    }
    return newPath;
}

std::string Utils::trim(const std::string &s)
{
    return rtrim(ltrim(s));
}

std::string WHITESPACE = " \n\r\t\f\v";

std::string Utils::ltrim(const std::string &s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::string Utils::rtrim(const std::string &s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}