#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>

#include "strings.h"
#include "req.h"
#include "http.h"

#define BUFF_GRAN 1024
#define READ_MAX 256
#define MAX_HTTP_HEADER_SIZE 16*1024

const char response[] =
	"HTTP/1.1 200 OK\r\n"
	"Content-Type: text/html\r\n"
	"\r\n"
	"<h1>Lemme Cook!</h1>\r\n";

int main(/*int argc, char **argv*/) {
	int server_fd;
	struct sockaddr_in addr;
	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len;
	long int bytes_read;
	char *buff;
	size_t buff_len, buff_cap;
	int determined_is_http, read_headers;
	struct HttpRequest req;

	                /* ipv4     TCP */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);
	addr.sin_addr.s_addr = INADDR_ANY;

	client_len = sizeof(client_addr);

	buff = malloc(BUFF_GRAN);
	if(!buff) {
		perror("malloc failed");
		exit(1);
	}
	buff_len = buff_cap = 0;

	if(bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind failed");
		exit(1);
	}
	                  /* num of pending cons */
	if(listen(server_fd, 4) < 0) {
		perror("listen failed");
		exit(1);
	}

	printf("Should be listening on ipv4 TCP port 8080\n");

	while(1) {
		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
		if(client_fd < 0) {
			perror("accept failed");
			continue;
		}
		determined_is_http = read_headers = 0;
		buff_len = 0;

		while((bytes_read = read(client_fd, buff + buff_len, READ_MAX-1)) > 0) {
			buff_len += bytes_read;
			buff[buff_len] = '\0';

			/* if \r\n\r\n was sent, don't read anymore */
			/* TODO */
			if(str_contains(buff, "\r\n\r\n") && determined_is_http) {
				break;
			}

			if(!determined_is_http) {
				if(buff_len + READ_MAX > MAX_HTTP_HEADER_SIZE) {
					/* 431 request header fields too massive */
					goto cleanup;
				}
				if(fill_out_http_req_line(buff, buff_len, &req)) {
					determined_is_http = 1;
				}
			}

			/* do something maybe ??? */

			if(buff_len + READ_MAX > buff_cap) {
				buff_cap += BUFF_GRAN;
				buff = realloc(buff, buff_cap);
			}
		}
		if(bytes_read < 0) {
			perror("read failed");
			continue;
		}

		printf("received: %.*s\n", (int)buff_len, buff);

		write(client_fd, response, strlen(response));

		cleanup:
		close(client_fd);
	};
};

