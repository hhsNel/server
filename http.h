#ifndef HTTP_H
#define HTTP_H

#include "strings.h"
#include "buffpart.h"
#include "req.h"
#include "headers.h"

int fill_out_http_req_line(char *buff, size_t buff_len, struct HttpRequest *req);
int fill_out_headers(char *buff, struct Headers *h);

static char *fill_out_header(char *begin, struct Headers *h);

int fill_out_http_req_line(char *buff, size_t buff_len, struct HttpRequest *req) {
	char *it;

	if(
		(!str_starts_with(buff, "GET") &&
		 !str_starts_with(buff, "HEAD") &&
		 !str_starts_with(buff, "POST") &&
		 !str_starts_with(buff, "PUT") &&
		 !str_starts_with(buff, "DELETE") &&
		 !str_starts_with(buff, "CONNECT") &&
		 !str_starts_with(buff, "OPTIONS") &&
		 !str_starts_with(buff, "TRACE") &&
		 !str_starts_with(buff, "PATCH")) ||
		!str_contains(buff, "\r\n")) {
		return 0;
	}

	req->buff = buff;

	if(str_starts_with(buff, "GET")) {
		req->method = GET;
		it = buff + 3;
	} else if(str_starts_with(buff, "HEAD")) {
		req->method = HEAD;
		it = buff + 4;
	} else if(str_starts_with(buff, "POST")) {
		req->method = POST;
		it = buff + 4;
	} else if(str_starts_with(buff, "PUT")) {
		req->method = PUT;
		it = buff + 3;
	} else if(str_starts_with(buff, "DELETE")) {
		req->method = DELETE;
		it = buff + 6;
	} else if(str_starts_with(buff, "CONNECT")) {
		req->method = CONNECT;
		it = buff + 7;
	} else if(str_starts_with(buff, "OPTIONS")) {
		req->method = OPTIONS;
		it = buff + 7;
	} else if(str_starts_with(buff, "TRACE")) {
		req->method = TRACE;
		it = buff + 5;
	} else {
		req->method = PATCH;
		it = buff + 5;
	}

	for(; isblank(*it); ++it);
	req->path.offset = it - buff;
	for(; !isblank(*it); ++it);
	req->path.length = it - buff - req->path.offset;
	for(; isblank(*it); ++it);

	if(!str_starts_with(it, "HTTP/") || buff +  buff_len < it + 7) {
		/* that's not http! */
		return 0;
	}
	req->http_maj = it[5];
	req->http_min = it[7];

	return 1;
}

int fill_out_headers(char *buff, struct Headers *h) {
	char *headers_str;

	headers_str = buff + str_index_of(buff, "\r\n") + 2;

	while(headers_str[0] != '\r' || headers_str[1] != '\n') {
		headers_str = fill_out_header(headers_str, h);
		if(!headers_str) return 0;
	}
	return 1;
}

static char *fill_out_header(char *begin, struct Headers *h) {
	struct BuffPart name, value;
	char *it;

	name.offset = begin - h->buff;
	it = strchr(begin, ':');
	if(! it) {
		return NULL;
	}
	name.length = it - begin;
	begin = it + 1;
	for(; isblank(*begin); ++begin);

	value.offset = begin - h->buff;
	value.length = str_index_of(begin, "\r\n");

	set_header(h, name, value);

	return begin + value.length + 2;
}

#endif

