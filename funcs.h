#ifndef FUNCS_H
#define FUNCS_H

#include <stdio.h>
#include <sys/stat.h>

#include "funcdecl.h"
#include "req.h"
#include "resolvfunc.h"
#include "strings.h"

#define HEADERS_1 "HTTP/1.1 200 OK\r\nContent-Type: "
#define HEADERS_2 "\r\n\r\n"

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
	if(! strncmp(ctx->req.buff+ctx->req.path.offset, arg.str, ctx->req.path.length)) {
		ctx->chain = arg.jump;
		ctx->index = 0;
	}
}

MK_SERVE_HEADERS(serve_headers_html, "text/html")
MK_SERVE_HEADERS(serve_headers_plaintext, "text/plain")
MK_SERVE_HEADERS(serve_headers_png, "image/png")
MK_SERVE_HEADERS(serve_headers_jpg, "image/jpeg")
MK_SERVE_HEADERS(serve_headers_custom, arg.str)

void serve_file(struct arg arg, struct ResolvCtx *ctx) {
	FILE *f;
	char buff[FILE_SERVE_BUFF_SIZE];
	struct stat st;
	size_t re;
	unsigned int remaining, wr, size;
	char *ptr;

	f = fopen(arg.str, "rb");
	if(!f) {
		fprintf(stderr, "file: %s\n", arg.str);
		perror("fopen failed @ serve_file");
		return;
	}

	if(fstat(fileno(f), &st) < 0) {
		fprintf(stderr, "file: %s\n", arg.str);
		perror("fstat failed @ serve_file");
		goto cleanup;
	}

	remaining = st.st_size;
	while(remaining > 0) {
		size = remaining < FILE_SERVE_BUFF_SIZE ? remaining : FILE_SERVE_BUFF_SIZE;

		re = fread(buff, 1, size, f);
		if(re <= 0) {
			fprintf(stderr, "file: %s\n", arg.str);
			perror("fread error @ serve_file");
			break;
		}

		ptr = buff;
		while(re > 0) {
			wr = write(ctx->req.client_fd, ptr, re);
			if(wr <= 0) {
				if (errno == EINTR) continue;
				fprintf(stderr, "file: %s\n", arg.str);
				perror("write to client failed @ serve_file");
				goto cleanup;
			}
			ptr += wr;
			re -= wr;
		}

		remaining -= ptr - buff;
	}

	cleanup:
	fclose(f);
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

#endif

