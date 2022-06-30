#pragma once

#include <string>

namespace Utility {
    namespace String {

        // trim from left
        inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v")
        {
            s.erase(0, s.find_first_not_of(t));
            return s;
        }

        // trim from right
        inline std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v")
        {
            s.erase(s.find_last_not_of(t) + 1);
            return s;
        }

        // trim from left & right
        inline std::string& trim(std::string& s, const char* t = " \t\n\r\f\v")
        {
            return ltrim(rtrim(s, t), t);
        }

        // split into parts
        inline std::vector<std::string> split(std::string& s, const std::string& seperator)
        {
            std::vector<std::string> output;
            std::string::size_type prev_pos = 0, pos = 0;
            while ((pos = s.find(seperator, pos)) != std::string::npos)
            {
                std::string substring(s.substr(prev_pos, pos - prev_pos));
                output.push_back(substring);
                prev_pos = ++pos + seperator.size() - 1;
            }
            output.push_back(s.substr(prev_pos, pos - prev_pos)); // Last word

            return output;
        }

        inline std::vector<std::string> split(std::string s, const std::string& seperator)
        {
            std::vector<std::string> output;
            std::string::size_type prev_pos = 0, pos = 0;
            while ((pos = s.find(seperator, pos)) != std::string::npos)
            {
                std::string substring(s.substr(prev_pos, pos - prev_pos));
                output.push_back(substring);
                prev_pos = ++pos + seperator.size() - 1;
            }
            output.push_back(s.substr(prev_pos, pos - prev_pos)); // Last word

            return output;
        }

        // split first only
        inline std::vector<std::string> splitFirst(std::string& s, const std::string& seperator) {
            std::vector<std::string> output;
            uint64_t pos = s.find(seperator);
            if (pos != std::string::npos) {
                output.push_back(s.substr(0, pos));
                output.push_back(s.substr(pos + seperator.size()));
            }
            else {
                output.push_back(s);
            }
            return output;
        }

        inline bool isInt(const std::string& s)
        {
            return !s.empty() && std::find_if(s.begin(),
                s.end(), [](unsigned char c) { return !(std::isdigit(c) || c == '-'); }) == s.end();
        }

        inline bool isFloat(const std::string& s)
        {
            bool hasPoint = false;
            return !s.empty() && std::find_if(s.begin(),
                s.end(), [&hasPoint](unsigned char c) { if (c == '.') { hasPoint = true; } return !(std::isdigit(c) || c == '-' || c == '.'); }) == s.end() && hasPoint;
        }

        inline std::string& toUpper(std::string& s) {
            std::transform(s.begin(), s.end(), s.begin(),
                [](unsigned char c) { return std::toupper(c); });
            return s;
        }

        inline std::string& toLower(std::string& s) {
            std::transform(s.begin(), s.end(), s.begin(),
                [](unsigned char c) { return std::tolower(c); });
            return s;
        }
    }
}