#ifndef COOKIES_H
#define COOKIES_H

#include <stdlib.h>

#include "buffpart.h"

struct Cookie {
	struct BuffPart name;
	struct BuffPart value;
	struct Cookie *next;
};

struct Cookies {
	struct Cookie *head;
	char *buff;
};

int cookies_contain(struct Cookies *c, char *name);
struct Cookie *get_cookie(struct Cookies *c, char *name);
void make_cookies(struct Cookies *c, char *buff);
void free_cookies(struct Cookies *c);
void set_cookie(struct Cookies *c, struct BuffPart name, struct BuffPart value);

static void free_cookie(struct Cookie *c);

int cookies_contain(struct Cookies *c, char *name) {
	struct Cookie *i;

	i = c->head;
	while(i) {
		if(bp_equ_str(c->buff, i->name, name)) return 1;
		i = i->next;
	}
	return 0;
}

struct Cookie *get_cookie(struct Cookies *c, char *name) {
	struct Cookie *i;

	i = c->head;
	while(i) {
		if(bp_equ_str(c->buff, i->name, name)) return i;
		i = i->next;
	}
	return NULL;
}

void make_cookies(struct Cookies *c, char *buff) {
	c->head = NULL;
	c->buff = buff;
}

static void free_cookie(struct Cookie *c) {
	if(c->next) free_cookie(c->next);
	free(c);
}

void free_cookies(struct Cookies *c) {
	if(c->head) free_cookie(c->head);
}

void set_cookie(struct Cookies *c, struct BuffPart name, struct BuffPart value) {
	struct Cookie *i;

	i = c->head;
	while(i) {
		if(bp_equ_bp(c->buff, i->name, name)) {
			i->value = value;
			return;
		}
		if(! i->next) {
			i->next = malloc(sizeof(struct Cookie));
			i->next->name = name;
			i->next->value = value;
			i->next->next = NULL;
			return;
		}
		i = i->next;
	};
	c->head = malloc(sizeof(struct Cookie));
	c->head->name = name;
	c->head->value = value;
	c->head->next = NULL;
}

#endif

