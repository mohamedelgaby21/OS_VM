/*
 * Shell Assignment
 * Matches style of Task 1 - Task 6
 */

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>

using namespace std;

// Needed for the 'environ' command (Task 6 style)
extern char **environ;

int main(int argc, char *argv[]) {
    char input[1024];
    char cwd[PATH_MAX];
    char *args[64]; // Fixed size array, like Task 4's args[]
    
    // Support batch file mode (Requirement 3)
    if (argc == 2) {
        int fd = open(argv[1], O_RDONLY);
        if (fd < 0) {
            perror("Error opening batch file");
            return 1;
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    // Set initial PWD env var
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        setenv("PWD", cwd, 1);
    }

    while (true) {
        // 1. Prompt (Task 5 style getcwd)
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
             cout << cwd << " > "; 
        } else {
             cout << "? > ";
        }

        // 2. Read Input
        // If cin fails (Ctrl+D or End of File), break the loop
        if (!cin.getline(input, 1024)) {
            break; 
        }

        // 3. Parse Input (Tokenize spaces)
        int i = 0;
        char *token = strtok(input, " \t\n");
        while (token != NULL && i < 63) {
            args[i] = token;
            token = strtok(NULL, " \t\n");
            i++;
        }
        args[i] = NULL; // Null terminate the list, exactly like Task 4

        if (args[0] == NULL) continue; // Empty line

        // --- Check for Redirection & Background (&) ---
        // We scan the args array for special symbols
        int background = 0;
        char *file_in = NULL;
        char *file_out = NULL;
        int append = 0;

        int cmd_end = i; // Index where actual command args end

        for (int j = 0; j < i; j++) {
            if (strcmp(args[j], "&") == 0) {
                background = 1;
                args[j] = NULL; // Remove & from args passed to exec
            } else if (strcmp(args[j], "<") == 0) {
                if (j + 1 < i) file_in = args[j+1];
                args[j] = NULL; // Cut off args here
            } else if (strcmp(args[j], ">") == 0) {
                if (j + 1 < i) file_out = args[j+1];
                append = 0;
                args[j] = NULL;
            } else if (strcmp(args[j], ">>") == 0) {
                if (j + 1 < i) file_out = args[j+1];
                append = 1;
                args[j] = NULL;
            }
        }

        // --- Internal Commands ---

        // 'cd' command
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                 cout << cwd << endl;
            } else {
                if (chdir(args[1]) != 0) {
                    perror("cd error");
                } else {
                    getcwd(cwd, sizeof(cwd));
                    setenv("PWD", cwd, 1);
                }
            }
            continue;
        }
        
        // 'quit' command
        if (strcmp(args[0], "quit") == 0) {
            break;
        }

        // 'dir' command (Matches Task 5 logic)
        if (strcmp(args[0], "dir") == 0) {
            const char* path = ".";
            if (args[1] != NULL) path = args[1];

            DIR *dir = opendir(path);
            if (dir == NULL) {
                perror("opendir error");
            } else {
                struct dirent *entry;
                cout << "Directory contents: " << path << endl;
                while ((entry = readdir(dir)) != NULL) {
                    cout << entry->d_name << endl;
                }
                closedir(dir);
            }
            continue;
        }

        // 'environ' command (Matches Task 6 logic)
        if (strcmp(args[0], "environ") == 0) {
            char **env = environ;
            while (*env) {
                cout << *env << endl;
                env++;
            }
            continue;
        }

        // 'set' command (Matches Task 6 setenv)
        if (strcmp(args[0], "set") == 0) {
            if (args[1] != NULL && args[2] != NULL) {
                setenv(args[1], args[2], 1);
            }
            continue;
        }

        // 'echo' command
        if (strcmp(args[0], "echo") == 0) {
            for (int k = 1; args[k] != NULL; k++) {
                cout << args[k] << " ";
            }
            cout << endl;
            continue;
        }

        // 'help' command
        if (strcmp(args[0], "help") == 0) {
             cout << "Shell Manual:" << endl;
             cout << "cd, dir, environ, set, echo, help, pause, quit" << endl;
             cout << "Supports < > >> redirection and & background." << endl;
             continue;
        }

        // 'pause' command
        if (strcmp(args[0], "pause") == 0) {
            char dump[10];
            cin.getline(dump, 10); // Wait for enter
            continue;
        }

        // --- External Commands (Task 2 & 4 style) ---
        
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        }
        else if (pid == 0) {
            // Child Process
            
            // Handle IO Redirection inside child
            if (file_in) {
                int fd0 = open(file_in, O_RDONLY);
                dup2(fd0, STDIN_FILENO);
                close(fd0);
            }
            if (file_out) {
                int fd1;
                if (append) 
                    fd1 = open(file_out, O_WRONLY | O_CREAT | O_APPEND, 0644);
                else 
                    fd1 = open(file_out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                
                dup2(fd1, STDOUT_FILENO);
                close(fd1);
            }

            // Execute (Task 4 style)
            execvp(args[0], args);
            
            // If we get here, it failed
            perror("Command not found");
            exit(1);
        }
        else {
            // Parent Process
            if (background == 0) {
                wait(NULL); // Task 2/4 style wait
            }
        }
    }

    return 0;
}
