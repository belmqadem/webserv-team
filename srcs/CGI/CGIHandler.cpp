#include "../../includes/CGIHandler.hpp"

#define ROOT_DIRECTORY "www/html"
#define CGI_TIMEOUT 1
std::string CGIHandler::FindWhichScriptIs(std::string& ex)
{
    if(ex == ".py")
        return std::string("/bin/python3");
    else if(ex == ".php")
        return std::string("/usr/bin/php-cgi");
    else if(ex == ".js")
        return std::string("/bin/js");
    else
        return "";
};
void timeout_handler(int signum) {
    (void)signum;
    std::cerr << "CGI script timed out" << std::endl;
    exit(1);
}


CGIHandler::CGIHandler(RequestParser &request)
{
    // Logger &logger = Logger::getInstance();
    scriptPath = ROOT_DIRECTORY + request.get_request_uri();
    method = request.get_http_method();
    queryString = request.get_query_string();
    body = request.get_body();
    headers = request.get_headers();

    size_t dotPos = scriptPath.find_last_of('.');
    if (dotPos != std::string::npos)
    {
        std::string extension = scriptPath.substr(dotPos);
        if (extension == ".php" || extension == ".py" || extension == ".js")
        {
            interpreter = FindWhichScriptIs(extension);
        }
    }
    if (interpreter.empty())
    {
        // logger.error("No CGI interpreter found");
        throw std::runtime_error("500 Internal Server Error: No CGI interpreter found");
    }
    // logger.info("CGIHandler initialized with script: " + scriptPath);
}
CGIHandler::~CGIHandler()
{
}
void CGIHandler::executeCGI() {
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        throw std::runtime_error("500 Internal Server Error: Socket pair creation failed");
    }

    pid_t pid = fork();
    if (pid == -1) {
        throw std::runtime_error("500 Internal Server Error: Fork failed");
    }

    if (pid == 0) {
        // Child process: Execute CGI
        close(sv[0]);
        if (method == "POST") {
            dup2(sv[1], STDIN_FILENO); // Attach input to CGI
        }
        dup2(sv[1], STDOUT_FILENO); // Attach output to CGI
        close(sv[1]);

        // Prepare arguments
        std::vector<std::string> args ;
        args.push_back(interpreter);
        args.push_back(scriptPath );
        char *argv[args.size() + 1];
        for (size_t i = 0; i < args.size(); ++i)
            argv[i] = const_cast<char *>(args[i].c_str());
        argv[args.size()] = NULL;

        // Prepare environment variables
        std::vector<std::string> env;
        env.push_back("REQUEST_METHOD=" + method);
        env.push_back("QUERY_STRING=" + queryString);
        env.push_back("SCRIPT_FILENAME=" + scriptPath);
        env.push_back("SCRIPT_NAME=" + scriptPath);
        env.push_back("DOCUMENT_ROOT=" + std::string(ROOT_DIRECTORY));
        env.push_back("PHP_SELF=" + scriptPath);
        env.push_back("PATH_TRANSLATED=" + scriptPath);
        env.push_back("REDIRECT_STATUS=200");

        if (method == "POST") {
            std::ostringstream ss;
            ss << body.size();
            env.push_back("CONTENT_LENGTH=" + ss.str());
            env.push_back("CONTENT_TYPE=" + headers["Content-Type"]);
        }

        char *envp[env.size() + 1];
        for (size_t i = 0; i < env.size(); ++i)
            envp[i] = const_cast<char *>(env[i].c_str());
        envp[env.size()] = NULL;

        signal(SIGALRM, timeout_handler);
        alarm(CGI_TIMEOUT);

        execve(argv[0], argv, envp);
        exit(1);
    } else {
        // Parent process: Send POST data (if needed) and read response
        close(sv[0]);

        if (method == "POST" && !body.empty()) {
            ssize_t written = write(sv[1], body.data(), body.size());
            if (written == -1) {
                throw std::runtime_error("500 Internal Server Error: Failed to write request body to CGI script");
            }
        }
        close(sv[1]);  // Close after writing

        // Read CGI output
        struct pollfd fds[1];
        fds[0].fd = sv[0];
        fds[0].events = POLLIN;
        cgiOutput.clear();
        char buffer[4096];
        int timeout_ms = CGI_TIMEOUT * 1000;
        int ret = poll(fds, 1, timeout_ms);

        if (ret > 0) {
            while (true) {
                int n = recv(sv[0], buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
                if (n > 0) {
                    buffer[n] = '\0';
                    cgiOutput += std::string(buffer, n);
                } else if (n == 0) {
                    break;
                } else {
                    break;
                }
            }
        } else if (ret == 0) {
            kill(pid, SIGKILL);
            waitpid(pid, NULL, 0);
        }
        close(sv[0]);
        waitpid(pid, NULL, 0);
    }
}

 const std::string CGIHandler::getOut() const 
 {
    return cgiOutput;
 }

