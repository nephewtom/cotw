#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define STATE_COUNT 32
#define EVENT_COUNT 4


const char* x_pos = "+x";
const char* x_neg = "-x";
const char* z_pos = "+z";
const char* z_neg = "-z";

enum rotation_axis {
	X_POSITIVE, X_NEGATIVE, Z_POSITIVE, Z_NEGATIVE, INVALID
} rotation_axis;

const char* states[STATE_COUNT] = {
    "i", "+x", "-x", "+z", "-z", "+x+x", "+x+z", "+x-z", "-x+z", "-x-z", "+z+x", "+z-x",
    "+z+z", "-z+x", "-z-x", "+x+x+z", "+x+x-z", "+x+z+x", "+x+z-x", "+x+z+z", "+x-z+x",
    "+x-z-x", "-x+z+z", "+z+x+x", "+z+x+z", "+z+x-z", "+z-x+z", "+z-x-z", "+z+z+x",
    "+z+z-x", "-z+x+x", "+x+x+z+z"
};

const char* rotation_table[STATE_COUNT][EVENT_COUNT] = {
    {"+x", "-x", "+z", "-z"},  // i
    {"+x+x", "i", "+x+z", "+x-z"},
    {"i", "+x+x", "-x+z", "-x-z"},
    {"+z+x", "+z-x", "+z+z", "i"},
    {"-z+x", "-z-x", "i", "+z+z"},
    {"-x", "+x", "+x+x+z", "+x+x-z"},
    {"+x+z+x", "+x+z-x", "+x+z+z", "+x"},
    {"+x-z+x", "+x-z-x", "+x", "+x+z+z"},
    {"+x-z-x", "+x-z+x", "-x+z+z", "-x"},
    {"+x+z-x", "+x+z+x", "-x", "-x+z+z"},
    {"+z+x+x", "+z", "+z+x+z", "+z+x-z"},
    {"+z", "+z+x+x", "+z-x+z", "+z-x-z"},
    {"+z+z+x", "+z+z-x", "-z", "+z"},
    {"-z+x+x", "-z", "+z-x-z", "+z-x+z"},
    {"-z", "-z+x+x", "+z+x-z", "+z+x+z"},
    {"-z-x", "-z+x", "+x+x+z+z", "+x+x"},
    {"+z-x", "+z+x", "+x+x", "+x+x+z+z"},
    {"-x-z", "+x+z", "-z-x", "+z+x"},
    {"+x+z", "-x-z", "+z-x", "-z+x"},
    {"+z+z", "+x+x+z+z", "+x-z", "+x+z"},
    {"-x+z", "+x-z", "-z+x", "+z-x"},
    {"+x-z", "-x+z", "+z+x", "-z-x"},
    {"+x+x+z+z", "+z+z", "-x-z", "-x+z"},
    {"+z-x", "+z+x", "+x+x", "+x+x+z+z"},
    {"-x-z", "+x+z", "-z-x", "+z+x"},
    {"+x-z", "-x+z", "+z+x", "-z-x"},
    {"-x+z", "+x-z", "-z+x", "+z-x"},
    {"+x+z", "-x-z", "+z-x", "-z+x"},
    {"+x+x+z+z", "+z+z", "-x-z", "-x+z"},
    {"+z+z", "+x+x+z+z", "+x-z", "+x+z"},
    {"-z-x", "-z+x", "+x+x+z+z", "+x+x"},
    {"+x+z+z", "-x+z+z", "+z+x+x", "+x+x+z"}
};

// Hash map: string key to int index
typedef struct {
    const char* key;
    int value;
} MapEntry;

MapEntry* state_map = NULL; // stb_ds hash table

void init_state_map() {
    for (int i = 0; i < STATE_COUNT; i++) {
		/* printf("state[%d]=%s\n", i, states[i]); */
		shput(state_map, states[i], i);
    }
}
void check_state_map() {
	
	const char* queries[] = { "+x", "-x", "+z", "-z", "+x+x", "+x+x+z" };
    char s[32];
    for (int i = 0; i < 6; i++) {
        strcpy(s, queries[i]);
        int index = shget(state_map, s);
        printf("%s -> %d\n", s, index);
    }
}

const char* rotate(const char* current_state, enum rotation_axis axis) {
	
	/* printf("-> current_state:%s\n", current_state); */
	/* printf("-> rotation_axis:%d\n", axis); */
	int index = shget(state_map, current_state);
	/* printf("-> index:%d\n", index); */
	
	if (index == -1) {
		printf("ERROR: State not found!\n");
		return NULL;
	}
	if (axis < X_POSITIVE || axis >= INVALID) {
		printf("ERROR: Invalid rotation axis!\n");
		return NULL;
	}
	const char* result = rotation_table[index][axis];
	printf("rotation_table[%i][%i]=%s\n", index, axis, result);
	return result;
}

enum rotation_axis get_rotation_axis(const char* ra) {
	if (strcmp(ra, x_pos) == 0) return X_POSITIVE;
	if (strcmp(ra, x_neg) == 0) return X_NEGATIVE;
	if (strcmp(ra, z_pos) == 0) return Z_POSITIVE;
	if (strcmp(ra, z_neg) == 0) return Z_NEGATIVE;
	return INVALID;
}
const char* get_rotation_axis_str(enum rotation_axis ra) {
	if (ra == X_POSITIVE) return x_pos;
	if (ra == X_NEGATIVE) return x_neg;
	if (ra == Z_POSITIVE) return z_pos;
	if (ra == Z_NEGATIVE) return z_neg;
	return "Invalid";
}


void show_rotation_table() {
	for (int i=0; i<STATE_COUNT; i++) {
		printf("%2d ", i+1);
		for (int j=0; j<EVENT_COUNT; j++) {
			printf("%s ", rotation_table[i][j]);
		}
		printf("\n");
	}
}

int main() {
	init_state_map();
	/* show_rotation_table(); */
	/* check_state_map(); */
	
	char state[32];
	strcpy(state, "i");
		
	char axis[32];
	while (true) {
		printf("--------------");
		for (size_t i=0; i<strlen(state);i++) { printf("-");}
		printf("\n");
		printf("Current state:%s\n", state);
		printf("Enter axis:");
		if (fgets(axis, sizeof(axis), stdin) != NULL) {
			size_t len = strlen(axis);
			if (len > 0 && axis[len - 1] == '\n') {
				// Remove newline character if present
				axis[len - 1] = '\0';
			}
			
			if (strcmp(axis, "q") == 0) {
				exit(0);
			}
			
			enum rotation_axis rot_axis = get_rotation_axis(axis);
			const char *result = rotate(state, rot_axis);
			if (result != NULL) {
				printf("-> result:%s\n", result);
				strcpy(state, result);
			}
		} else {
			printf("Error reading input!\n");
		}
	}
	return 0;
}
