#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>

#include "resolvfunc.h"
#include "funcdecl.h"

#include "config.h"

#include "funcs.h"
#include "strings.h"
#include "req.h"
#include "stats.h"
#include "http.h"

int handle_request(char **buff, int client_fd, size_t *buff_len, size_t *buff_cap, int *determined_is_http, int *read_headers, struct HttpRequest *req);

int main(int argc, char **argv) {
	int server_fd;
	struct timeval tv;
	int opt;
	struct sockaddr_in addr;
	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len;
	char *buff;
	size_t buff_len, buff_cap;
	int determined_is_http, read_headers;
	struct HttpRequest req;
	struct Header *header;
	struct ResolvCtx ctx;

	                /* ipv4     TCP */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd < 0) {
		perror("socket failed");
		exit(1);
	}
	tv.tv_sec = SLOW_LORIS_TIMEOUT;
	tv.tv_usec = 0;
	opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	client_len = sizeof(client_addr);

	buff = malloc(BUFF_GRAN);
	if(!buff) {
		perror("malloc failed");
		exit(1);
	}
	buff_len = buff_cap = BUFF_GRAN;
	req.headers.buff = req.buff = buff;
	stats_update_buff(buff);

	if(bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind failed");
		exit(1);
	}
	                  /* num of pending cons */
	if(listen(server_fd, backlog) < 0) {
		perror("listen failed");
		exit(1);
	}

	printf("Should be listening on ipv4 TCP port %d\n", port);

	STATS_INIT();

	while(1) {
		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
		if(client_fd < 0) {
			perror("accept failed");
			continue;
		}
		setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		determined_is_http = read_headers = 0;
		buff_len = 0;
		make_headers(&req.headers, buff);
		req.client_fd = client_fd;

		STATS_START_CONNECTION(&req);

		if(handle_request(&buff, client_fd, &buff_len, &buff_cap, &determined_is_http, &read_headers, &req) || !determined_is_http || !read_headers) {
			perror("handle request returned 1 or failed");
			goto cleanup;
		}

		printf("HTTP Request found!\n\tMETHOD: %d\n\tPATH: %.*s\n\tVER: %c.%c\n", req.method, (int)req.path.length, req.buff+req.path.offset, req.http_maj, req.http_min);
		header = req.headers.head;
		while(header) {
			printf("Header read:\n\tName:\t\t%.*s\n\tValue:\t\t%.*s\n", (int)header->name.length, buff+header->name.offset, (int)header->value.length, buff+header->value.offset);
			header = header->next;
		};

		ctx.chain = init;
		ctx.index = 0;
		ctx.req = req;
		exec_chain(ctx);

		STATS_END_CONNECTION(&req);

		cleanup:
		free_headers(&req.headers);
		close(client_fd);
	};
};

int handle_request(char **buff, int client_fd, size_t *buff_len, size_t *buff_cap, int *determined_is_http, int *read_headers, struct HttpRequest *req) {
	long int bytes_read;

	while((bytes_read = read(client_fd, *buff + *buff_len, READ_MAX-1)) > 0) {
		*buff_len += bytes_read;
		(*buff)[*buff_len] = '\0';


		if(!*determined_is_http) {
			if(*buff_len + READ_MAX > MAX_HTTP_REQ_LINE_SIZE) {
				/* 431 request header fields too massive */
				return 1;
			}
			if(fill_out_http_req_line(*buff, *buff_len, req)) {
				*determined_is_http = 1;
			}
		}

		if(*determined_is_http) {
			if(!*read_headers) {
				if(*buff_len + READ_MAX > MAX_HTTP_HEADERS_SIZE) {
					/* 431 request header fields too massive */
					return 1;
				}
				if(str_contains(*buff, "\r\n\r\n")) {
					if(!fill_out_headers(*buff, &req->headers)) {
						/* something with headers is fvcked up */
						return 1;
					}
					*read_headers = 1;
				}
			}
			if(*read_headers) {
				if(headers_contain(&req->headers, "Content-Length")) {
					printf("content-length, fvck!\n");
				}
				/* stop reading headers */
				break;
			}
		}


		/* do something maybe ??? */

		if(*buff_len + READ_MAX > *buff_cap) {
			*buff_cap += BUFF_GRAN;
			*buff = realloc(*buff, *buff_cap);
			if(! *buff) {
				perror("realloc failed");
				fprintf(stderr, "size: %lu\n", *buff_cap);
				exit(1);
			}
			req->headers.buff = req->buff = *buff;
			stats_update_buff(*buff);
		}
	}

	if(bytes_read < 0) {
		perror("read failed");
		return 1;
	}

	return 0;
}

