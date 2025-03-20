
#include "CGIHandler.hpp"
#include <sstream>
#include <stdexcept>

// Constructor
CgiHandler::CgiHandler(const std::string &scriptPath, const std::map<std::string, std::string> &envVars)
    : _scriptPath(scriptPath), _envVars(envVars) {}

// Destructor
CgiHandler::~CgiHandler() {}

// Execute the CGI script and retrieve the output
std::string CgiHandler::execute(const std::string &method, const std::string &body) {
    int sockets[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1) {
        throw std::runtime_error("Failed to create socket pair: " + std::string(strerror(errno)));
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(sockets[0]);
        close(sockets[1]);
        throw std::runtime_error("Failed to fork process: " + std::string(strerror(errno)));
    }

    if (pid == 0) { // Child process
        close(sockets[0]);

        // Redirect stdin and stdout to the socket
        dup2(sockets[1], STDIN_FILENO);
        dup2(sockets[1], STDOUT_FILENO);
        close(sockets[1]);

        // Set up environment variables
        char **env = createEnvArray();

        // Execute the CGI script
        char *const argv[] = {const_cast<char *>(_scriptPath.c_str()), NULL};
        execve(_scriptPath.c_str(), argv, env);

        // If execve fails
        cleanupEnvArray(env);
        exit(EXIT_FAILURE);
    } else { // Parent process
        close(sockets[1]);

        // If POST, write the body to the socket
        if (method == "POST" && !body.empty()) {
            write(sockets[0], &body[0], body.size());
        }

        // Read the output from the CGI script
        std::ostringstream output;
        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(sockets[0], buffer, sizeof(buffer))) > 0) {
            output.write(buffer, bytesRead);
        }

        close(sockets[0]);
        return output.str();
    }
}

// Create environment variable array
char **CgiHandler::createEnvArray() const {
    char **envArray = new char*[_envVars.size() + 1];
    size_t i = 0;
    for (std::map<std::string, std::string>::const_iterator it = _envVars.begin(); it != _envVars.end(); ++it, ++i) {
        std::string envEntry = it->first + "=" + it->second;
        envArray[i] = strdup(envEntry.c_str());
    }
    envArray[i] = NULL;
    return envArray;
}

// Clean up environment array
void CgiHandler::cleanupEnvArray(char **envArray) const {
    for (size_t i = 0; envArray[i] != NULL; ++i) {
        free(envArray[i]);
    }
    delete[] envArray;
}
