#include <string>
#include <vector>

class Utils
{
public:
    // from https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
    static inline std::vector<std::string> split(const std::string &argument,
                                                 const std::string &delimiter)
    {
        std::vector<std::string> strings;

        auto start = 0U;
        auto end = argument.find(delimiter);
        while (end != std::string::npos)
        {
            strings.push_back(argument.substr(start, end - start));
            start = end + delimiter.length();
            end = argument.find(delimiter, start);
        }
        strings.push_back(argument.substr(start, end));
        return strings;
    }
};
