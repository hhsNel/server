#ifndef FUNCS_H
#define FUNCS_H

#include <stdio.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>

#include "buffpart.h"
#include "funcdecl.h"
#include "req.h"
#include "resolvfunc.h"
#include "strings.h"

#define HEADERS_1 "HTTP/1.1 200 OK\r\nContent-Type: "
#define HEADERS_2 "\r\n\r\n"
#define RES_LINE_1 "HTTP/1.1 "
#define RES_LINE_2 "\r\n"

#define MK_SERVE_HEADERS(NAME, MIMETYPE) \
	void NAME(struct arg arg, struct ResolvCtx *ctx) { \
		write(ctx->req.client_fd, HEADERS_1, strlen(HEADERS_1)); \
		write(ctx->req.client_fd, MIMETYPE, strlen(MIMETYPE)); \
		write(ctx->req.client_fd, HEADERS_2, strlen(HEADERS_2)); \
	}

void if_path_begins(struct arg arg, struct ResolvCtx *ctx) {
	if(str_n_starts_with(ctx->req.buff+ctx->req.path.offset, arg.str, ctx->req.path.length)) {
		ctx->chain = arg.jump;
		ctx->index = 0;
	}
}

void if_is_path(struct arg arg, struct ResolvCtx *ctx) {
	if(bp_equ_str(ctx->req.buff, ctx->req.path, arg.str)) {
		ctx->chain = arg.jump;
		ctx->index = 0;
	}
}

void if_is_path_no_query(struct arg arg, struct ResolvCtx *ctx) {
	struct BuffPart path_no_query;
	char *query_start;

	path_no_query.offset = ctx->req.path.offset;
	path_no_query.length = ctx->req.path.length;
	query_start = memchr(ctx->req.buff + path_no_query.offset, '?', path_no_query.length);
	if(query_start) {
		path_no_query.length = query_start - ctx->req.buff - path_no_query.offset;
	}
	if(bp_equ_str(ctx->req.buff, path_no_query, arg.str)) {
		ctx->chain = arg.jump;
		ctx->index = 0;
	}
}

MK_SERVE_HEADERS(serve_headers_html, "text/html")
MK_SERVE_HEADERS(serve_headers_plaintext, "text/plain")
MK_SERVE_HEADERS(serve_headers_png, "image/png")
MK_SERVE_HEADERS(serve_headers_jpg, "image/jpeg")
MK_SERVE_HEADERS(serve_headers_css, "text/css")
MK_SERVE_HEADERS(serve_headers_custom, arg.str)

void serve_file(struct arg arg, struct ResolvCtx *ctx) {
	int file;
	struct stat st;
	off_t offset;
	ssize_t sent;

	file = open(arg.str, O_RDONLY);
	if(!file) {
		fprintf(stderr, "file: %s\n", arg.str);
		perror("open failed @ serve_file");
		return;
	}

	if(fstat(file, &st) < 0) {
		fprintf(stderr, "file: %s\n", arg.str);
		perror("fstat failed @ serve_file");
		goto cleanup;
	}

	offset = 0;

	while(offset < st.st_size) {
		sent = sendfile(ctx->req.client_fd, file, &offset, st.st_size - offset);
		if(sent <= 0) {
			if(errno == EINTR) continue;
			perror("sendfile failed @ serve_file");
			goto cleanup;
		}
	}

	cleanup:
	close(file);
}

void serve_exec_shell(struct arg arg, struct ResolvCtx *ctx) {
	FILE *f;
	char buff[FILE_SERVE_BUFF_SIZE];
	size_t re;
	char *p;
	int wr, rd;

	f = popen(arg.str, "r");
	if (!f) {
		fprintf(stderr, "commang: %s\n", arg.str);
		perror("popen failed @ serve_exec_shell");
		return;
	}

	while ((re = fread(buff, 1, FILE_SERVE_BUFF_SIZE, f)) > 0) {
		p = buff;
		rd = re;

		while (rd > 0) {
			wr = write(ctx->req.client_fd, p, rd);
			if (wr <= 0) {
				if (errno == EINTR) continue;
				fprintf(stderr, "commang: %s\n", arg.str);
				perror("write to client failed @ serve_file");
				pclose(f);
				return;
			}
			p += wr;
			rd -= wr;
		}
	}

	if (ferror(f)) {
		fprintf(stderr, "commang: %s\n", arg.str);
		perror("fread error from command output @ serve_file");
	}

	if (pclose(f) == -1) {
		fprintf(stderr, "commang: %s\n", arg.str);
		perror("pclose failed @ serve_file");
	}
}

/* TODO */
/* void pass_all_to_local_port(struct arg arg, struct ResolvCtx *ctx); */

void serve_plaintext(struct arg arg, struct ResolvCtx *ctx) {
	unsigned int i;
	int written;

	i = 0;
	while((written = write(ctx->req.client_fd, arg.str + i, strlen(arg.str) - i))) {
		if(written <= 0 && errno != EINTR) {
			break;
		}

		i += written;
	}
	if(written < 0) {
		perror("couldn't write @ serve_plaintext");
		return;
	}
}

void serve_header_value(struct arg arg, struct ResolvCtx *ctx) {
	struct Header *header;

	if(headers_contain(&ctx->req.headers, arg.str)) {
		header = get_header(&ctx->req.headers, arg.str);

		write(ctx->req.client_fd, ctx->req.buff + header->value.offset, header->value.length);
	}
}

void serve_cookie_value(struct arg arg, struct ResolvCtx *ctx) {
	struct Cookie *cookie;

	if(! cookies_contain(&ctx->req.cookies, arg.str)) {
		return;
	}

	cookie = get_cookie(&ctx->req.cookies, arg.str);
	write(ctx->req.client_fd, ctx->req.buff + cookie->value.offset, cookie->value.length);
}

void serve_http_status(struct arg arg, struct ResolvCtx *ctx) { \
	write(ctx->req.client_fd, RES_LINE_1, strlen(RES_LINE_1)); \
	write(ctx->req.client_fd, arg.str, strlen(arg.str)); \
	write(ctx->req.client_fd, RES_LINE_2, strlen(RES_LINE_2)); \
}

void serve_query_param(struct arg arg, struct ResolvCtx *ctx) {
	struct QueryParam *qp;

	if(! query_params_contain(&ctx->req.query, arg.str)) {
		return;
	}

	qp = get_query_param(&ctx->req.query, arg.str);
	write(ctx->req.client_fd, qp->value, strlen(qp->value));
}

#endif

