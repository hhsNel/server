#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>

#include "resolvfunc.h"
#include "funcdecl.h"

#include "config.h"

#if USE_UNIX_SOCKET
#include <sys/socket.h>
#include <sys/un.h>
#endif

#define TIME_SIZE 20 + 8 /* time string should fit in 20 chars, +8 for safety */
time_t server_start_time;
char start_time[TIME_SIZE];

#include "funcs.h"
#include "strings.h"
#include "req.h"
#include "stats.h"
#include "logging.h"
#include "http.h"

static int handle_request(char **buff, int client_fd, size_t *buff_len, size_t *buff_cap, int *determined_is_http, int *read_headers, struct HttpRequest *req);
static void exit_handler(int signo);
static void init_exit_handler();

static int server_fd;

int main(int argc, char **argv) {
	struct timeval tv;
	int opt;
#if USE_UNIX_SOCKET
	struct sockaddr_un addr;
#else
	struct sockaddr_in addr;
#endif
	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len;
	char *buff;
	size_t buff_len, buff_cap;
	int determined_is_http, read_headers;
	struct HttpRequest req;
	struct Header *header;
	struct ResolvCtx ctx;
	struct tm tm_info;

#if USE_UNIX_SOCKET
	                /* unix     TCP */
	server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
#else
	                /* ipv4     TCP */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
#endif
	if(server_fd < 0) {
		perror("socket failed");
		exit(1);
	}
	tv.tv_sec = SLOW_LORIS_TIMEOUT;
	tv.tv_usec = 0;
	opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

#if USE_UNIX_SOCKET
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, UNIX_SOCK_PATH);
#else
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
#endif

	client_len = sizeof(client_addr);

	buff = malloc(BUFF_GRAN);
	if(!buff) {
		perror("malloc failed");
		exit(1);
	}
	buff_cap = BUFF_GRAN;
	buff_len = req.buff_len = 0;
	req.headers.buff = req.buff = buff;
	STATS_UPDATE_BUFF(buff);
	LOGS_UPDATE_BUFF(buff);

	if(bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind failed");
		exit(1);
	}
	                  /* num of pending cons */
	if(listen(server_fd, backlog) < 0) {
		perror("listen failed");
		exit(1);
	}

#if USE_UNIX_SOCKET
	printf("Should be listening on unix socket %s\n", UNIX_SOCK_PATH);
#else
	printf("Should be listening on ipv4 TCP port %d\n", port);
#endif

	server_start_time = time(NULL);
	localtime_r(&server_start_time, &tm_info);
	strftime(start_time, sizeof(start_time), "%m_%d_%y-%I_%M_%S%p", &tm_info);


	init_exit_handler();

	STATS_INIT();
	LOGS_INIT();

	while(1) {
		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
		if(client_fd < 0) {
			perror("accept failed");
			continue;
		}
		setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
		determined_is_http = read_headers = 0;
		req.buff_len = buff_len = 0;
		make_headers(&req.headers, buff);
		req.client_fd = client_fd;

		STATS_START_CONNECTION(&req);
		LOGS_START_CONNECTION(&req);

		if(handle_request(&buff, client_fd, &buff_len, &buff_cap, &determined_is_http, &read_headers, &req) || !determined_is_http || !read_headers) {
			perror("handle request returned 1 or failed");
			goto cleanup;
		}

		printf("HTTP Request found!\n\tMETHOD: %d\n\tPATH: %.*s\n\tVER: %c.%c\n", req.method, (int)req.path.length, req.buff+req.path.offset, req.http_maj, req.http_min);
		header = req.headers.head;
		while(header) {
			printf("\tHeader read:\n\t\tName:\t\t%.*s\n\t\tValue:\t\t%.*s\n", (int)header->name.length, buff+header->name.offset, (int)header->value.length, buff+header->value.offset);
			header = header->next;
		};

		ctx.chain = init;
		ctx.index = 0;
		ctx.req = req;
		exec_chain(ctx);

		STATS_END_CONNECTION(&req);
		LOGS_END_CONNECTION(&req);

		cleanup:
		free_headers(&req.headers);
		close(client_fd);
	};
};

int handle_request(char **buff, int client_fd, size_t *buff_len, size_t *buff_cap, int *determined_is_http, int *read_headers, struct HttpRequest *req) {
	long int bytes_read;

	while((bytes_read = read(client_fd, *buff + *buff_len, READ_MAX-1)) > 0) {
		*buff_len += bytes_read;
		req->buff_len = *buff_len;
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
			STATS_UPDATE_BUFF(*buff);
			LOGS_UPDATE_BUFF(*buff);
		}
	}

	if(bytes_read < 0) {
		perror("read failed");
		return 1;
	}

	return 0;
}

void exit_handler(int signo) {
#if USE_UNIX_SOCKET
	if (server_fd != -1) {
		close(server_fd);
	}
	unlink(UNIX_SOCK_PATH);
#else
#endif
	exit(0);
}

void init_exit_handler() {
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = exit_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT,  &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
}

