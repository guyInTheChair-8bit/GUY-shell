#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUF_SIZE 1024
#define ANSI_COLOR_GREEN   "\033[1;32m"
#define ANSI_COLOR_RESET   "\033[0m"

#ifdef _WIN32
    #define PATH_DELIMITER ";"
#else
    #define PATH_DELIMITER ":"
#endif

const char* builtin_commands[] = {"exit", "echo", "type", "pwd", "cd"};
const int no_of_builtin_commands = 5;

void type(char* command);
int get_full_path(char* command, char* full_path, size_t max_size);
void execute_external_program(char* full_path, char* input);

int main() {
    
    setbuf(stdout, NULL);

    char input[BUF_SIZE];
    char* input_copy;
    char* command;

    while (1) {
        printf(ANSI_COLOR_GREEN "GUY" ANSI_COLOR_RESET "~$ ");

        if (fgets(input, sizeof(input), stdin) != NULL) {
            input[strlen(input) - 1] = '\0';
            input_copy = strdup(input);
        }
        else {
            printf("ERROR reading input command\n");
        }


        if (strcmp(input, "exit") == 0) {
            free(input_copy);
            return 0;
        }
        else {
            command = strtok(input, " \n");

            if (command == NULL) continue;

            if (strcmp(command, "echo") == 0) {
                char* text = strtok(NULL, "\n");
                if (text != NULL) {
                    printf("%s\n", text);
                }
                else {
                    printf("\n");
                }
            }
            else if (strcmp(command, "type") == 0) {
                command = strtok(NULL, " \n");
                if (command == NULL) continue;
                
                type(command);
            }
            else if (strcmp(command, "pwd") == 0) {
                char* absolute = realpath(".", NULL);
                if (absolute) {
                    printf("%s\n", absolute);
                    free(absolute);
                }
                else {
                    printf("FAILED to fetch current working directory\n");
                }
            }
            else if (strcmp(command, "cd") == 0) {
                char* path = strtok(NULL, " \n");
                
                if (path == NULL || strcmp(path, "~") == 0) {
                    path = getenv("HOME");
                    int success = chdir(path);
                }
                else if (path != NULL) {
                    int success = chdir(path);
                    if (success != 0) {
                        printf("cd: %s: No such file or directory\n", path);
                    }
                }
            }
            else {

                char full_path[256];
                
                if (get_full_path(command, full_path, sizeof(full_path))) {
                    execute_external_program(full_path, input_copy);
                    continue;
                }

                printf("%s: command not found\n", command);
            }
        }
        
    }
}

void type (char* command) {

    int found = 0;
    for (int i = 0; i < no_of_builtin_commands; i++) {
        if (strcmp(command, builtin_commands[i]) == 0) {
            printf("%s is a shell builtin\n", command);
            found = 1;
            break;
        }
    }

    if (!found) {
        //handling for the case that the given command is not a builtin 
        char full_path[256];
        found = get_full_path(command, full_path, sizeof(full_path));
        if (found) printf("%s is %s\n", command, full_path);
    }

    if (!found) {
        printf("%s: not found\n", command);
    }
 
}

int get_full_path(char* command, char* full_path, size_t max_size) {

    char* path_copy = strdup(getenv("PATH"));

    char* dir = strtok(path_copy, PATH_DELIMITER);

    while (dir != NULL) {
                    

        snprintf(full_path, max_size, "%s/%s", dir, command);

        if (access(full_path, F_OK) == 0) {
            if (access(full_path, X_OK) == 0) {
                free(path_copy);
                return 1;
            }    
        }

        dir = strtok(NULL, PATH_DELIMITER);
    }

    return 0;
}                

void execute_external_program(char* full_path, char* input) {
   char* args[100];

   int i = 0;

   char* token = strtok(input, " \n");
   args[i++] = token;

   while (1) {
        token = strtok(NULL, " \n");
        if (token == NULL) break;
        args[i++] = token;
   }

   args[i] = NULL;

    //creating a child process
    pid_t pid = fork();

    if (pid < 0) {
        perror("FAILED to create a child process.\n");
        return ;
    }
    else if (pid == 0) {
        execv(full_path, args);

        perror("failed to execute program\n");
        exit(1);
    }
    else {
        int status;
        wait(&status);
    }
} 
