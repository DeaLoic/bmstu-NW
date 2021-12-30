#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <inttypes.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <signal.h>
#include "../common.h"

int fd_socket;
void sigint_catcher(int signum)
{
    printf("Ctrl+C caught - closing socket \n");
    close(fd_socket);
    exit(0);
}

char* form_request(char* path)
{
    char* get = "GET /";
    char* http_version = " HTTP/1.1\r\n";
    char* request = malloc((strlen(path) + strlen(get) + strlen(http_version) + 1) * sizeof(char));
    strcpy(request, get);
    printf(" Req: %s\n", request);
    strcpy(request + strlen(get), path);
    printf(" Req: %s\n", request);
    strcpy(request + strlen(get) + strlen(path), http_version);
    printf(" Req: %s\n", request);
    return request;
}

int main(int argc, char **argv)
{
    fd_socket = socket(AF_INET, SOCK_STREAM, 0);
    signal(SIGINT, sigint_catcher);
    if (fd_socket < 0) {
        printf("Failed create socket: %s\n", strerror(errno));
        return -errno;
    }
 
    struct hostent *host = gethostbyname(SERVER_HOST);
    if (!host) {
        printf("Error gethostbyname: %s\n", strerror(errno));
        return errno;
    }
 
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
    
    if (connect(fd_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Failed connect to server: %s\n", strerror(errno));
        return errno;
    }
 
    char path[MSG_MAX_LEN];
    int code_error = 0;
    while (!code_error) {
        printf("Input path (Enter for exit): ");
        fgets(path, MSG_MAX_LEN, stdin);
        printf("\n");
        size_t size = strlen(path);
        if (path[size - 1] == '\n') {
            if (size == 1)
            {
                break;
            }
            path[size - 1] = 0;
            size--;
        }
        
        char* req = form_request(path);
        if (send(fd_socket, req, strlen(req), 0) < 0) {
            printf("Failed send to server: %s", strerror(errno));
            return errno;
        }
        
        char buf[MSG_MAX_LEN];
        int readed = MSG_MAX_LEN - 1;
        while (readed == MSG_MAX_LEN - 1)
        {
            readed = recv(fd_socket, buf, MSG_MAX_LEN - 1, 0);
            printf("resded: %d", readed);
            buf[readed] = '\0';
            if (readed > 0) 
            {
                printf("%s", buf);
            }
        }
        if (readed < 0)
        {
            printf("Failed to recv from server: %s", strerror(errno));
            return errno;
        }
    }
    
    close(fd_socket);
    return code_error;
}
