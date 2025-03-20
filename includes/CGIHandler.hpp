#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <string>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <vector>

class CgiHandler {
public:
    // Constructor
    CgiHandler(const std::string &scriptPath, const std::map<std::string, std::string> &envVars);

    // Destructor
    ~CgiHandler();

    // Execute the CGI script and retrieve the output
    std::string execute(const std::string &method, const std::string &body = "");

private:
    std::string _scriptPath;
    std::map<std::string, std::string> _envVars;

    // Set up environment variables for the CGI script
    char **createEnvArray() const;

    // Clean up dynamically allocated environment array
    void cleanupEnvArray(char **envArray) const;
};

#endif // CGI_HANDLER_HPP
