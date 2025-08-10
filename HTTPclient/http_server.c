#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

const int BUFFER_SIZE = 104857600;

const char *get_file_extension(const char *file_name) {
    const char *dot = strrchr(file_name, '.');
    if (!dot || dot == file_name) {
        return "";
    }
    return dot + 1;
}

const char *get_mime_type(const char *file_ext) {
    if (strcasecmp(file_ext, "html") == 0 || strcasecmp(file_ext, "htm") == 0) {
        return "text/html";
    } else if (strcasecmp(file_ext, "txt") == 0) {
        return "text/plain";
    } else if (strcasecmp(file_ext, "jpg") == 0 || strcasecmp(file_ext, "jpeg") == 0) {
        return "image/jpeg";
    } else if (strcasecmp(file_ext, "png") == 0) {
        return "image/png";
    } else {
        return "application/octet-stream";
    }
}

bool case_insensitive_compare(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) {
            return false;
        }
        s1++;
        s2++;
    }
    return *s1 == *s2;
}

char *get_file_case_insensitive(const char *file_name) {
    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        return NULL;
    }

    struct dirent *entry;
    char *found_file_name = NULL;
    while ((entry = readdir(dir)) != NULL) {
        if (case_insensitive_compare(entry->d_name, file_name)) {
            found_file_name = entry->d_name;
            break;
        }
    }

    closedir(dir);
    return found_file_name;
}

char *url_decode(const char *src) {
    size_t src_len = strlen(src);
    char *decoded = malloc(src_len + 1);
    size_t decoded_len = 0;

    // decode %2x to hex
    for (size_t i = 0; i < src_len; i++) {
        if (src[i] == '%' && i + 2 < src_len) {
            int hex_val;
            sscanf(src + i + 1, "%2x", &hex_val);
            decoded[decoded_len++] = hex_val;
            i += 2;
        } else {
            decoded[decoded_len++] = src[i];
        }
    }

    // add null terminator
    decoded[decoded_len] = '\0';
    return decoded;
}

char waf_filename[200];

void build_http_response(const char *file_name, 
                        const char *file_ext, 
                        char *response, 
                        size_t *response_len) {
    // build HTTP header
    const char *mime_type = get_mime_type(file_ext);
    char *header = (char *)malloc(BUFFER_SIZE * sizeof(char));
    snprintf(header, BUFFER_SIZE,
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "\r\n",
             mime_type);

    // if file not exist, response is 404 Not Found
    int file_fd = open(file_name, O_RDONLY);
    if (file_fd == -1 || strcmp(file_name, waf_filename) == 0) {
        snprintf(response, BUFFER_SIZE,
                 "HTTP/1.1 404 Not Found\r\n"
                 "Content-Type: text/plain\r\n"
                 "\r\n"
                 "404 Not Found");
        *response_len = strlen(response);
        return;
    }

    // get file size for Content-Length
    struct stat file_stat;
    fstat(file_fd, &file_stat);
    off_t file_size = file_stat.st_size;

    // copy header to response buffer
    *response_len = 0;
    memcpy(response, header, strlen(header));
    *response_len += strlen(header);

    // copy file to response buffer
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, 
                            response + *response_len, 
                            BUFFER_SIZE - *response_len)) > 0) {
        *response_len += bytes_read;
    }
    free(header);
    close(file_fd);
}

void *handle_client(void *arg);

int main(int argc, char* argv[]){
	if (argc != 2) {
        	fprintf(stderr, "Usage: %s <IP_file>\n", argv[0]);
        	return EXIT_FAILURE;
    	}

	strcpy(waf_filename, argv[1]);

	// create a socket
	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	// define the address
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(8001);
	server_address.sin_addr.s_addr = INADDR_ANY;

	bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
	listen(server_socket, 5);

	printf("Listening...\n");
	while(1){
		// define client info
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		int *client_socket = malloc(sizeof(int));

		// accept and get connection data
		*client_socket = accept(server_socket, (struct sockaddr*) &client_addr, &client_addr_len);
		struct in_addr ipAddr = client_addr.sin_addr;
		char client_ip[INET_ADDRSTRLEN];
		inet_ntop( AF_INET, &ipAddr, client_ip, INET_ADDRSTRLEN );
		printf("This guy connected: %s\n", client_ip);
		bool forbid_IP = true;

		int fd = open(argv[1], O_RDONLY);
    		if (fd == -1) {
        		perror("open");
        		return EXIT_FAILURE;
    		}

    		char buf[4096];
    		char token[256];
    		size_t tlen = 0;

    		for (;;) {
        		ssize_t n = read(fd, buf, sizeof(buf));
        		if (n == 0) break; // EOF
        		if (n < 0) {
            			if (errno == EINTR) continue;
            			perror("read");
            			close(fd);
            			return EXIT_FAILURE;
        		}

		for (ssize_t i = 0; i < n; ++i) {
            		unsigned char c = (unsigned char)buf[i];
            		if (isspace(c)) {
                		if (tlen > 0) {
                    			token[tlen] = '\0';
                    			if (strcmp(token, client_ip) == 0) {
						printf("Got IP\n");
						forbid_IP = false;
                        			break;
                    			}
                    			tlen = 0;
                		}
            		} else {
                		if (tlen + 1 < 256) {
                    			token[tlen++] = (char)c;
                		}
            		}
		}
    }

    		// Handle last token if file doesn't end with whitespace
    		if (tlen > 0) {
        		token[tlen] = '\0';
        		if (strcmp(token, client_ip) == 0) {
            			printf("Got IP\n");
				forbid_IP = false;
        		}
    		}
		close(fd);

		// create a new thread to handle client data
		if(!forbid_IP) {
			pthread_t thread_id;
			pthread_create(&thread_id, NULL, handle_client, (void*) client_socket);
			pthread_detach(thread_id);
		}
	}

	close(server_socket);

	return 0;
}

void *handle_client(void *arg){
	int client_fd = *((int *)arg);
    	char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));

    	// receive request data from client and store into buffer
    	ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
    	if (bytes_received > 0) {
        	// check if request is GET
        	regex_t regex;
        	regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
        	regmatch_t matches[2];

        if (regexec(&regex, buffer, 2, matches, 0) == 0) {
            	// extract filename from request and decode URL
            	buffer[matches[1].rm_eo] = '\0';
            	const char *url_encoded_file_name = buffer + matches[1].rm_so;
            	char *file_name = url_decode(url_encoded_file_name);

            	// get file extension
            	char file_ext[32];
            	strcpy(file_ext, get_file_extension(file_name));

            	// build HTTP response
            	char *response = (char *)malloc(BUFFER_SIZE * 2 * sizeof(char));
            	size_t response_len;
            	build_http_response(file_name, file_ext, response, &response_len);

            	// send HTTP response to client
            	send(client_fd, response, response_len, 0);

            	free(response);
            	free(file_name);
        }
        regfree(&regex);
    }
    	close(client_fd);
    	free(arg);
    	free(buffer);
    	return NULL;
}
