#ifndef FUNCDECL_H
#define FUNCDECL_H

#include "req.h"
#include "resolvfunc.h"

void if_path_begins(struct arg, struct ResolvCtx *ctx);
void if_is_path(struct arg, struct ResolvCtx *ctx);
void serve_headers_html(struct arg, struct ResolvCtx *ctx);
void serve_headers_plaintext(struct arg, struct ResolvCtx *ctx);
void serve_headers_png(struct arg, struct ResolvCtx *ctx);
void serve_headers_jpg(struct arg, struct ResolvCtx *ctx);
void serve_headers_custom(struct arg, struct ResolvCtx *ctx);
void serve_file(struct arg, struct ResolvCtx *ctx);
void serve_exec_shell(struct arg, struct ResolvCtx *ctx);
/* void pass_all_to_local_port(struct arg, struct ResolvCtx *ctx); */


#endif

