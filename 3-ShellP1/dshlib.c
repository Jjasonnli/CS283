#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    // Initialize clist
    memset(clist, 0, sizeof(command_list_t));

    // Check for empty input
    if (strlen(cmd_line) == 0)
    {
        return WARN_NO_CMDS;
    }

    char *cmd_token;
    char *saveptr;
    int cmd_count = 0;

    // Tokenize command input based on "|"
    cmd_token = strtok_r(cmd_line, PIPE_STRING, &saveptr);
    while (cmd_token != NULL)
    {
        if (cmd_count >= CMD_MAX)
        {
            return ERR_TOO_MANY_COMMANDS;
        }

        // Trim leading spaces
        while (*cmd_token == SPACE_CHAR) cmd_token++;

        
        if (*cmd_token == '\0')
        {
            return WARN_NO_CMDS;
        }

        // Extract command name and arguments
        char *arg_token = strchr(cmd_token, SPACE_CHAR);
        if (arg_token != NULL)
        {
            *arg_token = '\0';  // Split exe and args
            arg_token++;        // Move to args
            while (*arg_token == SPACE_CHAR) arg_token++; // Trim leading spaces in args
        }

        // Check size constraints
        if (strlen(cmd_token) >= EXE_MAX || (arg_token && strlen(arg_token) >= ARG_MAX))
        {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        // Store parsed command
        strcpy(clist->commands[cmd_count].exe, cmd_token);
        if (arg_token)
        {
            strcpy(clist->commands[cmd_count].args, arg_token);
        }
        cmd_count++;

        // Get next command
        cmd_token = strtok_r(NULL, PIPE_STRING, &saveptr);
    }

    clist->num = cmd_count;
    return OK;
}
