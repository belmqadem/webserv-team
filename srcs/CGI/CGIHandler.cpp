
#include "CGIHandler.hpp"
#include "webserv.hpp"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

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
        std::cerr << "CGI execution failed: " << strerror(errno) << std::endl;
        cleanupEnvArray(env);
        exit(EXIT_FAILURE);
    } else { // Parent process
        close(sockets[1]);

        // If POST, write the body to the socket
        if (method == "POST" && !body.empty()) {
            if (write(sockets[0], body.c_str(), body.size()) < 0) {
                close(sockets[0]);
                throw std::runtime_error("Failed to write to CGI: " + std::string(strerror(errno)));
            }
        }

        // Set non-blocking mode
        int flags = fcntl(sockets[0], F_GETFL, 0);
        fcntl(sockets[0], F_SETFL, flags | O_NONBLOCK);
        
        // Read the output with timeout
        std::ostringstream output;
        char buffer[1024];
        ssize_t bytesRead;
        
        fd_set readfds;
        struct timeval tv;
        int timeout_seconds = 30;
        time_t start_time = time(NULL);
        
        // Read with timeout
        while (true) {
            FD_ZERO(&readfds);
            FD_SET(sockets[0], &readfds);
            tv.tv_sec = 1;  // Check every second
            tv.tv_usec = 0;
            
            int ready = select(sockets[0] + 1, &readfds, NULL, NULL, &tv);
            
            if (ready == -1) {
                close(sockets[0]);
                throw std::runtime_error("Select error: " + std::string(strerror(errno)));
            } else if (ready == 0) {
                // Check total timeout
                if (time(NULL) - start_time > timeout_seconds) {
                    kill(pid, SIGTERM);
                    close(sockets[0]);
                    throw std::runtime_error("CGI execution timed out");
                }
                continue;
            }
            
            bytesRead = read(sockets[0], buffer, sizeof(buffer) - 1);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                output.write(buffer, bytesRead);
            } else if (bytesRead == 0) {
                break; // End of file
            } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                close(sockets[0]);
                throw std::runtime_error("Failed to read from CGI: " + std::string(strerror(errno)));
            }
        }
        
        close(sockets[0]);
        
        // Wait for child process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            throw std::runtime_error("CGI script exited with status " + to_string(WEXITSTATUS(status)));
        }
        
        std::string result = output.str();
        
        // Ensure there's an HTTP header
        if (result.find("HTTP/") == std::string::npos && 
            result.find("Status:") == std::string::npos) {
            // Add HTTP headers if they're missing
            return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + result;
        }
        
        return result;
    }
}

// The rest of your code (createEnvArray, cleanupEnvArray) remains the same
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
