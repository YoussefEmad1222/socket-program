# HTTP Client

## Overview

This repository contains a simple HTTP client implemented in C++ using sockets to communicate with a server. The client supports both **GET** and **POST** requests, providing functionality to interact with a remote server, retrieve files, and send data.

## Features

**GET Requests**: The client can send HTTP GET requests to a server to retrieve files.
**POST Requests**: HTTP POST requests are supported, allowing the upload of data to the server.
**Input File**: The client reads its commands from an input file (`input.txt`), where each line corresponds to a separate HTTP request.
Dynamic Port Handling: Intelligent handling of port numbers based on the input command, allowing flexibility in specifying the port in the input file.
**File Download**: The client can download files from the server and save them in a local directory (`./clientfiles`).

## Usage

1. **Clone the Repository:**

    ```bash
    git clone https://github.com/your-username/http-client.git
    ```

2. **Compile the Code:**

    ```bash
    g++ http-client.cpp -o http-client
    ```

3. Run the client:

    ```bash
    ./http-client <server-ip> <port-number>
    ```

Replace `<server-ip>` and `<port-number>` with the appropriate values.

4. **Input File:**

Edit the `input.txt` file to include your HTTP requests. Each line should represent a separate command.

5. Execute the client:

Run the compiled executable with the server's IP address and port number.

6. **View Output:**

The client will print information about the received data, and downloaded files will be saved in the `./clientfiles` directory.

## Example Input File

```plaintext
GET /index.html example.com 80
POST/upload data.txt example.com 80
```

