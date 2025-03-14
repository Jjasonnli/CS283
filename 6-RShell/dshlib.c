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
        return ERR_EXEC_CMD;
    }
    
    if (pid == 0) {
        // Child process: execute command
        execvp(cmd->argv[0], cmd->argv);
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
    if (cmd->argc < 2) {
        return ERR_CMD_ARGS_BAD;
    }
    
    if (chdir(cmd->argv[1]) != 0) {
        return ERR_EXEC_CMD;
    }
    
    return OK;
}

/*
 * Executes a pipeline of commands.
 */
int execute_pipeline(command_list_t *clist) {
    int num_cmds = clist->num;
    int pipes[CMD_MAX - 1][2]; // Pipes between commands
    pid_t pids[CMD_MAX];

    // Create pipes
    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return ERR_MEMORY;
        }
    }

    // Fork processes
    for (int i = 0; i < num_cmds; i++) {
        pids[i] = fork();
        
        if (pids[i] < 0) {
            return ERR_MEMORY;
        }

        if (pids[i] == 0) { // Child process
            // read from previous pipe
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            // write to next pipe
            if (i < num_cmds - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Close all pipes in child process
            for (int j = 0; j < num_cmds - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Execute command
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            exit(ERR_EXEC_CMD);
        }
    }

    // Close all pipes in parent process
    for (int i = 0; i < num_cmds - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < num_cmds; i++) {
        waitpid(pids[i], NULL, 0);
    }

    return OK;
}

/*
 * Implements the main shell loop.
 */
int exec_local_cmd_loop() {
    char input[SH_CMD_MAX];
    command_list_t cmd_list;
    
    while (1) {
        printf("%s", SH_PROMPT);
        
        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;
        }
        
        input[strcspn(input, "\n")] = '\0'; // Remove trailing newline
        
        int res = build_cmd_list(input, &cmd_list);
        if (res == WARN_NO_CMDS) {
            printf("%s", CMD_WARN_NO_CMD);
            continue;
        }
        
        if (strcmp(cmd_list.commands[0].argv[0], EXIT_CMD) == 0) {
            return OK_EXIT;
        } else if (strcmp(cmd_list.commands[0].argv[0], "cd") == 0) {
            exec_cd(&cmd_list.commands[0]);
        } else if (cmd_list.num == 1) {
            exec_cmd(&cmd_list.commands[0]);
        } else {
            execute_pipeline(&cmd_list);
        }

        free_cmd_list(&cmd_list);
    }
    
    return OK;
}

/*
 * Parses the command line into a list of commands separated by pipes.
 */
int build_cmd_list(char *cmd_line, command_list_t *clist) {
    memset(clist, 0, sizeof(command_list_t));

    char *cmd_token;
    char *saveptr;
    int cmd_count = 0;

    // Tokenize by "|"
    cmd_token = strtok_r(cmd_line, PIPE_STRING, &saveptr);
    while (cmd_token != NULL) {
        if (cmd_count >= CMD_MAX) {
            return ERR_TOO_MANY_COMMANDS;
        }

        if (build_cmd_buff(cmd_token, &clist->commands[cmd_count]) != OK) {
            return ERR_MEMORY;
        }

        cmd_count++;
        cmd_token = strtok_r(NULL, PIPE_STRING, &saveptr);
    }

    clist->num = cmd_count;
    return OK;
}

/*
 * Frees memory allocated for the command list.
 */
int free_cmd_list(command_list_t *cmd_lst) {
    for (int i = 0; i < cmd_lst->num; i++) {
        free_cmd_buff(&cmd_lst->commands[i]);
    }
    return OK;
}

/*
 * Allocates memory for a command buffer and tokenizes the command.
 */
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = strdup(cmd_line);
    if (!cmd_buff->_cmd_buffer) {
        return ERR_MEMORY;
    }

    cmd_buff->argc = 0;
    cmd_buff->argv[cmd_buff->argc] = strtok(cmd_buff->_cmd_buffer, " ");
    
    while (cmd_buff->argv[cmd_buff->argc] != NULL) {
        cmd_buff->argc++;
        if (cmd_buff->argc >= CMD_ARGV_MAX - 1) {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }
        cmd_buff->argv[cmd_buff->argc] = strtok(NULL, " ");
    }
    
    cmd_buff->argv[cmd_buff->argc] = NULL; // Null-terminate argv array
    return OK;
}

/*
 * Frees memory allocated for a command buffer.
 */
int free_cmd_buff(cmd_buff_t *cmd_buff) {
    free(cmd_buff->_cmd_buffer);
    return OK;
}