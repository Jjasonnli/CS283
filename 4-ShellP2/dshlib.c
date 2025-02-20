#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

/*
 * Executes a single command from the parsed `cmd_buff_t` structure.
 */
int exec_cmd(cmd_buff_t *cmd) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Fork failed");
        return ERR_EXEC_CMD;
    }
    
    if (pid == 0) {
        // Child process: execute command
        execvp(cmd->argv[0], cmd->argv);
        perror("Execution failed"); // If execvp fails
        exit(ERR_EXEC_CMD);
    } else {
        // Parent process: wait for child
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

/*
 * Implements the built-in `cd` command.
 */
int exec_cd(cmd_buff_t *cmd) {
    if (cmd->argc == 1) {
        return OK; // Do nothing if no directory is specified
    }
    
    if (chdir(cmd->argv[1]) != 0) {
        perror("cd failed");
        return ERR_EXEC_CMD;
    }
    
    return OK;
}

/*
 * Parses the input string, handling spaces and quotes correctly.
 */
int parse_input(char *cmd_line, cmd_buff_t *cmd) {
    char *token;
    bool in_quotes = false;
    int argc = 0;
    
    cmd->_cmd_buffer = strdup(cmd_line);
    if (!cmd->_cmd_buffer) {
        return ERR_MEMORY;
    }
    
    char *ptr = cmd->_cmd_buffer;
    while (*ptr) {
        while (isspace(*ptr) && !in_quotes) ptr++; // Skip leading spaces
        
        if (*ptr == '\"') {
            in_quotes = !in_quotes;
            ptr++; // Skip quote
        }
        
        if (*ptr == '\0') break;
        
        cmd->argv[argc++] = ptr;
        
        while (*ptr && (in_quotes || !isspace(*ptr))) {
            if (*ptr == '\"') {
                in_quotes = !in_quotes;
                *ptr = '\0';
            }
            ptr++;
        }
        
        if (*ptr) {
            *ptr = '\0';
            ptr++;
        }
        
        if (argc >= CMD_ARGV_MAX - 1) {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }
    }
    
    cmd->argv[argc] = NULL;
    cmd->argc = argc;
    
    return argc > 0 ? OK : WARN_NO_CMDS;
}

/*
 * Implements the main shell loop.
 */
int exec_local_cmd_loop() {
    char input[SH_CMD_MAX];
    cmd_buff_t cmd;
    
    while (1) {
        printf("%s", SH_PROMPT);
        
        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;
        }
        
        input[strcspn(input, "\n")] = '\0'; // Remove trailing newline
        
        if (parse_input(input, &cmd) == WARN_NO_CMDS) {
            printf("%s", CMD_WARN_NO_CMD);
            continue;
        }
        
        if (strcmp(cmd.argv[0], EXIT_CMD) == 0) {
            return OK_EXIT;
        } else if (strcmp(cmd.argv[0], "cd") == 0) {
            exec_cd(&cmd);
        } else {
            exec_cmd(&cmd);
        }
        
        free(cmd._cmd_buffer);
    }
    
    return OK;
}
