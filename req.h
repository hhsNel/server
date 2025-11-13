#ifndef REQ_H
#define REQ_H

#include "buffpart.h"
#include "headers.h"
#include "cookies.h"

enum HttpMethod {
	GET,
	HEAD,
	POST,
	PUT,
	DELETE,
	CONNECT,
	OPTIONS,
	TRACE,
	PATCH,
};

struct HttpRequest {
	char *buff;
	size_t buff_len;
	enum HttpMethod method;
	struct BuffPart path;
	char http_maj;
	char http_min;
	struct Headers headers;
	struct Cookies cookies;
	int client_fd;
};

#endif

