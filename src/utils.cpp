//
// Created by Andres Jaimes on 28/06/25.
//

#include <string>
#include "utils.h"

namespace sevilla {

    std::string slugify(const std::string& input) {
        std::string result;
        bool wasSeparator = true; // Used to avoid multiple consecutive hyphens

        for (char c : input) {
            if (std::isalnum(static_cast<unsigned char>(c))) {
                result += std::tolower(static_cast<unsigned char>(c));
                wasSeparator = false;
            } else if (!wasSeparator) {
                result += '-';
                wasSeparator = true;
            }
            // Else: ignore non-alnum characters
        }

        // Remove trailing hyphen if it exists
        if (!result.empty() && result.back() == '-') {
            result.pop_back();
        }

        return result;
    }


    std::string ltrim(const std::string& s) {
        auto start = std::find_if_not(s.begin(), s.end(), [](unsigned char ch) {
            return std::isspace(ch);
        });
        return std::string(start, s.end());
    }

    std::string rtrim(const std::string& s) {
        auto end = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char ch) {
            return std::isspace(ch);
        });
        return std::string(s.begin(), end.base());
    }

    std::string trim(const std::string& s) {
        return ltrim(rtrim(s));
    }

}
