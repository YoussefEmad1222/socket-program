#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h> //socket
#include <arpa/inet.h> //inet_addr
#include <poll.h>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#define BUF_SIZE 1024
void parse_request(string request , int client_socket) {


}

void *handle_client(void *arg) {
    int client_socket = *(int *) arg;
    cout << "the client socket is " << client_socket << endl;
    char buffer[BUF_SIZE];
    string request;
    int read_size;
    read_size = recv(client_socket, buffer, BUF_SIZE, 0);
    cout << "the read size is " << read_size << endl;
   if (read_size > 0) {
    cout << "the request is " << string(buffer) << endl;
    string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    response += "<!DOCTYPE html><html><head><title>Test</title></head><body><h1>Test</h1></body></html>";
    send(client_socket, response.c_str(), response.size(), 0);
    cout << "the response sent" << endl;
   }
   else {
    printf("Error receiving\n");
   }
    close(client_socket);
    pthread_exit(NULL);
}



int main() {
    int PORT = 8080;
    int server_socket;
    struct sockaddr_in server_address;
    int address_length = sizeof(server_address);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        printf("Error creating socket\n");
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    int bind_status = bind(server_socket, (struct sockaddr *) &server_address, address_length);
    if (bind_status < 0) {
        printf("Error binding socket\n");
        return 1;
    }
    int listen_status = listen(server_socket, 1);
    if (listen_status < 0) {
        printf("Error listening socket\n");
        return 1;
    }
    printf("Server listening on port %d\n", PORT);
    while (1) {
        struct sockaddr_in clntAddr{};
        socklen_t clntAddrLen = sizeof(clntAddr);
        int clntSock = accept( server_socket , (struct sockaddr *) &clntAddr, &clntAddrLen);
        if (clntSock < 0) {
            perror("accept() failed");
            break; // Terminate the server on accept error
        }
        pthread_t thread;
        if(pthread_create(&thread, NULL, handle_client, (void *) &clntSock) < 0) {
            printf("Error creating thread\n");
            return 1;
        }
        pthread_detach(thread);
    }
    return 0;
}