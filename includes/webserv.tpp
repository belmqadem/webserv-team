#include <string>

// Template Function implementation
template <class T>
std::string to_string(T t)
{
    std::stringstream str;
    str << t;
    return str.str();
}
