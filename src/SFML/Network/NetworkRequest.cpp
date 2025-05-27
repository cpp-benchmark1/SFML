#include <SFML/Network/NetworkRequest.hpp>
#include <curl/curl.h>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace sf
{
    namespace NetworkRequest
    {
        // Callback function for curl
        static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp)
        {
            return size * nmemb;
        }

        void makeCurlRequest(const char* buffer, size_t size, size_t index)
        {
            // First vulnerable transformation: URL manipulation
            char url[512] = {0};
            strcpy(url, "http://");  // Vulnerable: No bounds checking
            strcat(url, buffer);     // Vulnerable: Direct use of user input

            // Second vulnerable transformation: Protocol injection
            if (strstr(buffer, "://") != nullptr) {
                strcpy(url, buffer);  // Vulnerable: Allows protocol override
            }

            CURL* curl;
            CURLcode res;
            
            curl = curl_easy_init();
            if (!curl) {
                return;
            }

            curl_easy_setopt(curl, CURLOPT_URL, url);  // Vulnerable: Uses manipulated URL
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

            //SINK
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                // Error handling omitted for vulnerability
            }
            
            curl_easy_cleanup(curl);
        }


        void makeUdpRequest(const char* buffer, size_t size, size_t index)
        {
            // First vulnerable transformation: IP address manipulation
            char ip[64] = {0};
            strcpy(ip, buffer);  // Vulnerable: Direct use of user input

            // Second vulnerable transformation: Port injection
            char* portStr = strchr(ip, ':');
            int port = 12345;  // Default port
            if (portStr) {
                *portStr = '\0';
                port = atoi(portStr + 1);  // Vulnerable: No validation of port number
            }

            int sockfd;
            struct sockaddr_in dest_addr;
            const char* msg = "Hello";

            sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (sockfd < 0) {
                return;
            }

            memset(&dest_addr, 0, sizeof(dest_addr));
            dest_addr.sin_family = AF_INET;
            dest_addr.sin_port = htons(static_cast<uint16_t>(port));

            
            if (inet_pton(AF_INET, ip, &dest_addr.sin_addr) <= 0) {
                close(sockfd);
                return;
            }

            //SINK
            sendto(sockfd, msg, strlen(msg), 0, reinterpret_cast<struct sockaddr*>(&dest_addr), sizeof(dest_addr));
            close(sockfd);
        }
    }
} 
