#pragma once

#include <stdexcept>
#include <string>

namespace protobuf::cebus
{
    struct GenerationException : public std::runtime_error
    {
        GenerationException(std::string what)
            : std::runtime_error(what)
        { }

        template<typename... Args>
        static GenerationException throwF(const char* format, Args&& ...args)
        {
            char buf[255];
            std::snprintf(buf, sizeof(buf), format, std::forward<Args>(args)...);
            throw GenerationException(buf);
        }
    };
}
