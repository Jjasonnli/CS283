#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

#include "dshlib.h"
#include "rshlib.h"

/*
 * start_server(ifaces, port, is_threaded)
 */
int start_server(char *ifaces, int port, int is_threaded) {
    int svr_socket;
    int rc;

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0) {
        return svr_socket;  // Return error code 
    }

    rc = process_cli_requests(svr_socket);
    stop_server(svr_socket);

    return rc;
}

/*
 * stop_server(svr_socket)
 */
int stop_server(int svr_socket) {
    return close(svr_socket);
}

/*
 * boot_server(ifaces, port)
 */
int boot_server(char *ifaces, int port) {
    int svr_socket;
    struct sockaddr_in addr;
    int enable = 1;

    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }

    setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ifaces, &addr.sin_addr);

    if (bind(svr_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    if (listen(svr_socket, 20) == -1) {
        perror("listen");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

/*
 * process_cli_requests(svr_socket)
 */
int process_cli_requests(int svr_socket) {
    int cli_socket;
    int rc = OK;

    while (1) {
        cli_socket = accept(svr_socket, NULL, NULL);
        if (cli_socket < 0) {
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }

        rc = exec_client_requests(cli_socket);
        close(cli_socket);

        if (rc == OK_EXIT) {
            break;
        }
    }

    return rc;
}

/*
 * exec_client_requests(cli_socket)
 */
int exec_client_requests(int cli_socket) {
    char *io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (!io_buff) {
        return ERR_RDSH_SERVER;
    }

    while (1) {
        ssize_t recv_bytes = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ, 0);
        if (recv_bytes < 0) {
            perror("recv");
            free(io_buff);
            return ERR_RDSH_COMMUNICATION;
        } else if (recv_bytes == 0) {
            free(io_buff);
            return OK;
        }

        io_buff[recv_bytes - 1] = '\0';

        if (strcmp(io_buff, "exit") == 0) {
            free(io_buff);
            return OK;
        }

        if (strcmp(io_buff, "stop-server") == 0) {
            free(io_buff);
            return OK_EXIT;
        }

        command_list_t cmd_list;
        if (build_cmd_list(io_buff, &cmd_list) == WARN_NO_CMDS) {
            send_message_string(cli_socket, CMD_WARN_NO_CMD);
            continue;
        }

        send_message_eof(cli_socket);

        free_cmd_list(&cmd_list);
    }

    free(io_buff);
    return OK;
}

/*
 * send_message_eof(cli_socket)
 */
int send_message_eof(int cli_socket) {
    int sent_len = send(cli_socket, &RDSH_EOF_CHAR, 1, 0);
    return (sent_len == 1) ? OK : ERR_RDSH_COMMUNICATION;
}

/*
 * send_message_string(cli_socket, char *buff)
 */
int send_message_string(int cli_socket, char *buff) {
    int sent_len = send(cli_socket, buff, strlen(buff) + 1, 0);
    if (sent_len != (int)strlen(buff) + 1) {
        return ERR_RDSH_COMMUNICATION;
    }
    return send_message_eof(cli_socket);
}

/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int pipes[clist->num - 1][2];
    pid_t pids[clist->num];
    int pids_st[clist->num];
    int exit_code = OK;

    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return ERR_RDSH_COMMUNICATION;
        }
    }

    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            return ERR_MEMORY;
        }

        if (pids[i] == 0) {
            if (i == 0) {
                dup2(cli_sock, STDIN_FILENO);
            }
            if (i == clist->num - 1) {
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            }

            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            exit(ERR_EXEC_CMD);
        }
    }

    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &pids_st[i], 0);
    }

    exit_code = WEXITSTATUS(pids_st[clist->num - 1]);

    for (int i = 0; i < clist->num; i++) {
        if (WEXITSTATUS(pids_st[i]) == EXIT_SC) {
            exit_code = EXIT_SC;
        }
    }

    return exit_code;
}

/*
 * rsh_match_command(const char *input)
 */
Built_In_Cmds rsh_match_command(const char *input) {
    if (strcmp(input, "exit") == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    if (strcmp(input, "stop-server") == 0)
        return BI_CMD_STOP_SVR;
    if (strcmp(input, "rc") == 0)
        return BI_CMD_RC;
    return BI_NOT_BI;
}

/*
 * rsh_built_in_cmd(cmd_buff_t *cmd)
 */
Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds ctype = rsh_match_command(cmd->argv[0]);

    switch (ctype) {
        case BI_CMD_EXIT:
            return BI_CMD_EXIT;
        case BI_CMD_STOP_SVR:
            return BI_CMD_STOP_SVR;
        case BI_CMD_RC:
            return BI_CMD_RC;
        case BI_CMD_CD:
            chdir(cmd->argv[1]);
            return BI_EXECUTED;
        default:
            return BI_NOT_BI;
    }
}