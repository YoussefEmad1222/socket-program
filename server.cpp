#include <arpa/inet.h> //inet_addr
#include <bits/stdc++.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> //socket
#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

#define BUFF_SIZE 1024

void setTimeout(int socket, int timeout) {
    struct timeval tv;
    tv.tv_sec = timeout / 10;
    tv.tv_usec = (timeout % 10) * 10;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));   
}


unordered_map<string, string> parse_request(string request, size_t &responseBytes) {
    string delimiter = "\r\n\r\n";
    string token;
    size_t pos;
    string s = request;
    vector<string> results;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        results.push_back(token);
        s.erase(0, pos + delimiter.length());
        //we want now to ckack if the length of the token is 0
        if(token.length() == 0){
            break;
        }
    }
    string delim = "\r\n";
    string tok;
    size_t posi;
    string s1 = results[0];
    vector<string> results1;
    while ((posi = s1.find(delim)) != std::string::npos) {
        tok = s1.substr(0, posi);
        results1.push_back(tok);
        s1.erase(0, posi + delim.length());
        //we want now to ckack if the length of the token is 0
        if(tok.length() == 0){
            break;
        }
    }
    results1.push_back(s1);

    unordered_map<string, string> headers;
    for (size_t i = 0; i < results1.size(); i++) {
        if (i == 0) {
            headers["Method"] = results1[i].substr(0, results1[i].find(" "));
            headers["Path"] = results1[i].substr(results1[i].find(" ") + 1, results1[i].rfind(" ") - results1[i].find(" ") - 1);
            headers["Version"] = results1[i].substr(results1[i].rfind(" ") + 1);
        } else if (results1[i].find(":") != string::npos) {
            headers[results1[i].substr(0, results1[i].find(":"))] = results1[i].substr(results1[i].find(":") + 2);
        }
    }
    responseBytes=request.find(delimiter)+4;
    return headers;
}

void DieWithUserMessage(const string &msg, const string &detail) {
    cerr << msg << ": " << detail << endl;
    exit(1);
}

void handle_get(int client_socket, const unordered_map<string, string>& header) {
    char buffer[BUFF_SIZE];
    string path = header.at("Path");
    string response;
    ifstream file("."+ path, ios::binary);
    if (!file.is_open()) {
        DieWithUserMessage("file is not found", "you tried to open a file that doesn't exist or you wrote the wrong path");
    }
    file.seekg(0, ios::end);
    size_t currentSize = 0;
    size_t sizeOfFile = file.tellg();
    file.seekg(0, ios::beg);
    response = "HTTP/1.1 200 OK\r\nContent-Length: " + to_string(sizeOfFile) + "\r\n" + "Content-Type: " +path.substr(path.find_last_of(".") + 1) + "\r\n\r\n";
    send(client_socket, response.c_str(), response.length(), 0);
    while (currentSize < sizeOfFile) {
        cout<<currentSize<<endl;
        int minSize = min(static_cast<int>(BUFF_SIZE), static_cast<int>(sizeOfFile - currentSize));
        file.read(buffer, minSize);
        int size = send(client_socket, buffer, minSize, 0);
        if (size < 0) {
            DieWithUserMessage("failed to send data", "the server failed to send data");
        }
        currentSize += size;
    }
    cout<<"total bytes sent: " <<currentSize<<endl;
    file.close();
}

void handle_post(int client_socket, char* buffer, int recv_size, unordered_map<string, string> header, size_t headerBytes) {
    string response = "HTTP/1.1 200 OK\r\n\r\n";
    string path = header.at("Path");
    ofstream f("." + path, ios::binary);
    if (!f.is_open()) {
        DieWithUserMessage("file is not found", "you tried to open a file that doesn't exist or you wrote the wrong path");
    }
    f.write(buffer + headerBytes, recv_size - headerBytes);
    size_t totalBytes = recv_size - headerBytes;
    size_t receivedBytes = stoi(header.at("Content-Length"));
    while (totalBytes < receivedBytes) {
        int size = recv(client_socket, buffer, BUFF_SIZE, 0);
        cout << size << endl;
        totalBytes += size;
        f.write(buffer, size);
    }
    response = "HTTP/1.1 200 OK\r\n\r\n";
    cout << response << endl;
    send(client_socket, response.c_str(), response.length(), 0);
    f.close();
}
void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Error setting timeout");
        return NULL;
    }
    while(1){
        char buffer[BUFF_SIZE];
        int recv_size = recv(client_socket, buffer, BUFF_SIZE, 0);
        if (recv_size < 0) {
            close(client_socket);
            return NULL;
        }

        size_t headerBytes = 0;
        unordered_map<string, string> header = parse_request(buffer, headerBytes);
        string method = header["Method"];
        if (method == "GET") {
            handle_get(client_socket, header);
        } else if (method == "POST") {
            handle_post(client_socket, buffer, recv_size, header, headerBytes);
        } else {
            string response = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
            send(client_socket, response.c_str(), response.length(), 0);
        }
    }
    close(client_socket);
    return NULL;
}
int main() {
    int PORT = 8090;
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

    int bind_status = bind(server_socket, (struct sockaddr *)&server_address, address_length);
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
    int i=0;
    while (true) {
        struct sockaddr_in clntAddr {};
        socklen_t clntAddrLen = sizeof(clntAddr);
        int clntSock = accept(server_socket, (struct sockaddr *)&clntAddr, &clntAddrLen);
        if (clntSock < 0) {
            perror("accept() failed");
            break;
        }
        thread t(handle_client, &clntSock);
        t.detach();
        if(i>=50){
            setTimeout(int(clntSock), i);
        }
        i++;
    }
    close(server_socket);
    return 0;
}
