# Koenig Shell

**Koenig Shell** is a simple shell implementation written in C. It provides basic shell functionalities while aiming to be easy to understand and modify. This shell is based on the xv6 operating system framework.

## Features

- **Basic Shell Operations**: Supports executing commands, handling built-in commands, and managing processes.
- **Command Line Editing**: Provides basic command line editing functionalities using the readline library.
- **Signal Handling**: Implements signal handlers to handle SIGINT and SIGTSTP signals effectively.

## Getting Started

These instructions will help you get a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

You need to have the following installed on your system:
- C compiler (e.g., GCC)
- Readline library

### Installation

1. Clone the repository:

    ```bash
    git clone https://github.com/trevorkoenig/koenig-shell.git
    ```

2. Navigate to the cloned directory:

    ```bash
    cd koenig-shell
    ```

3. Compile the source code:

    ```bash
    make
    ```

    Alternatively, you can compile manually using:

    ```bash
    gcc -g main.c sh.c get_path.c -o mysh
    ```

4. Run the shell:

    ```bash
    ./mysh
    ```

## Usage

Once the shell is running, you can use it just like any other shell. It supports executing commands, handling built-in commands (e.g., `cd`, `exit`), and managing processes.

### Built-in Commands

- **exit**: Exit the shell.
- **which**: Display the full path of the command.
- **where**: Display all instances of the command in the PATH.
- **cd**: Change the current directory.
- **pwd**: Print the current working directory.
- **list**: List all files and directories in a given directory.
- **pid**: Print the process ID of the shell.
- **kill**: Terminate a process by its process ID.
- **prompt**: Set or change the shell prompt.
- **printenv**: Print all environment variables or a specific environment variable.
- **setenv**: Set or modify environment variables.
- **test**: A placeholder for testing purposes.

## Makefile

The project includes a Makefile for easy compilation. You can use the following commands with the Makefile:

- **make**: Compiles the source code and creates the executable `mysh`.
- **make clean**: Removes object files and the executable.


## Authors

- **Trevor Koenig** - [GitHub Profile](https://github.com/trevorkoenig)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- This shell is based on the Xv6 operating system framework.
- Completed as a project for CISC361.
