#include "../../includes/RequestParser.hpp"
#include "../CGI/CGIHandler.hpp"

int main()
{
    // Sample HTTP request
    std::string request = 
        "GET /a.py?name=world HTTP/1.1\r\n"
        "Host: a.py\r\n"
        "Connection: keep-alive\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "Hello";
    // Create a RequestParser object
    RequestParser parser(request);
    CGIHandler cgi;
    cgi.executeCGI(parser);
    // Print the parsed request
    parser.print_request();
    return 0;
}
