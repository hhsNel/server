#ifndef REQ_H
#define REQ_H

#include "buffpart.h"

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
	enum HttpMethod method;
	struct BuffPart path;
	char http_maj;
	char http_min;
};

#endif

