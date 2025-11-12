#ifndef LOGGING_H
#define LOGGING_H

#if LOGS

#include <time.h>
#include <stdio.h>
#include <signal.h>

#include "headers.h"
#include "buffpart.h"
#include "req.h"

#define LOGS_START_CONNECTION(req) logs_start_connection(req)
#define LOGS_END_CONNECTION(req) logs_end_connection(req)
#define LOGS_UPDATE_BUFF(buff) logs_update_buff(buff)
#define LOGS_INIT() logs_init()
#define LOGS_WRITE() logs_write()

struct xff_log {
	char *xff;
	unsigned int connections;
	char **dumps;
	size_t *lengths;
	unsigned int len, cap;
	struct xff_log *next;
};

struct logs {
	char *buff;
	unsigned int num_sus, num_normal;
	struct xff_log *head;
};
static struct logs logs;
static char logs_filename[sizeof(LOGS_FILE) + TIME_SIZE - 1];

void logs_start_connection(struct HttpRequest *req);
void logs_end_connection(struct HttpRequest *req);
void logs_update_buff(char *buff);
void logs_init();
void logs_write();

static struct xff_log *get_xff_log(struct Header xff);
static int xff_log_exists(struct Header xff);
static int is_sus(struct HttpRequest *req);

void logs_start_connection(struct HttpRequest *req) {
}

void logs_end_connection(struct HttpRequest *req) {
	struct xff_log *xff_log;
	struct Header *xff_header;
	char **dumps;
	size_t *lengths;

	xff_header = get_header(&req->headers, "X-Forwarded-For");
	if(! xff_header) {
		fprintf(stderr, "No XFF header @ logs_end_connection\n");
		return;
	}

	if(is_sus(req)) {
		++logs.num_sus;
	} else {
		++logs.num_normal;
	}

	if((LOG_DUMP_IF(*req, *xff_header)) && (xff_log = get_xff_log(*xff_header))) {
		if(xff_log->len == xff_log->cap) {
			xff_log->cap += XFF_LOG_ARR_GRAN;
			dumps = realloc(xff_log->dumps, sizeof(char *) * xff_log->cap);
			lengths = realloc(xff_log->lengths, sizeof(size_t) * xff_log->cap);
			if(! dumps || ! lengths) {
				fprintf(stderr, "Could not reallocate new dumps/lengths for xff log; len=%u cap=%u xff=\"%s\" @ logs_end_connection\n", xff_log->len, xff_log->cap, xff_log->xff);
				if(dumps) free(dumps);
				if(lengths) free(lengths);
				return;
			}
			xff_log->dumps = dumps;
			xff_log->lengths = lengths;
		}

		xff_log->lengths[xff_log->len] = req->buff_len + 1;
		xff_log->dumps[xff_log->len] = malloc(xff_log->lengths[xff_log->len]);
		if(! xff_log->dumps[xff_log->len]) {
			fprintf(stderr, "Could not allocate enough memory to store the whole request dump: %llu butes @ logs_end_connection\n", (unsigned long long int)xff_log->lengths[xff_log->len]);
			return;
		}
		memcpy(xff_log->dumps[xff_log->len], req->buff, xff_log->lengths[xff_log->len]);
		xff_log->dumps[xff_log->len] [xff_log->lengths[xff_log->len] - 1] = '\0';
		++xff_log->len;
		++xff_log->connections;
	}

	if((logs.num_sus + logs.num_normal) % LOGS_WRITE_PERIOD == 0) {
		logs_write();
	}
}

void logs_update_buff(char *buff) {
	logs.buff = buff;
}

void logs_init() {
	logs.num_sus = 0;
	logs.num_normal = 0;
	logs.head = NULL;

	if(atexit(logs_write)) {
		fprintf(stderr, "Could not atexit logs_write @ logs_init\n");
	}

	snprintf(logs_filename, sizeof(logs_filename), LOGS_FILE, start_time);

	printf("Logs initialized for time %s, logs_filename: %s\n", start_time, logs_filename);
}

void logs_write() {
	FILE *file;
	struct xff_log *it;
	unsigned int i;

	file = fopen(logs_filename, "w");
	if(! file) {
		perror("fopen failed @ logs_write");
		return;
	}

	fprintf(file,
			"number of normal connections: %u\n"
			"number of suspicious connections: %u\n",
			logs.num_normal,
			logs.num_sus);

	it = logs.head;
	while(it) {
		fprintf(file,
				"\txff: %s\n"
				"\tconnections: %u\n"
				"\tlen: %u\n"
				"\tcap: %u\n",
				it->xff,
				it->connections,
				it->len,
				it->cap);
		for(i = 0; i < it->len; ++i) {
			fprintf(file,
					"\t\tlength[%u]: %llu\n"
					"\t\tdumps[%u]: ",
					i,
					(unsigned long long int)it->lengths[i],
					i);
			fwrite(it->dumps[i], 1, it->lengths[i], file);
			fprintf(file, "\n");
		}
		it = it->next;
	}
	
	fclose(file);
	printf("Logs written to %s\n", logs_filename);
}

static struct xff_log *get_xff_log(struct Header xff) {
	struct xff_log *i;

	i = logs.head;
	while(i) {
		if(bp_equ_str(logs.buff, xff.value, i->xff)) {
			return i;
		}
		i = i->next;
	}

	i = malloc(sizeof(struct xff_log));
	if(! i) {
		perror("could not malloc xff_log");
		return NULL;
	}
	i->next = logs.head;
	i->xff = malloc(xff.value.length + 1);
	strncpy(i->xff, logs.buff+xff.value.offset, xff.value.length);
	i->xff[xff.value.length] = '\0';
	i->lengths = NULL;
	i->dumps = NULL;
	i->len = i->cap = 0;
	i->connections = 0;
	logs.head = i;
	return i;
}

static int xff_log_exists(struct Header xff) {
	struct xff_log *i;

	i = logs.head;
	while(i) {
		if(bp_equ_str(logs.buff, xff.value, i->xff)) {
			return 1;
		}
		i = i->next;
	}

	return 0;
}

static int is_sus(struct HttpRequest *req) {
#define SUS_CHECK(STR, ...) if(__VA_ARGS__) {printf(STR "\n"); return 1;};
	IS_SUS_IF(*req);
#undef SUS_CHECK
	return 0;
}

#else

#define LOGS_SAVE_CONNECTION(req)
#define LOGS_END_CONNECTION(req)
#define LOGS_UPDATE_BUFF(buff)
#define LOGS_INIT()
#define LOGS_WRITE()

#endif

#endif

