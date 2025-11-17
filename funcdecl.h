#ifndef FUNCDECL_H
#define FUNCDECL_H

#include "req.h"
#include "resolvfunc.h"

void if_path_begins(struct arg, struct ResolvCtx *ctx);
void if_is_path(struct arg, struct ResolvCtx *ctx);
void if_is_path_no_query(struct arg, struct ResolvCtx *ctx);
void serve_headers_html(struct arg, struct ResolvCtx *ctx);
void serve_headers_plaintext(struct arg, struct ResolvCtx *ctx);
void serve_headers_png(struct arg, struct ResolvCtx *ctx);
void serve_headers_jpg(struct arg, struct ResolvCtx *ctx);
void serve_headers_css(struct arg, struct ResolvCtx *ctx);
void serve_headers_mpeg(struct arg, struct ResolvCtx *ctx);
void serve_headers_ogg(struct arg, struct ResolvCtx *ctx);
void serve_headers_custom(struct arg, struct ResolvCtx *ctx);
void serve_file(struct arg, struct ResolvCtx *ctx);
void serve_exec_shell(struct arg, struct ResolvCtx *ctx);
/* void pass_all_to_local_port(struct arg, struct ResolvCtx *ctx); */
void serve_plaintext(struct arg, struct ResolvCtx *ctx);
void serve_header_value(struct arg, struct ResolvCtx *ctx);
void serve_cookie_value(struct arg, struct ResolvCtx *ctx);
void serve_http_status(struct arg, struct ResolvCtx *ctx);
void serve_query_param(struct arg, struct ResolvCtx *ctx);

#endif

