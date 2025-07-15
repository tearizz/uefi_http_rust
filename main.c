#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define BUFFER_SIZE 4096
#define SERVER_IP "10.20.173.8"
#define PORT 80
#define ENDPOINT "/v1/keypair"
#define USER_AGENT "C-Json-Client/1.0"
#define CONTENT_TYPE "application/json"

// void print_http_response(const char *response) {
//     const char *header_end = str(response, "\r\n\r\n");
//     if (header_end) {
//         printf("HTTP Response Headers:\n");
//         // Print headers 
//         int header_length = header_end - response;
//         char header[header_length + 1];
//         strncpy(header, response, header_length);
//         header[header_length] = '\0';
//         printf("%s\n", header);

//         // Print body
//         printf("HTTP Response Body:\n");
//         printf("%s\n", header_end + 4); // Skip past the headers
//     } else {
//         printf("Complete HTTP:\n %s\n", response);
//     }
// }

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    ssize_t bytes_sent, bytes_received;
    int content_length;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE); 
    }

    // Set up server address 
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IP address from string to binary form 
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))<0) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server %s:%d\n", SERVER_IP, PORT);

    // Prepare HTTP POST request
    const char *json_body = "{\"algo\": \"sm2\",\"kms\": \"\",\"flow\": \"classic\"}";
    content_length = strlen(json_body);

    snprintf(request, BUFFER_SIZE,
            "POST %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "User-Agent: %s\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",
            ENDPOINT, SERVER_IP, USER_AGENT, CONTENT_TYPE, content_length,json_body);
   
    // Send HTTP request
    bytes_sent = send(sockfd, request, strlen(request), 0);
    if (bytes_sent < 0) {
        perror("Failed to send request");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("---------------------------------------------\n");
    printf("HTTP Request:\n%s\n", request);
    printf("---------------------------------------------\n");
    printf("Sent %zd bytes to server\n", bytes_sent);

    // Receive HTTP response
    printf("Waiting for response...\n");

    bytes_received = recv(sockfd, response, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0) {
        perror("Failed to receive response");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    response[bytes_received] = '\0'; // Null-terminate the response

    printf("Received %zd bytes from server\n", bytes_received);
    printf("---------------------------------------------\n");
    printf("HTTP Response:\n%s\n", response);
    printf("---------------------------------------------\n");

    // Close the socket
    close(sockfd);
    printf("Connection closed\n");


    return 0;

}