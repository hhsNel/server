#ifndef STRINGS_H
#define STRINGS_H

#include <string.h>

int str_starts_with(char *str, char *start);
int str_contains(char *str, char *substr);
int str_index_of(char *str, char *substr);

int str_starts_with(char *str, char *start) {
	for(; *str && *str == *start; ++str, ++start);
	return ! *start;
};

int str_contains(char *str, char *substr) {
	for(; strlen(str) >= strlen(substr); ++str) {
		if(str_starts_with(str, substr)) {
			return 1;
		}
	}
	return 0;
}

int str_index_of(char *str, char *substr) {
	char *i;

	for(i = str; strlen(i) >= strlen(substr); ++i) {
		if(str_starts_with(i, substr)) {
			return i - str;
		}
	}
	return -1;
}

#endif

