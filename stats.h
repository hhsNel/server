#ifndef STATS_H
#define STATS_H

#if STATS

#include <stdio.h>

#include "buffpart.h"
#include "req.h"

#define STATS_START_CONNECTION(req) stats_start_connection(req)
#define STATS_END_CONNECTION(req) stats_end_connection(req)
#define STATS_UPDATE_BUFF(buff) stats_update_buff(buff)
#define STATS_INIT() stats_init()
#define STATS_WRITE() stats_write()

struct perpath_stats {
	char *path;
	unsigned int connections;
	unsigned long long int path_ns;
	struct perpath_stats *next;
};

struct stats {
	char *buff;
	unsigned int connections;
	unsigned long long int service_ns;
	struct perpath_stats *head;
};
static struct stats stats;
static unsigned long long int start_ns;
static char stats_filename[sizeof(STATS_FILE) + TIME_SIZE - 1];

struct perpath_stats *get_perpath(struct BuffPart path);

void stats_start_connection(struct HttpRequest *req);
void stats_end_connection(struct HttpRequest *req);
void stats_update_buff(char *buff);
void stats_init();
void stats_write();

static unsigned long long int ns_now();

struct perpath_stats *get_perpath(struct BuffPart path) {
	struct perpath_stats *i;

	i = stats.head;
	while(i) {
		if(bp_equ_str(stats.buff, path, i->path)) {
			return i;
		}
		i = i->next;
	}

	i = malloc(sizeof(struct perpath_stats));
	if(! i) {
		perror("could not malloc perpath_stats");
		return NULL;
	}
	i->next = stats.head;
	i->path = malloc(path.length + 1);
	strncpy(i->path, stats.buff+path.offset, path.length);
	i->path[path.length] = '\0';
	i->connections = 0;
	i->path_ns = 0;
	stats.head = i;
	return i;
}

void stats_start_connection(struct HttpRequest *req) {
	start_ns = ns_now();
}

void stats_end_connection(struct HttpRequest *req) {
	struct perpath_stats *perpath;
	unsigned long long int now;

	now = ns_now();

	stats.service_ns += now - start_ns;
	++stats.connections;

	if((perpath = get_perpath(req->path))) {
		perpath->path_ns += now - start_ns;
		++perpath->connections;
	}

	if(stats.connections % STATS_WRITE_PERIOD == 0) {
		stats_write();
	}
}

void stats_update_buff(char *buff) {
	stats.buff = buff;
}

void stats_init() {
	stats.connections = 0;
	stats.service_ns = 0;
	stats.head = NULL;

	if(atexit(stats_write)) {
		fprintf(stderr, "Could not atexit stats_write @ stats_init\n");
	}

	snprintf(stats_filename, sizeof(stats_filename), STATS_FILE, start_time);

	printf("Stats initialized for time %s, filename: %s\n", start_time, stats_filename);
}

void stats_write() {
	FILE *file;
	struct perpath_stats *i;

	file = fopen(stats_filename, "w");
	if(! file) {
		perror("fopen failed @ stats_write");
		return;
	}

	fprintf(file,
			"total connections: %u\n"
			"service time: %llu\n"
			"average service time per connection: %llu\n"
			"average service time per connection [ms]: %F\n",
			stats.connections,
			stats.service_ns,
			stats.connections ? stats.service_ns / stats.connections : 0,
			stats.connections ? (double)stats.service_ns / (double)stats.connections / 1e6F : 0.0F);

	i = stats.head;
	while(i) {
		fprintf(file,
				"\tin path: %s\n"
				"\t\tconnections: %u\n"
				"\t\tpath time: %llu\n"
				"\t\taverage path time per connection: %llu\n"
				"\t\taverage path time per connection [ms]: %F\n",
				i->path,
				i->connections,
				i->path_ns,
				i->connections ? i->path_ns / i->connections : 0,
				i->connections ? (double)i->path_ns / (double)i->connections / 1e6F : 0.0F);
		i = i->next;
	}
	
	fclose(file);
	printf("Stats written to %s\n", stats_filename);
}

static unsigned long long int ns_now() {
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (unsigned long long int)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

#else

#define STATS_START_CONNECTION(req)
#define STATS_END_CONNECTION(req)
#define STATS_UPDATE_BUFF(buff)
#define STATS_INIT()
#define STATS_WRITE()

#endif

#endif

