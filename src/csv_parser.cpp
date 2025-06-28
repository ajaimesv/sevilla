//
// Created by Andres Jaimes on 25/06/25.
//

#include "csv_parser.h"

namespace sevilla {

    size_t csv_parser::parse_line(const std::string &line, const char separator) {
        fields.clear();
        std::string field;
        bool in_quotes = false;

        for (size_t i = 0; i < line.size(); i++) {
            const char c = line[i];
            if (in_quotes) {
                if (c == '"') {
                    if (i + 1 < line.size() && line[i + 1] == '"') {
                        field += '"';
                        i++;
                    } else {
                        in_quotes = false;
                    }
                } else {
                    field += c;
                }
            } else {
                if (c == '"') {
                    in_quotes = true;
                } else if (c == separator) {
                    fields.push_back(field);
                    field.clear();
                } else {
                    field += c;
                }
            }
        }

        fields.push_back(field);
        return fields.size();
    }

    const std::string& csv_parser::operator[](const size_t index) const {
        if (index >= fields.size()) {
            throw std::out_of_range("Index is out of range");
        }
        return fields[index];
    }

    size_t csv_parser::size() const {
        return fields.size();
    }

    void csv_parser::reset() {
        fields.clear();
    }

}
