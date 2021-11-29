#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include "../common.h"

int fd_socket;

int clients[MAX_CLIENTS] = { 0 };

void sigint_catcher(int signum)
{
    printf("Ctrl+C caught - closing socket \n");
    close(fd_socket);
    exit(0);
}

char* get_extention(char* filename)
{
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

int handle_request(char *request, int client)
{
    std::string debug(msg);
    char *method = strtok(msg, " ");
    char *path = strtok(NULL, " /");
    char *version = strtok(NULL, " \r\n");
    char user[50];
    
    if (strcmp(method, "GET"))
    {
        return -1;
    }

    int rc = 0;
    char *status, *status_code;
    char *body = malloc(MSG_MAX_LEN);
    char *content_type = "text/html";

    char *response = malloc(MSG_MAX_LEN);
    body[0] = "\0";
    FILE *fl = fopen(strcat(PATH_ROOT, path), "rb");
    if (fl)
    {
        char *ext = get_extention(filename);

        if (strcmp(ext, "ico") == 0)
        {
            content_type = "image/x-icon";
        }
        else if (strcmp(ext, "json") == 0)
        {
            content_type = "application/json";
        }

        char* buf = char[100];
        while (!feof(fl) && fread(buf, 1, 100, fl) != 0)
        {
            strcpy(body + strlen(body), buf);
        }
        strcpy(body + strlen(body), "\0");
        status_code = "200";
        status = "OK";
    }
    else
    {
        rc = -2;
        status_code = "404";
        status = "Not Found";
        body = "<html>\n\r<body>\n\r<h1>404 Not Found</h1>\n\r</body>\n\r</html>";
    }
    strcpy(response, version);
    strcpy(response + strlen(response), " ");
    strcpy(response + strlen(response), status_code);
    strcpy(response + strlen(response), " \r\nContent-Length: ");
    strcpy(response + strlen(response), strlen(body));
    strcpy(response + strlen(response), "\r\nConnection: closed\r\nContent-Type: ");
    strcpy(response + strlen(response), content_type);
    strcpy(response + strlen(response), "; charset=UTF-8\r\n\r\n");
    strcpy(response + strlen(response), body);

    send(clients[client], response.c_str(), response.size(), 0);

    if (rc)
        printf("Client #%d has sent an invalid message.\n", client);
    else
        printf("Successfully handled a request from client #%d\n", client);
    return rc;
}

int register_client(unsigned int socket)
{
    int i = 0;
    while (i < MAX_CLIENTS && clients[i])
    {
        i++;
    }
    if (i < MAX_CLIENTS)
    {
        clients[i] = clientfd;
        printf("Register client %d\n\n", i);
        return 0;
    }
    return 1;
}

int main()
{
    printf("Server starting ...\n");
    chdir(FILES_DIR);

    fd_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (fd_socket < 0) {
        printf("Failed create socket: %s\n", strerror(errno));
        return -errno;
    }
 
    signal(SIGINT, sigint_catcher);
    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
 
    if (bind(fd_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Failed bind socket: %s\n", strerror(errno));
        return errno;
    }
 
    if (listen(fd_socket, MAX_COUNT) < 0) {
        printf("Failed listen socket: %s\n", strerror(errno));
        close(fd_socket);
        return errno;
    }

    printf("Server running ...\n");
    int client_sockets[MAX_COUNT] = {0};
    int client_socket = 0;
    while (1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(fd_socket, &rfds);
        int max_fd = fd_socket;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = client_sockets[i];
            if (fd > 0)
                FD_SET(fd, &rfds);
            max_fd = (fd > max_fd) ? (fd) : (max_fd);
        }
        int updates_cl_count = pselect(max_fd + 1, &rfds, NULL, NULL, NULL, NULL);
        if (FD_ISSET(fd_socket, &rfds)) {
            struct sockaddr_in client_addr;
            int client_addr_size = sizeof(client_addr);
            int accepted_socket = accept(fd_socket, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);
            if (accepted_socket < 0 || register_client(accepted_socket)) {
                printf("Failed accept: %s", strerror(errno));
                close(fd_socket);
                return errno;
            }
            printf("New client: fd = %d, address = %s:%d\n", accepted_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            
        }
        char request[MSG_MAX_LEN];
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = client_sockets[i];
            if ((fd > 0) && FD_ISSET(fd, &rfds)) {
                if (recv(fd, request, MSG_MAX_LEN) <= 0) {
                    client_sockets[i] = 0;
                    close(fd);
                    printf("Client disconnected: fd = %d\n", fd);
                } else {
                    thread (request, i)
                }
            }
        }
    }
    return 0;
}