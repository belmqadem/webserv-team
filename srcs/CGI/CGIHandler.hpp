#include "../../includes/webserv.hpp"

#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

class CGIHandler {
private:
    std::string scriptPath;
    char *envp[5];  // Static environment variables for now

public:
    CGIHandler(std::string path);
    void setupEnvironment();
    void execute();
};

#endif

