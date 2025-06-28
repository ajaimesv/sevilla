//
// Created by Andres Jaimes on 25/06/25.
//

#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <string>
#include <vector>

namespace sevilla {

    class csv_parser {

    private:
        std::vector<std::string> fields;

    public:
        /**
         * Parses a csv line. Fields may contain internal quotes.
         */
        size_t parse_line(const std::string& line, char separator);

        /**
         * Returns a field by index.
         */
        const std::string& operator[](size_t index) const;

        /**
         * Returns the number of fields found in the last parsed line.
         */
        size_t size() const;

    };

}

#endif //CSV_PARSER_H
