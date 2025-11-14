#ifndef QUERY_H
#define QUERY_H

#include <stdlib.h>
#include <string.h>

#include "buffpart.h"

struct QueryParam {
	char *name;
	char *value;
	struct QueryParam *next;
};

struct QueryParams {
	struct QueryParam *head;
	char *buff;
};

int query_params_contain(struct QueryParams *qp, char *name);
struct QueryParam *get_query_param(struct QueryParams *qp, char *name);
void make_query_params(struct QueryParams *qp, char *buff);
void free_query_params(struct QueryParams *qp);
void set_query_param(struct QueryParams *qp, struct BuffPart name, struct BuffPart value);

static void free_query_param(struct QueryParam *qp);

int query_params_contain(struct QueryParams *qp, char *name) {
	struct QueryParam *i;

	i = qp->head;
	while(i) {
		if(! strcmp(i->name, name)) return 1;
		i = i->next;
	}
	return 0;
}

struct QueryParam *get_query_param(struct QueryParams *qp, char *name) {
	struct QueryParam *i;

	i = qp->head;
	while(i) {
		if(! strcmp(i->name, name)) return i;
		i = i->next;
	}
	return NULL;
}

void make_query_params(struct QueryParams *qp, char *buff) {
	qp->head = NULL;
	qp->buff = buff;
}

static void free_query_param(struct QueryParam *qp) {
	if(qp->next) free_query_param(qp->next);
	free(qp->name);
	free(qp->value);
	free(qp);
}

void free_query_params(struct QueryParams *qp) {
	if(qp->head) free_query_param(qp->head);
}

void set_query_param(struct QueryParams *qp, struct BuffPart name, struct BuffPart value) {
	struct QueryParam *i;

	i = qp->head;
	while(i) {
		if(bp_equ_str(qp->buff, name, i->name)) {
			free(i->value);
			i->value = strndup(qp->buff + value.offset, value.length);
			if(! i->value) {
				perror("strndup failed, deallocating the whole qp @ set_query_param");
				free(i->name);
				i->name = NULL;
			}
			return;
		}
		if(! i->next) {
			i->next = malloc(sizeof(struct QueryParam));
			if(! i->next) {
				perror("malloc failed @ set_query_param");
				return;
			}
			i->next->name = malloc(name.length + 1);
			if(! i->next->name) {
				perror("malloc failed @ set_query_param");
				free(i->next);
				i->next = NULL;
				return;
			}
			i->next->value = malloc(value.length + 1);
			if(! i->next->value) {
				perror("malloc failed @ set_query_param");
				free(i->next->name);
				free(i->next);
				i->next = NULL;
				return;
			}
			strncpy(i->next->name, qp->buff + name.offset, name.length);
			i->next->name[name.length] = '\0';
			strncpy(i->next->value, qp->buff + value.offset, value.length);
			i->next->value[value.length] = '\0';
			i->next->next = NULL;
			return;
		}
		i = i->next;
	};

	qp->head = malloc(sizeof(struct QueryParam));
	if(! qp->head) {
		perror("malloc failed @ set_query_param");
		return;
	}
	qp->head->name = malloc(name.length + 1);
	if(! qp->head->name) {
		perror("malloc failed @ set_query_param");
		free(qp->head);
		qp->head = NULL;
		return;
	}
	qp->head->value = malloc(value.length + 1);
	if(! qp->head->value) {
		perror("malloc failed @ set_query_param");
		free(qp->head->name);
		free(qp->head);
		qp->head = NULL;
		return;
	}
	strncpy(qp->head->name, qp->buff + name.offset, name.length);
	qp->head->name[name.length] = '\0';
	strncpy(qp->head->value, qp->buff + value.offset, value.length);
	qp->head->value[value.length] = '\0';
	qp->head->next = NULL;
}

#endif

