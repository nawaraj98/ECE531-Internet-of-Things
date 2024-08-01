// Compilation command: gcc -o hw hw.c -lcurl

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <curl/curl.h>

// Constants for HTTP methods
#define HTTP_GET 0
#define HTTP_POST 1
#define HTTP_PUT 2
#define HTTP_DELETE 3

// Function to perform HTTP GET request
int http_get(const char *url) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return 1;
        }
        curl_easy_cleanup(curl);
    }
    return 0;
}

// Function to perform HTTP POST request
int http_post(const char *url, const char *data) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return 1;
        }
        curl_easy_cleanup(curl);
    }
    return 0;
}

// Function to perform HTTP PUT request
int http_put(const char *url, const char *data) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return 1;
        }
        curl_easy_cleanup(curl);
    }
    return 0;
}

// Function to perform HTTP DELETE request
int http_delete(const char *url) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return 1;
        }
        curl_easy_cleanup(curl);
    }
    return 0;
}

// Function to escape and prepare data for transmission
char* escape_data(const char *data) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize CURL\n");
        return NULL;
    }
    char *output = curl_easy_escape(curl, data, strlen(data));
    curl_easy_cleanup(curl);
    return output;
}

// Function to print help information
void print_help(char *progname) {
    printf("Usage: %s [options] <message>\n", progname);
    printf("Options:\n");
    printf("  -u, --url     Specify the URL (required)\n");
    printf("  -o, --post    Perform HTTP POST request\n");
    printf("  -g, --get     Perform HTTP GET request\n");
    printf("  -p, --put     Perform HTTP PUT request\n");
    printf("  -d, --delete  Perform HTTP DELETE request\n");
    printf("  -h, --help    Print this help message\n");
}

// Main function
int main(int argc, char *argv[]) {
    int opt;
    int http_method = -1;
    char *url = NULL;
    char *data = NULL;
    char *escaped_data = NULL;

    static struct option long_options[] = {
        {"url", required_argument, 0, 'u'},
        {"post", no_argument, 0, 'o'},
        {"get", no_argument, 0, 'g'},
        {"put", no_argument, 0, 'p'},
        {"delete", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    // Parse command line arguments
    while ((opt = getopt_long(argc, argv, "u:ogpdh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'u':
                url = optarg;
                break;
            case 'o':
                http_method = HTTP_POST;
                break;
            case 'g':
                http_method = HTTP_GET;
                break;
            case 'p':
                http_method = HTTP_PUT;
                break;
            case 'd':
                http_method = HTTP_DELETE;
                break;
            case 'h':
                print_help(argv[0]);
                return 0;
            default:
                fprintf(stderr, "Usage: %s [options] <message>\nType \"./hw -h\" for help option.\n", argv[0]);
                return 1;
        }
    }

    // Ensure URL is provided
    if (url == NULL) {
        fprintf(stderr, "Error: URL is required.\nType \"./hw -h\" for help option.\n");
        return 1;
    }

    // Process the remaining arguments as message data
    if (optind < argc) {
        // Concatenate all remaining arguments into a single string
        int total_length = 0;
        for (int i = optind; i < argc; ++i) {
            total_length += strlen(argv[i]) + 1; // +1 for space or null terminator
        }

        data = (char *)malloc(total_length * sizeof(char));
        if (data == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\nType \"./hw -h\" for help option.\n");
            return 1;
        }

        strcpy(data, "");
        for (int i = optind; i < argc; ++i) {
            strcat(data, argv[i]);
            strcat(data, " ");
        }

        // Escape the data to handle special characters
        escaped_data = escape_data(data);
        if (escaped_data == NULL) {
            fprintf(stderr, "Error: Data escaping failed.\n");
            free(data);
            return 1;
        }
    }

    // Perform HTTP request based on the chosen method
    int result = 0;
    switch (http_method) {
        case HTTP_GET:
            result = http_get(url);
            break;
        case HTTP_POST:
            if (escaped_data == NULL) {
                fprintf(stderr, "Error: POST requires message data.\nType \"./hw -h\" for help option.\n");
                result = 1;
            } else {
                result = http_post(url, escaped_data);
            }
            break;
        case HTTP_PUT:
            if (escaped_data == NULL) {
                fprintf(stderr, "Error: PUT requires message data.\nType \"./hw -h\" for help option.\n");
                result = 1;
            } else {
                result = http_put(url, escaped_data);
            }
            break;
        case HTTP_DELETE:
            result = http_delete(url);
            break;
        default:
            fprintf(stderr, "Error: HTTP method not specified.\nType \"./hw -h\" for help option.\n");
            result = 1;
            break;
    }

    // Free allocated memory
    if (data != NULL) {
        free(data);
    }
    if (escaped_data != NULL) {
        curl_free(escaped_data);
    }

    return result;
}
