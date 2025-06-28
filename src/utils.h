//
// Created by Andres Jaimes on 28/06/25.
//

#ifndef SLUGIFIER_H
#define SLUGIFIER_H

namespace sevilla {

    std::string slugify(const std::string& input);

    std::string ltrim(const std::string& s);
    std::string rtrim(const std::string& s);
    std::string trim(const std::string& s);

}

#endif //SLUGIFIER_H
