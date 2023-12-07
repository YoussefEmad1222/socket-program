# Socket Program:

## HTTP Client 

This C++ HTTP client is designed to send HTTP requests to a server, supporting both **GET** and **POST** methods. It reads commands from an input file (`input.txt`), processes them sequentially, and interacts with the server.

## Features

- **GET Requests**: Retrieve files from the server.
- **POST Requests**: Upload data to the server.
- **Dynamic Port Handling**: Intelligently handle port numbers based on the input command.
- **File Download**: Download files from the server and save them locally (`./clientfiles`).

## Usage


1. **Compile the Code:**

    ```bash
    g++ http-client.cpp -o http-client
    ```

2. **Run the Client:**

    ```bash
    ./http-client <server-ip> <port-number>
    ```

    Replace `<server-ip>` and `<port-number>` with the appropriate values.

3. **Input File:**

    Edit the `input.txt` file to include HTTP requests. Each line represents a separate command.

4. **Execute the Client:**

    Run the compiled executable with the server's IP address and port number.

5. **View Output:**

    The client prints information about received data, and downloaded files are saved in `./clientfiles`.

#### Example Input File

```plaintext
GET /index.html example.com 80
POST /upload data.txt example.com 80
```

# Socket Program: HTTP Server

This C++ HTTP server listens for incoming connections, handles each client in a separate thread, and supports both **GET** and **POST** requests. It provides functionality to retrieve files and accept uploaded data.

## Features

- **GET Requests**: Retrieve files requested by clients.
- **POST Requests**: Accept uploaded data from clients.
- **Timeout Handling**: Set a timeout for clients to ensure efficient resource usage.

## Usage


2. **Compile the Code:**

    ```bash
    g++ http-server.cpp -o http-server 
    ```

3. **Run the Server:**

    ```bash
    ./http-server
    ```

    The server starts listening on port 8080 by default. Modify the `PORT` variable in the code to change the port.

4. **Handling Requests:**

    The server handles both GET and POST requests. Clients can connect and send requests using any HTTP client.

## Server Behavior

- The server listens for incoming connections and handles each client in a separate thread.
- GET Requests: Retrieve requested files and send them to the client.
- POST Requests: Accept uploaded data and save it to the appropriate file.

## Notes

- The server assumes a specific file structure. Ensure requested files or upload destinations exist.
- Error handling is implemented for various scenarios, including file not found and method not allowed.
- A timeout is set for each client connection to ensure efficient resource usage.

## Additional Details

- Customize the server behavior by modifying the source code.
- Ensure that the server has the necessary permissions to read/write files.
- Review console output for detailed information about server activities.

