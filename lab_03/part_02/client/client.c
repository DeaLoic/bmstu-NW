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
#include "../common.h"

char* form_request(char* path)
{
    char* get = "GET ";
    char* http_version = " HTTP/1.1\r\n";
    char* request = malloc((strlen(path) + strlen(get) + strlen(http_version) + 1) * sizeof(char));
    strcpy(request, get);
    strcpy(request + strlen(get), path);
    strcpy(request + strlen(get) + strlen(path), http_version);
    return request;
}

int main(int argc, char **argv)
{
    int fd_socket = socket(AF_INET, SOCK_STREAM, 0);
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
        
        if (send(fd_socket, form_request(path), size, 0) < 0) {
            printf("Failed send to server: %s", strerror(errno));
            return errno;
        }
        
        char buf[MSG_MAX_LEN];
        if (recv(sock, buf, MSG_MAX_LEN, 0) < 0)
            printf("Failed to recv from server: %s", strerror(errno));
            return errno;

        printf("Client has recieved an answer:\n\n%s", buf);
    }
    
    close(fd_socket);
    return code_error;
}
