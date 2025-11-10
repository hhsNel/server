#ifndef BUFFPART_H
#define BUFFPART_H

struct BuffPart {
	size_t offset, length;
};

int bp_equ_str(char *buff, struct BuffPart bp, char *str);
int bp_equ_bp(char *buff, struct BuffPart bp1, struct BuffPart bp2);

int bp_equ_str(char *buff, struct BuffPart bp, char *str) {
	return !strncmp(buff + bp.offset, str, bp.length);
}

int bp_equ_bp(char *buff, struct BuffPart bp1, struct BuffPart bp2) {
	if(bp1.length != bp2.length) return 0;
	return !strncmp(buff + bp1.offset, buff + bp2.offset, bp1.length);
}

#endif

