#include "../../includes/CGIHandler.hpp"

#define ROOT_DIRECTORY "www/html"
#define CGI_TIMEOUT 10
std::string CGIHandler::FindWhichScriptIs(std::string& dotS)
{
    if(dotS == ".py")
        return std::string("/bin/python3");
    else if(dotS == ".php")
        return std::string("/bin/php-cgi");
    else if(dotS == ".js")
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
    Logger &logger = Logger::getInstance();
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
        logger.error("No CGI interpreter found");
        throw std::runtime_error("500 Internal Server Error: No CGI interpreter found");
    }
    logger.info("CGIHandler initialized with script: " + scriptPath);
}


void CGIHandler::executeCGI() {

    int pid;
    int sv[2];

    Logger &logger = Logger::getInstance();
    if(socketpair(AF_UNIX , SOCK_STREAM ,0 , sv) ==  -1)
        logger.error("socket pair fails");
    if((pid = fork()) == -1)
    {
        close(sv[0]);
        dup2(sv[1] ,STDIN_FILENO );
        dup2(sv[1], STDOUT_FILENO);
        std::vector<std::string> args;
        args.push_back(interpreter);
        args.push_back(scriptPath);
        char *av[args.size()];
        for( size_t i = 0 ; i <args.size() ; ++i)
        {
            av[i] = const_cast<char *>(args[i].c_str());
        }
        av[args.size()] = NULL;
        std::vector<std::string>env;
        std::string as;
        std::stringstream ss;
        ss << body.length();
        env.push_back("REQUEST_METHOD=" + method);
        env.push_back("QUERY_STRING=" + queryString);
        env.push_back("CONTENT_LENGTH=" + ss.str());
        env.push_back("REDIRECT_STATUS=200");
        env.push_back("DOCUMENT_ROOT=" + std::string(ROOT_DIRECTORY));
        env.push_back("SCRIPT_NAME=" + scriptPath);
        env.push_back("CONTENT_TYPE=" + headers["Content-Type"]);
        env.push_back("SCRIPT_FILRNAME=" + scriptPath);
    
        char *envp[env.size() + 1];
        for( size_t i = 0 ; i <args.size() ; ++i)
        {
            av[i] = const_cast<char *>(env[i].c_str());
        }
        envp[env.size()] = NULL;
        execve(av[0], av, envp);
        logger.error("execve fail");
        exit(1);
    }

    close(sv[1]);

    struct pollfd fds[1];
    fds[0].fd = sv[0];
    fds[0].events = POLLIN;

    int ret = poll(fds, 1, -1);
    if (ret > 0) {
        if (fds[0].revents & POLLIN) {
            char buffer[4096];
            int n = read(sv[0], buffer, sizeof(buffer));
            if (n > 0) {
                std::cout << "CGI Response: " << std::string(buffer, n) << std::endl;
            }
            else if (n == 0) {
                std::cout << "No more data from CGI process" << std::endl;
            }
            else {
                std::cerr << "Error reading from child" << std::endl;
            }
        }
    }
    else if (ret == 0) {
        std::cerr << "Poll timeout reached" << std::endl;
    }
    else {
        std::cerr << "Poll failed" << std::endl;
    }

    close(sv[0]);
}


