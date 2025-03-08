#include "../../includes/RequestParser.hpp"
#include "../CGI/CGIHandler.hpp"
#include <iostream>
// #include "CGIHandler.hpp"
int main() {
        std::string raw_request = "POST ./a.php HTTP/1.1\r\n"
                              "Host: localhost\r\n"
                              "Content-Type: application/x-www-form-urlencoded\r\n"
                              "Content-Length: 11\r\n\r\n"
                              "name=HelloWorld";
    // Assuming RequestParser is already initialized with the request details
    RequestParser request(raw_request);
    
    // Create a CGIHandler with a static PHP interpreter path
    std::string php_cgi_path = "/usr/bin/php-cgi";
    CGIHandler cgiHandler(request, php_cgi_path);
    
    // Execute the CGI script and get the result
    std::string response = cgiHandler.executeCGI();
    
    // Print the response
    std::cout << "CGI Response: " << std::endl << response <<"=================="<< std::endl;

    return 0;
}

// int main() {
//     // Step 1: Static configuration setup
//     std::string server_host = "localhost";
//     int server_port = 8080;
//     std::string php_cgi_path = "/usr/bin/php-cgi";  // CGI handler for PHP

//     // Step 2: Simulate an HTTP request (POST for example)
//     std::string raw_request = "POST /test.php?name=ChatGPT HTTP/1.1\r\n"
//                               "Host: localhost\r\n"
//                               "Content-Type: application/x-www-form-urlencoded\r\n"
//                               "Content-Length: 11\r\n\r\n"
//                               "name=HelloWorld";

//     // Step 3: Parse the incoming request
//     RequestParser request_parser(raw_request);

//     // If there was an error while parsing the request
//     if (request_parser.get_error_code() != 200) {
//         std::cerr << "Error parsing the request!" << std::endl;
//         return 1;
//     }

//     // Print the parsed request details
//     request_parser.print_request();

//     // Step 4: Handle the CGI request
//     try {
//         // Here we pass the CGI path directly, instead of using a configuration class
//         CGIHandler cgi_handler(request_parser, php_cgi_path);  // Pass the CGI handler path
//         std::string cgi_output = cgi_handler.executeCGI();
//         std::cout << cgi_output << "jkj=============== Response"<< std::endl;
//         // Step 5: Build a response
//         ResponseBuilder response_builder;
//         response_builder.set_http_version("HTTP/1.1");
//         response_builder.set_status(200, "OK");
//         response_builder.set_headers("Content-Type", "text/html");
//         response_builder.set_body(cgi_output);  // Set CGI output as response body

//         // Step 6: Send the response
//         std::string response = response_builder.build_response(request_parser);
//         std::cout << "Response:\n" << response << std::endl;
//     } catch (const std::exception &e) {
//         std::cerr << "Error during CGI execution: " << e.what() << std::endl;
//     }

//     return 0;
// }


// int main()
// {
//     std::string request = 
//         "GET /hello?name=world HTTP/1.1\r\n"
//         "Host: example.com\r\n"
//         "Connection: keep-alive\r\n"
//         "Content-Type: text/plain\r\n"
//         "Content-Length: 5\r\n"
//         "\r\n"
//         "Hello";
//     RequestParser parser(request);
//     CGIHandler cgi;
//     cgi.executeCGI(parser);
//     return 0;
// }
