#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <bits/stdc++.h>

using namespace std;

// Utility function to print error messages and exit
void DieWithUserMessage(const string &msg, const string &detail)
{
    cerr << msg << ": " << detail << endl;
    exit(1);
}

void DieWithSystemMessage(const string &msg)
{
    perror(msg.c_str());
    exit(1);
}

// Split a string into tokens using a specified delimiter
vector<string> split(const string &s, char delimiter)
{
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

// Generate an HTTP request string
string getRequest(const string &method, const string &filePath, const string &hostName, in_port_t portNumber)
{
    stringstream ss;
    ss << method << " " << filePath << " HTTP/1.1\r\nHost: " << hostName << "\r\n\r\n";
    return ss.str();
}

// Generate an HTTP request string for POST method
string postRequest(const string &method, const string &filePath, const string &hostName, in_port_t portNumber, const string &data)
{
    stringstream ss;
    ss << method << " " << filePath << " HTTP/1.1\r\nHost: " << hostName << "\r\nContent-Length: " << data.length() << "\r\n\r\n"
       << data;
    return ss.str();
}

// Send an HTTP request over a socket
void sendRequest(int sock, const string &request)
{
    ssize_t numBytes = send(sock, request.c_str(), request.length(), 0);
    if (numBytes < 0)
    {
        DieWithSystemMessage("send() failed");
    }
    else if (numBytes != request.length())
    {
        DieWithUserMessage("send()", "sent unexpected number of bytes");
    }
}

void saveToFile(int contentLength, int sock, const char *filePath)
{
    string Path = "./clientfiles";
    Path.append(filePath);
    ofstream outputFile(Path, ios::binary);
    if (!outputFile.is_open())
    {
        DieWithUserMessage("Failed to create file", filePath);
    }
    char buffer[2048];
    ssize_t numBytes;
    while (contentLength > 0)
    {
        int minSize = min(static_cast<int>(sizeof(buffer)), static_cast<int>(contentLength));
        numBytes = recv(sock, buffer, sizeof(buffer), 0);
        if (numBytes < 0)
        {
            DieWithSystemMessage("recv() failed");
        }
        contentLength -= numBytes;
        outputFile.write(buffer, numBytes);
    }
    outputFile.close();
}

// Receive an HTTP response from a socket and write it to a file
void receiveResponse(int sock, const char *filePath)
{
    char buffer[2048];
    string header;
    bool headerReceived = false;
    int contentLength = 0;
    ssize_t numBytes = recv(sock, buffer, sizeof(buffer), 0);
    cout << numBytes << endl;
    if (numBytes < 0)
    {
        DieWithSystemMessage("recv() failed");
    }
    string data(buffer, numBytes);
    if (!headerReceived)
    {
        header += data;
        size_t headerEnd = header.find("\r\n\r\n");
        if (headerEnd != string::npos)
        {
            headerReceived = true;
            string headerContent = header.substr(0, headerEnd);
            cout << headerContent << endl;
            size_t contentLengthStart = headerContent.find("Content-Length:");
            if (contentLengthStart != string::npos)
            {
                size_t contentLengthEnd = headerContent.find("\r\n", contentLengthStart);
                if (contentLengthEnd != string::npos)
                {
                    string contentLengthStr = headerContent.substr(contentLengthStart + 15, contentLengthEnd - contentLengthStart - 15);
                    contentLength = stoi(contentLengthStr);
                }
            }
        }
    }
    if (contentLength > 0)
    {
        saveToFile(contentLength, sock,filePath);
    }
}

int main(int argc, char *argv[])
{
    // Check command-line parameters
    if (argc != 3)
    {
        DieWithUserMessage("Parameter(s)", "<server-ip> <port-number>");
    }

    // Set server IP and port number
    const char *serverIP = argv[1];
    in_port_t portNumber = atoi(argv[2]);

    // Create a socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
    {
        DieWithSystemMessage("socket() failed");
    }

    // Configure the socket
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;

    // Convert the IP address from a string to a numeric value
    int rtnVal = inet_pton(AF_INET, serverIP, &servAddr.sin_addr.s_addr);
    if (rtnVal == 0)
    {
        DieWithUserMessage("inet_pton() failed", "invalid address string");
    }
    else if (rtnVal < 0)
    {
        DieWithSystemMessage("inet_pton() failed");
    }

    // Set the port number
    servAddr.sin_port = htons(portNumber);

    // Connect to the server
    if (connect(sock, (sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        DieWithSystemMessage("connect() failed");
    }

    // Open the input file
    ifstream file("input.txt");
    if (!file.is_open())
    {
        DieWithUserMessage("File not found", "input.txt");
    }

    // Process each line in the input file
    string line;
    while (getline(file, line))
    {
        vector<string> args = split(line, ' ');
        if (args[0] == "GET" || args[0] == "POST")
        {
            const char *filePath = args[1].c_str();
            const char *hostName = args[2].c_str();
            // check if the port number is given
            in_port_t portNumber = ((args.size() == 4 && args[0] == "GET")||(args.size() == 5 && args[0] == "POST")) ? atoi(args[3].c_str()) : 80;

            // Send the request
            if (args[0] == "GET")
            {
                string request = getRequest(args[0], filePath, hostName, portNumber);
                sendRequest(sock, request);
            }
            else if (args[0] == "POST")
            {
                string data;
                string dataFilePath = (args.size() == 4)? args[3] : args[4];
                ifstream dataFile(dataFilePath);
                if (!dataFile.is_open())
                {
                    DieWithUserMessage("Failed to open data file", dataFilePath);
                }
                stringstream dataStream;
                dataStream << dataFile.rdbuf();
                data = dataStream.str();
                dataFile.close();

                string request = postRequest(args[0], filePath, hostName, portNumber, data);
                sendRequest(sock, request);
            }

            receiveResponse(sock,filePath);
        }
        else
        {
            DieWithUserMessage("Unsupported command", args[0]);
        }

        // std::this_thread::sleep_for(std::chrono::seconds(3)); // Wait for 3 seconds
    }

    // Close the file and socket
    file.close();
    close(sock);
    exit(0);
}