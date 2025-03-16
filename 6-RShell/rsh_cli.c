#include <sys/socket.h>
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
 * exec_remote_cmd_loop(server_ip, port)
 */
int exec_remote_cmd_loop(char *address, int port)
{
    char *cmd_buff = malloc(RDSH_COMM_BUFF_SZ);
    char *rsp_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (!cmd_buff || !rsp_buff) {
        return client_cleanup(-1, cmd_buff, rsp_buff, ERR_MEMORY);
    }

    int cli_socket = start_client(address, port);
    if (cli_socket < 0) {
        perror("start client");
        return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_CLIENT);
    }

    while (1) 
    {
        // Print prompt
        printf("%s", SH_PROMPT);

        // Get input
        if (!fgets(cmd_buff, RDSH_COMM_BUFF_SZ, stdin)) {
            break;
        }

        // Remove newline character
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // Send command including null terminator
        if (send(cli_socket, cmd_buff, strlen(cmd_buff) + 1, 0) < 0) {
            perror("send");
            return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
        }

        // Receive response from server
        ssize_t recv_bytes;
        int is_eof = 0;
        while (!is_eof) {
            recv_bytes = recv(cli_socket, rsp_buff, RDSH_COMM_BUFF_SZ, 0);
            if (recv_bytes < 0) {
                perror("recv");
                return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
            } else if (recv_bytes == 0) {
                return client_cleanup(cli_socket, cmd_buff, rsp_buff, OK);
            }

            is_eof = (rsp_buff[recv_bytes - 1] == RDSH_EOF_CHAR);
            if (is_eof) {
                rsp_buff[recv_bytes - 1] = '\0';  // Remove EOF marker
            }

            printf("%.*s", (int)recv_bytes, rsp_buff);
        }

        if (strcmp(cmd_buff, "exit") == 0) {
            break;
        }
    }

    return client_cleanup(cli_socket, cmd_buff, rsp_buff, OK);
}

/*
 * start_client(server_ip, port)
 */
int start_client(char *server_ip, int port){
    struct sockaddr_in addr;
    int cli_socket;

    cli_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_socket < 0) {
        perror("socket");
        return ERR_RDSH_CLIENT;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &addr.sin_addr);

    if (connect(cli_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(cli_socket);
        return ERR_RDSH_CLIENT;
    }

    return cli_socket;
}

/*
 * client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc)
 */
int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc){
    if(cli_socket > 0){
        close(cli_socket);
    }
    free(cmd_buff);
    free(rsp_buff);
    return rc;
}