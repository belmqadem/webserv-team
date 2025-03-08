#include "../../includes/RequestParser.hpp"
#include "../CGI/CGIHandler.hpp"
#include <iostream>
int main() {
        std::string raw_request = "GET /test.php?name=ChatGPT HTTP/1.1\r\n"
                              "Host: localhost\r\n"
                              "Content-Type: application/x-www-form-urlencoded\r\n"
                              "Content-Length: 11\r\n\r\n";
    RequestParser request(raw_request);
    
    std::string php_cgi_path = "/usr/bin/php-cgi";
    CGIHandler cgiHandler(request, php_cgi_path);
    
    std::string response = cgiHandler.executeCGI();
    
    std::cout << "CGI Response: " << std::endl << response << std::endl;

    return 0;
}










