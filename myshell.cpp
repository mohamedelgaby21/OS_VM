#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <cstdlib>
#include <cstdio>

using namespace std;

extern char **environ;

int main(int argc, char *argv[]) {

    // Batch mode file
    FILE *batch = nullptr;
    if (argc == 2) {
        batch = fopen(argv[1], "r");
        if (!batch) {
            perror("batch file open");
            return 1;
        }
    } else if (argc > 2) {
        cerr << "Usage: " << argv[0] << " [batchfile]\n";
        return 1;
    }

    // Ensure PATH and PWD exist (as assignment says by default)
    if (getenv("PATH") == nullptr) setenv("PATH", "/usr/bin:/bin", 1);
    char cwd_init[PATH_MAX];
    if (getcwd(cwd_init, sizeof(cwd_init)) != nullptr) setenv("PWD", cwd_init, 1);

    while (true) {

        // 1) Print prompt (must contain current directory)
        if (!batch) {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != nullptr) {
                cout << cwd << " > ";
            } else {
                cout << "myshell > ";
            }
            cout.flush();
        }

        // 2) Read input line
        string line;
        if (!batch) {
            if (!getline(cin, line)) break;
        } else {
            char buf[4096];
            if (!fgets(buf, sizeof(buf), batch)) break;
            line = buf;
        }

        // remove trailing newline
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) line.pop_back();

        // ignore empty line
        {
            bool allSpace = true;
            for (char c : line) if (c != ' ' && c != '\t') { allSpace = false; break; }
            if (allSpace) continue;
        }

        // 3) Tokenize by whitespace (TA style)
        vector<string> tokens;
        {
            istringstream iss(line);
            string t;
            while (iss >> t) tokens.push_back(t);
        }
        if (tokens.empty()) continue;

        // 4) Background execution if last token is &
        bool background = false;
        if (!tokens.empty() && tokens.back() == "&") {
            background = true;
            tokens.pop_back();
            if (tokens.empty()) continue;
        }

        // 5) Parse redirection tokens: <, >, >>
        string inFile = "";
        string outFile = "";
        bool append = false;

        vector<string> argsOnly; // command + args (without redirection tokens)
        for (size_t i = 0; i < tokens.size(); i++) {
            if (tokens[i] == "<") {
                if (i + 1 >= tokens.size()) { cerr << "Syntax error: missing file after <\n"; argsOnly.clear(); break; }
                inFile = tokens[i + 1];
                i++;
            } else if (tokens[i] == ">") {
                if (i + 1 >= tokens.size()) { cerr << "Syntax error: missing file after >\n"; argsOnly.clear(); break; }
                outFile = tokens[i + 1];
                append = false;
                i++;
            } else if (tokens[i] == ">>") {
                if (i + 1 >= tokens.size()) { cerr << "Syntax error: missing file after >>\n"; argsOnly.clear(); break; }
                outFile = tokens[i + 1];
                append = true;
                i++;
            } else {
                argsOnly.push_back(tokens[i]);
            }
        }
        if (argsOnly.empty()) continue;

        string cmd = argsOnly[0];

        // =========================
        // BUILT-IN COMMANDS (internal)
        // =========================

        // quit
        if (cmd == "quit") {
            break;
        }

        // cd [DIRECTORY]
        if (cmd == "cd") {
            if (argsOnly.size() == 1) {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != nullptr) cout << cwd << "\n";
                else perror("getcwd");
            } else {
                if (chdir(argsOnly[1].c_str()) < 0) {
                    perror("chdir");
                } else {
                    char cwd[PATH_MAX];
                    if (getcwd(cwd, sizeof(cwd)) != nullptr) setenv("PWD", cwd, 1);
                }
            }
            continue;
        }

        // For built-ins that write output: support > and >>
        // We'll do TA-style stdout redirection using dup/dup2
        int savedStdout = -1;
        int outFD = -1;

        auto redirect_stdout_if_needed = [&]() -> bool {
            if (outFile.empty()) return true;
            int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
            outFD = open(outFile.c_str(), flags, 0644);
            if (outFD < 0) { perror("open output"); return false; }
            savedStdout = dup(STDOUT_FILENO);
            if (savedStdout < 0) { perror("dup"); close(outFD); outFD = -1; return false; }
            if (dup2(outFD, STDOUT_FILENO) < 0) { perror("dup2"); close(outFD); close(savedStdout); outFD=-1; savedStdout=-1; return false; }
            close(outFD);
            outFD = -1;
            return true;
        };

        auto restore_stdout_if_needed = [&]() {
            if (savedStdout != -1) {
                dup2(savedStdout, STDOUT_FILENO);
                close(savedStdout);
                savedStdout = -1;
            }
        };

        // dir [DIRECTORY]
        if (cmd == "dir") {
            if (!redirect_stdout_if_needed()) { restore_stdout_if_needed(); continue; }

            string dirPath = (argsOnly.size() >= 2) ? argsOnly[1] : ".";
            DIR *d = opendir(dirPath.c_str());
            if (!d) {
                perror("opendir");
            } else {
                struct dirent *entry;
                while ((entry = readdir(d)) != nullptr) {
                    cout << entry->d_name << "\n";
                }
                closedir(d);
            }

            restore_stdout_if_needed();
            continue;
        }

        // environ
        if (cmd == "environ") {
            if (!redirect_stdout_if_needed()) { restore_stdout_if_needed(); continue; }

            for (char **env = environ; *env != nullptr; ++env) {
                cout << *env << "\n";
            }

            restore_stdout_if_needed();
            continue;
        }

        // set VARIABLE VALUE
        if (cmd == "set") {
            if (argsOnly.size() < 3) {
                cerr << "set: usage: set VARIABLE VALUE\n";
            } else {
                string var = argsOnly[1];
                string value = argsOnly[2];
                for (size_t i = 3; i < argsOnly.size(); i++) value += " " + argsOnly[i];
                if (setenv(var.c_str(), value.c_str(), 1) < 0) perror("setenv");
            }
            continue;
        }

        // echo [COMMENT]
        if (cmd == "echo") {
            if (!redirect_stdout_if_needed()) { restore_stdout_if_needed(); continue; }

            string text = "";
            if (argsOnly.size() >= 2) {
                text = argsOnly[1];
                for (size_t i = 2; i < argsOnly.size(); i++) text += " " + argsOnly[i];
            }
            cout << text << "\n";

            restore_stdout_if_needed();
            continue;
        }

        // pause
        if (cmd == "pause") {
            cout << "Press Enter to continue...";
            cout.flush();
            string dummy;
            getline(cin, dummy);
            continue;
        }

        // help (must use more filter)
        if (cmd == "help") {

            // If redirected, just print text to redirected stdout
            if (!outFile.empty()) {
                if (!redirect_stdout_if_needed()) { restore_stdout_if_needed(); continue; }
                cout <<
                    "myshell - simple shell\n\n"
                    "Built-in commands:\n"
                    "  cd [DIRECTORY]      Change directory; if no DIRECTORY, print current directory.\n"
                    "  dir [DIRECTORY]     List contents of directory.\n"
                    "  environ             List environment variables.\n"
                    "  set VAR VALUE       Set environment variable.\n"
                    "  echo [TEXT]         Print text.\n"
                    "  help                Show this manual using more.\n"
                    "  pause               Wait until Enter is pressed.\n"
                    "  quit                Exit shell.\n\n"
                    "External commands:\n"
                    "  Anything else runs using fork + execvp.\n\n"
                    "Redirection:\n"
                    "  < infile, > outfile (truncate), >> outfile (append)\n"
                    "Background:\n"
                    "  Add & at end.\n";
                restore_stdout_if_needed();
                continue;
            }

            // Not redirected: pipe to more (TA-style with fork/exec)
            int pipefd[2];
            if (pipe(pipefd) < 0) {
                perror("pipe");
                continue;
            }

            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                close(pipefd[0]); close(pipefd[1]);
                continue;
            }

            if (pid == 0) {
                // child runs more, reads from pipe
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);

                char *margs[] = {(char*)"more", nullptr};
                execvp(margs[0], margs);
                perror("execvp more");
                _exit(1);
            } else {
                // parent writes help text into pipe
                close(pipefd[0]);
                string text =
                    "myshell - simple shell\n\n"
                    "Built-in commands:\n"
                    "  cd [DIRECTORY]      Change directory; if no DIRECTORY, print current directory.\n"
                    "  dir [DIRECTORY]     List contents of directory.\n"
                    "  environ             List environment variables.\n"
                    "  set VAR VALUE       Set environment variable.\n"
                    "  echo [TEXT]         Print text.\n"
                    "  help                Show this manual using more.\n"
                    "  pause               Wait until Enter is pressed.\n"
                    "  quit                Exit shell.\n\n"
                    "External commands:\n"
                    "  Anything else runs using fork + execvp.\n\n"
                    "Redirection:\n"
                    "  < infile, > outfile (truncate), >> outfile (append)\n"
                    "Background:\n"
                    "  Add & at end.\n";

                write(pipefd[1], text.c_str(), text.size());
                close(pipefd[1]);
                waitpid(pid, nullptr, 0);
                continue;
            }
        }

        // =========================
        // EXTERNAL COMMANDS
        // =========================
        // TA-style: fork -> child does redirection -> execvp
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            // child: input redirection
            if (!inFile.empty()) {
                int inFD = open(inFile.c_str(), O_RDONLY);
                if (inFD < 0) { perror("open input"); _exit(1); }
                if (dup2(inFD, STDIN_FILENO) < 0) { perror("dup2 stdin"); close(inFD); _exit(1); }
                close(inFD);
            }

            // child: output redirection
            if (!outFile.empty()) {
                int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
                int outFD2 = open(outFile.c_str(), flags, 0644);
                if (outFD2 < 0) { perror("open output"); _exit(1); }
                if (dup2(outFD2, STDOUT_FILENO) < 0) { perror("dup2 stdout"); close(outFD2); _exit(1); }
                close(outFD2);
            }

            // prepare execvp args
            vector<char*> cargs;
            for (string &s : argsOnly) cargs.push_back((char*)s.c_str());
            cargs.push_back(nullptr);

            execvp(cargs[0], cargs.data());
            perror("execvp");
            _exit(1);

        } else {
            // parent: wait unless background
            if (!background) {
                waitpid(pid, nullptr, 0);
            } else {
                cout << "[background pid " << pid << "]\n";
            }
        }
    }

    if (batch) fclose(batch);
    return 0;
}
