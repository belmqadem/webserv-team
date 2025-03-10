# webserv-team Project

## Overview
The `webserv-team` project is a server application designed to handle client connections and manage I/O operations in an efficient manner. It utilizes an event-driven architecture to handle multiple clients simultaneously.

## Project Structure
The project is organized into the following directories and files:

```
webserv-team
├── includes
│   ├── ClientServer.hpp        // Defines the ClientServer class for managing client connections.
│   ├── ConfigManager.hpp       // Manages server configurations and settings.
│   ├── IEvenetListeners.hpp     // Interface for handling events from the I/O multiplexer.
│   ├── IOMultiplexer.hpp       // Manages multiple I/O operations and event-driven programming.
│   ├── Logger.hpp              // Provides logging functionality for messages and errors.
│   ├── Server.hpp              // Defines the Server class for managing server operations.
│   └── webserv.hpp             // Main header file for the webserv application.
├── src
│   ├── ClientServer.cpp        // Implementation of the ClientServer class.
│   ├── ConfigManager.cpp       // Implementation of the ConfigManager class.
│   ├── IOMultiplexer.cpp       // Implementation of the IOMultiplexer class.
│   ├── Logger.cpp              // Implementation of the Logger class.
│   └── Server.cpp              // Implementation of the Server class.
├── Makefile                    // Build instructions for compiling the project.
└── README.md                   // Documentation for the project.
```

## Setup Instructions
1. **Clone the Repository**: 
   ```
   git clone <repository-url>
   cd webserv-team
   ```

2. **Build the Project**: 
   Use the provided Makefile to compile the project.
   ```
   make
   ```

3. **Run the Server**: 
   After building, you can run the server executable. Ensure you have the necessary permissions and configurations set up.

## Usage
- The server listens for incoming client connections and handles them using the `ClientServer` class.
- Configuration settings can be managed through the `ConfigManager` class, allowing for easy adjustments to server behavior.
- Logging is handled by the `Logger` class, providing insights into server operations and errors.

## Contributing
Contributions are welcome! Please fork the repository and submit a pull request with your changes.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.