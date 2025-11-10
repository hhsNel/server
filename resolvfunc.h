#ifndef RESOLVFUNC_H
#define RESOLVFUNC_H

#include "req.h"

typedef struct ResolvFunc ResolvFunc;
struct ResolvCtx;

struct arg {
	int i;
	char *str;
	struct ResolvFunc *jump;
};

typedef struct ResolvFunc {
	void (*func)(struct arg, struct ResolvCtx *ctx);
	struct arg arg;
} ResolvFunc;

struct ResolvCtx {
	struct ResolvFunc *chain;
	unsigned int index;
	struct HttpRequest req;
};

void exec_chain(struct ResolvCtx ctx);

void exec_chain(struct ResolvCtx ctx) {
	unsigned int index;

	while(ctx.chain[ctx.index].func) {
		index = ctx.index;
		++ ctx.index;
		ctx.chain[index].func(ctx.chain[index].arg, &ctx);
	}
}

#endif

