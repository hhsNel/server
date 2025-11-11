#ifndef STATS_H
#define STATS_H

#if STATS

#include <time.h>
#include <inttypes.h>
#include <stdio.h>
#include <signal.h>

#include "req.h"

#define STATS_START_CONNECTION(req) stats_start_connection(req)
#define STATS_END_CONNECTION(req) stats_end_connection(req)
#define STATS_INIT() stats_init()
#define STATS_WRITE() stats_write()

struct stats {
	unsigned int connections;
	unsigned long long int service_ns;
};
static struct stats stats;
static unsigned long long int start_ns;

void stats_start_connection(struct HttpRequest *req);
void stats_end_connection(struct HttpRequest *req);
void stats_init();
void stats_write();

static unsigned long long int ns_now();
static void exit_write_stats(int signo);

static unsigned long long int ns_now() {
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (unsigned long long int)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

void stats_start_connection(struct HttpRequest *req) {
	start_ns = ns_now();
}

void stats_end_connection(struct HttpRequest *req) {
	stats.service_ns += ns_now() - start_ns;
	++stats.connections;
}

static void exit_write_stats(int signo) {
	stats_write();
	exit(0);
}

void stats_init() {
	struct sigaction sa;

	stats.connections = 0;
	stats.service_ns = 0;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = exit_write_stats;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT,  &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	printf("Stats initialized\n");
}

void stats_write() {
	FILE *file;

	file = fopen(STATS_FILE, "w");
	if(! file) {
		perror("fopen failed @ stats_write");
		return;
	}

	fprintf(file,
			"total connections: %u\n"
			"service time: %llu\n"
			"average service time per connection: %llu\n"
			"average service time per connection [ms]: %.4F\n",
			stats.connections,
			stats.service_ns,
			stats.connections ? stats.service_ns / stats.connections : 0,
			stats.connections ? (double)stats.service_ns / (double)stats.connections / 1e6F : 0.0F);
	
	fclose(file);
	printf("Stats written to %s\n", STATS_FILE);
}

#else

#define STATS_SAVE_CONNECTION(req)
#define STATS_END_CONNECTION(req)
#define STATS_INIT()
#define STATS_WRITE()

#endif

#endif

