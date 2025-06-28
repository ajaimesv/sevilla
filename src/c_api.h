//
// Created by Andres Jaimes on 25/06/25.
//

#ifndef C_API_H
#define C_API_H

#ifdef _WIN32
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT
#endif

std::string make_error(const std::string& msg);

#endif //C_API_H
