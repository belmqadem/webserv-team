#include "../../includes/webserv.hpp"

#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

class CGIHandler {
private:
    std::string scriptPath;
    char *envp[5];
    std::string path;

public:
    CGIHandler();
    CGIHandler(std::string path);
    CGIHandler(std::string pathS, std::string pathE);
    
    void setupEnvironment();
    char **setupExecveArgs(const std::string& path);
    void execute();
};

#endif

