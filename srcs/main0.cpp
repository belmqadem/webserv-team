#include "../includes/RequestParser.hpp"
#include "../includes/CGIHandler.hpp"
#include "../includes/RequestParser.hpp"
#include "../includes/CGIHandler.hpp"

#include <iostream>
#include "../includes/RequestParser.hpp"
#include "../includes/CGIHandler.hpp"
#include "../includes/ResponseBuilder.hpp"

// void handle_request(const std::string &raw_request) {
//     try {
//         RequestParser request_parser(raw_request);
//         CGIHandler cgi_handler(request_parser);
//         cgi_handler.executeCGI();
//         std::string cgi_output = cgi_handler.getOut();

//         ResponseBuilder response_builder(request_parser);
//         response_builder.set_body(cgi_output);

//         std::cout << "======== response builder start ========\n"
//                   << response_builder.get_response()
//                   << "\n======== response builder start ends ========\n"
//                   << std::endl;
//     } catch (const std::exception &e) {
//         std::cerr << "Error during CGI execution: " << e.what() << std::endl;
//     }
// }
// int main() {
//     std::string raw_request[] = {"GET /a.php?name=farid HTTP/1.1\r\n"
//                               "Host: localhost\r\n"
//                               "Content-Type: application/x-www-form-urlencoded\r\n"
//                               "Content-Length: 11\r\n\r\n",
//                                 "GET /a.py?name=hamid HTTP/1.1\r\n"
//                               "Host: localhost\r\n"
//                               "Content-Type: application/x-www-form-urlencoded\r\n"
//                               "Content-Length: 11\r\n\r\n" , "GET /a.php?name=ayman HTTP/1.1\r\n"
//                               "Host: localhost\r\n"
//                               "Content-Type: application/x-www-form-urlencoded\r\n"
//                               "Content-Length: 11\r\n\r\n","GET /a.php?name=anas HTTP/1.1\r\n"
//                               "Host: localhost\r\n"
//                               "Content-Type: application/x-www-form-urlencoded\r\n"
//                               "Content-Length: 11\r\n\r\n","GET /a.py?name=abas HTTP/1.1\r\n"
//                               "Host: localhost\r\n"
//                               "Content-Type: application/x-www-form-urlencoded\r\n"
//                               "Content-Length: 11\r\n\r\n",};

//     const int num_requests = 5; // Number of concurrent requests
//     std::vector<std::thread> threads;

//     for (int i = 0; i < num_requests; ++i) {
//         threads.emplace_back(handle_request, raw_request[i]);
//     }

//     for (auto &thread : threads) {
//         thread.join();
//     }

//     return 0;
// }
int main() {
    // std::string server_host = "localhost";
    // int server_port = 8080;
    // std::string php_cgi_path = "/bin/python3";
    std::string raw_request = "GET /a.py?name=hamid HTTP/1.1\r\n"
                              "Host: localhost\r\n"
                              "Content-Type: application/x-www-form-urlencoded\r\n"
                              "Content-Length: 2\r\n\r\n"
                              "name=hamid";
        
    RequestParser request_parser(raw_request);
	// request_parser.print_request();
    // if (request_parser.get_error_code() != 200) 
    // {
    //     std::cerr << "Error parsing the request!" << std::endl;
    //     return 1;
    // }
    try 
    {
        // CGIHandler cgi_handler(request_parser, php_cgi_path);
        // std::string cgi_output = cgi_handler.executeCGI();
        // std::cout <<"======== CGI output start ========\n" << cgi_output <<"\n======== CGI output start ends ========\n" <<std::endl;

        ResponseBuilder response_builder(request_parser);
        // response_builder.set_http_version("HTTP/1.1");
        // response_builder.set_status(200);
        // response_builder.set_headers("Content-Type", "text/html");
        // response_builder.set_body(cgi_output);

        std::cout <<"======== response builder start ========\n" << response_builder.get_response() <<"\n======== response builder start ends ========\n" <<std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Error during CGI execution: " << e.what() << std::endl;
    }
    return 0;
}


