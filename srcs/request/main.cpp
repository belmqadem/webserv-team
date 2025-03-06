#include "../../includes/RequestParser.hpp"
#include "../CGI/CGIHandler.hpp"

int main()
{
    std::string request = 
        "GET /hello?name=world HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Connection: keep-alive\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "Hello";
    RequestParser parser(request);
    CGIHandler cgi;
    cgi.executeCGI(parser);
    return 0;
}
