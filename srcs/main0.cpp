#include "../includes/RequestParser.hpp"
#include "../includes/CGIHandler.hpp"
#include "../includes/RequestParser.hpp"
#include "../includes/CGIHandler.hpp"

int main() {
    // std::string server_host = "localhost";
    // int server_port = 8080;
    std::string php_cgi_path = "/usr/bin/php-cgi";
    std::string raw_request = "POST /a.php HTTP/1.1\r\n"
                              "Host: localhost\r\n"
                              "Content-Type: application/x-www-form-urlencoded\r\n"
                              "Content-Length: 11\r\n\r\n"
                              "name=HelloWorld";
    RequestParser request_parser(raw_request);
    // if (request_parser.get_error_code() != 200) 
    // {
    //     std::cerr << "Error parsing the request!" << std::endl;
    //     return 1;
    // }
    try 
    {
        CGIHandler cgi_handler(request_parser, php_cgi_path);
        std::string cgi_output = cgi_handler.executeCGI();
        std::cout <<"======== CGI output start ========\n" << cgi_output <<"\n======== CGI output start ends ========\n" <<std::endl;

        ResponseBuilder response_builder;
        response_builder.set_http_version("HTTP/1.1");
        response_builder.set_status(200, "OK");
        response_builder.set_headers("Content-Type", "text/html");
        response_builder.set_body(cgi_output);

        std::string response = response_builder.build_response(request_parser);
        std::cout <<"======== response builder start ========\n" << response <<"\n======== response builder start ends ========\n" <<std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Error during CGI execution: " << e.what() << std::endl;
    }
    return 0;
}


