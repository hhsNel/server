#ifndef HEADERS_H
#define HEADERS_H

#include <stdlib.h>

#include "buffpart.h"

struct Header {
	struct BuffPart name;
	struct BuffPart value;
	struct Header *next;
};

struct Headers {
	struct Header *head;
	char *buff;
};

int headers_contain(struct Headers *h, char *name);
struct Header *get_header(struct Headers *h, char *name);
void make_headers(struct Headers *h, char *buff);
void free_headers(struct Headers *h);
void set_header(struct Headers *h, struct BuffPart name, struct BuffPart value);

static void free_header(struct Header *h);

int headers_contain(struct Headers *h, char *name) {
	struct Header *i;

	i = h->head;
	while(i) {
		if(bp_equ_str(h->buff, i->name, name)) return 1;
		i = i->next;
	}
	return 0;
}

struct Header *get_header(struct Headers *h, char *name) {
	struct Header *i;

	i = h->head;
	while(i) {
		if(bp_equ_str(h->buff, i->name, name)) return i;
		i = i->next;
	}
	return NULL;
}

void make_headers(struct Headers *h, char *buff) {
	h->head = NULL;
	h->buff = buff;
}

static void free_header(struct Header *h) {
	if(h->next) free_header(h->next);
	free(h);
}

void free_headers(struct Headers *h) {
	if(h->head) free_header(h->head);
}

void set_header(struct Headers *h, struct BuffPart name, struct BuffPart value) {
	struct Header *i;

	i = h->head;
	while(i) {
		if(bp_equ_bp(h->buff, i->name, name)) {
			i->value = value;
			return;
		}
		if(! i->next) {
			i->next = malloc(sizeof(struct Header));
			i->next->name = name;
			i->next->value = value;
			i->next->next = NULL;
			return;
		}
		i = i->next;
	};
	h->head = malloc(sizeof(struct Header));
	h->head->name = name;
	h->head->value = value;
	h->head->next = NULL;
}

#endif

