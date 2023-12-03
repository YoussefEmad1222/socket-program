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

#define BUF_SIZE 1024 // Maximum size of buffer for messages

// DieWithUserMessage prints an error message and terminates the program.
void DieWithUserMessage(const string &msg, const string &detail) {
  cerr << msg << ": " << detail << endl;
  exit(1);
}
// This function parses the request from the client and returns a map of the
// headers and the body. The map will have key-value pairs of the headers and
// the content of each header, including the body.
unordered_map<string, string> parse_request(string request) {
  string delimiter = "\r\n";
  string token;
  istringstream ss(
      request); // Create an input string stream to read the request
  vector<string>
      results; // This vector will hold the lines of the request after parsing
  while (getline(ss, token)) {
    results.push_back(token); // Push each line into the vector
  }

  unordered_map<string, string>
      headers; // This map will hold the headers and the body
  bool body_start = false;
  string body;

  for (int i = 0; i < results.size(); i++) {
    if (results[i].empty()) { // If an empty line is encountered, it indicates the start of the body
      body_start = true;
      continue;
    }

    if (body_start) { // If body_start is true, append the current line to the body string
      body += results[i];
      continue;
    }

    if (i == 0) { // If it's the first line, parse the Method, Path, and Version
      headers["Method"] = results[i].substr(0, results[i].find(" "));
      headers["Path"] =
          results[i].substr(results[i].find(" ") + 1,
                            results[i].rfind(" ") - results[i].find(" ") - 1);
      headers["Version"] = results[i].substr(results[i].rfind(" ") + 1);
    } else if (results[i].find(":") !=
               string::npos) { // If the line contains a ":" character, it
                               // indicates a header
      headers[results[i].substr(0, results[i].find(":"))] =
          results[i].substr(results[i].find(":") + 2);
    }
  }

  headers["Body"] = results[results.size() - 1]; // Add the body to the headers map
  return headers;                  // Return the map of headers and body
}

void handle_get(unordered_map<string, string> results, int client_socket) {
  // Extract the path and version from the results map
  string path = results["Path"];
  string version = results["Version"];

  // Initialize an empty string to store the response
  string response;

  // Open the file specified by the path
  ifstream file(path);

  // If the file is successfully opened
  if (file.is_open()) {
    string line;

    // Read each line of the file and append it to the response string
    while (getline(file, line)) {
      response += line;
    }

    // Close the file
    file.close();

    // Prepend the response with the HTTP status line
    response = "HTTP/1.1 200 OK\r\n\r\n" + response;
  } else {
    // If the file cannot be opened, set the response to a 404 status line
    response = "HTTP/1.1 404 Not Found\r\n\r\n";
  }

  // Send the response string to the client socket
  send(client_socket, response.c_str(), response.length(), 0);
}

void handle_post(unordered_map<string, string> results, int client_socket) {
  string path = results["Path"];
  string version = results["Version"];
  string response;
  string body = results["Body"];
  ofstream file("." + path);
  cout << "file opened" << endl;
  if (file.is_open()) {
    cout << "file is opened" << endl;
    file << body;
    file.close();
    response = "HTTP/1.1 200 OK\r\n\r\n";
  } else {
    response = "HTTP/1.1 404 Not Found\r\n\r\n";
  }
  file.close();
  send(client_socket, response.c_str(), response.length(), 0);
}

// Function to handle a client connection
void *handle_client(void *arg) {
  // Extract the client socket from the argument
  int client_socket = *(int *)arg;

  // Buffer to store the received data
  char buffer[BUF_SIZE];

  // Variable to store the size of the received data
  int recv_size;

  // String to store the complete request
  string request;

  // Receive the request in chunks until the entire request is received
  do {
    // Receive data from the client and store it in the buffer
    recv_size = recv(client_socket, buffer, BUF_SIZE, 0);

    // If there was an error receiving the data, print an error message and
    // return
    if (recv_size < 0) {
      printf("Error receiving\n");
      return NULL;
    }

    // Append the received data to the request string
    request.append(buffer, recv_size);

  } while (recv_size == BUF_SIZE);

  // If there was an error receiving the data, print an error message and return
  if (recv_size < 0) {
    printf("Error receiving\n");
    return NULL;
  }
  unordered_map<string, string> results = parse_request(request);

  string method = results["Method"];

  // Check the HTTP method of the request and handle accordingly
  if (method == "GET") {
    handle_get(results, client_socket);
  } else if (method == "POST") {
    cout << "POST" << endl;
    handle_post(results, client_socket);
  } else {
    // If the method is not supported, send a 405 Method Not Allowed response
    string response = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
    send(client_socket, response.c_str(), response.length(), 0);
  }

  // Close the client socket
  close(client_socket);

  // Return NULL to indicate successful completion of the function
  return NULL;
}
int main() {

  // Define the port number
  int PORT = 8080;

  // Create a socket for the server
  int server_socket;

  // Store the server address
  struct sockaddr_in server_address;

  // Store the length of the server address
  int address_length = sizeof(server_address);

  // Create a socket with the specified address family, socket type, and
  // protocol
  server_socket = socket(AF_INET, SOCK_STREAM, 0);

  // Check if the socket creation was successful
  if (server_socket < 0) {
    printf("Error creating socket\n");
    return 1;
  }

  // Set the server address family to AF_INET (IPv4)
  server_address.sin_family = AF_INET;

  // Set the server port number and convert it to network byte order
  server_address.sin_port = htons(PORT);

  // Set the server IP address to INADDR_ANY (any available IP address)
  server_address.sin_addr.s_addr = INADDR_ANY;

  // Bind the socket to the server address
  int bind_status =
      bind(server_socket, (struct sockaddr *)&server_address, address_length);

  // Check if the bind operation was successful
  if (bind_status < 0) {
    printf("Error binding socket\n");
    return 1;
  }

  // Listen for incoming connections on the socket
  int listen_status = listen(server_socket, 1);

  // Check if the listen operation was successful
  if (listen_status < 0) {
    printf("Error listening socket\n");
    return 1;
  }

  // Print a message indicating that the server is listening on the specified
  // port
  printf("Server listening on port %d\n", PORT);

  // Enter an infinite loop to accept and handle client connections
  while (1) {
    // Store the client address
    struct sockaddr_in clntAddr {};

    // Store the length of the client address
    socklen_t clntAddrLen = sizeof(clntAddr);

    // Accept a client connection and create a new socket for the client
    int clntSock =
        accept(server_socket, (struct sockaddr *)&clntAddr, &clntAddrLen);

    // Check if the accept operation was successful
    if (clntSock < 0) {
      perror("accept() failed");
      break; // Terminate the server on accept error
    }
    // Create a new thread to handle the client connection
    thread t(handle_client, &clntSock);
    t.detach();
  }

  // Return 0 to indicate successful execution
  return 0;
}
