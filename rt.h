#include <stdio.h>
#include <string.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

// This is the convention for the cube starting position and orientation:

// T = TOP (face 1), F = FRONT (face 2), R = RIGHT (face 3) 
// K = BACK (face 4), L = LEFT (face 5), M = BOTTOM (face 6) 
//
// z <-----------------------------------+
//          +---+             +---+      | 
//          | K |             | 4 |      | 
//      +---+---+---+     +---+---+---+  | 
//      | L | T | R |     | 5 | 1 | 3 |  | 
//      +---+---+---+     +---+---+---+  | 
//          | F |             | 2 |      | 
//          +---+             +---+      | 
//          | M |             | 6 |      | 
//          +---+             +---+      | 
//                                       |
//                                       V
//                                       x


// ORIENTED_UP      ORIENTED_RIGHT     ORIENTED_DOWN      ORIENTED_LEFT
// z <--------+     z <--------+       z <--------+       z <--------+
//      |  ^  |          |     |            |     |            |     |
//      |  |  |          | --> |            |  |  |            | <-- |
//      |     |          |     |            |  V  |            |     |
//      +-----+          +-----+            +-----+            +-----+
//            |                |                  |                  |
//            V                V                  V                  V
//            x                x                  x                  x

// Rotation Events
#define ROTATION_EVENT_COUNT 4
const char* rotation_events[ROTATION_EVENT_COUNT] = { "+x", "-x", "+z", "-z" }; 

typedef enum {
	X_POSITIVE=0, X_NEGATIVE, Z_POSITIVE, Z_NEGATIVE, INVALID, Y_POSITIVE, Y_NEGATIVE
} Axis;

#define x_pos rotation_events[X_POSITIVE]
#define x_neg rotation_events[X_NEGATIVE]
#define z_pos rotation_events[Z_POSITIVE]
#define z_neg rotation_events[Z_NEGATIVE]

// Rotation States
#define ROTATION_STATE_COUNT 32
const char* rotation_states[ROTATION_STATE_COUNT] = {
	"i", "+x", "-x", "+z", "-z", "+x+x", "+x+z", "+x-z", "-x+z", "-x-z", "+z+x", "+z-x",
	"+z+z", "-z+x", "-z-x", "+x+x+z", "+x+x-z", "+x+z+x", "+x+z-x", "+x+z+z", "+x-z+x",
	"+x-z-x", "-x+z+z", "+z+x+x", "+z+x+z", "+z+x-z", "+z-x+z", "+z-x-z", "+z+z+x",
	"+z+z-x", "-z+x+x", "+x+x+z+z"
};

// Rotation Table
typedef struct {
	const char* state;
	const char* TG_face; // Touching Ground face
	const char* LS_face; // looking Sky face
	// TODO: change const char* by ints or char? Like TG_face = 1-5 & TG_orientation= U/D/R/L ? 
	// int TG_face;
	// int TG_orientation;
} RotationEntry;

RotationEntry rotation_table[ROTATION_STATE_COUNT][ROTATION_EVENT_COUNT] = {
    { {"+x", "5L", "3L"}, {"-x", "3R", "5R"}, {"+z", "4U", "2U"}, {"-z", "2D", "4D"} },
    { {"+x+x", "1U", "6D"}, {"i", "6D", "1U"}, {"+x+z", "4L", "2L"}, {"+x-z", "2L", "4L"} },
    { {"i", "6D", "1U"}, {"+x+x", "1U", "6D"}, {"-x+z", "4R", "2R"}, {"-x-z", "2R", "4R"} },
    { {"+z+x", "5U", "3U"}, {"+z-x", "3U", "5U"}, {"+z+z", "1D", "6U"}, {"i", "6D", "1U"} },
    { {"-z+x", "5D", "3D"}, {"-z-x", "3D", "5D"}, {"i", "6D", "1U"}, {"+z+z", "1D", "6U"} },
    { {"-x", "3R", "5R"}, {"+x", "5L", "3L"}, {"+x+x+z", "4D", "2D"}, {"+x+x-z", "2U", "4U"} },
    { {"+x+z+x", "1R", "6L"}, {"+x+z-x", "6R", "1L"}, {"+x+z+z", "3L", "5L"}, {"+x", "5L", "3L"} },
    { {"+x-z+x", "1L", "6R"}, {"+x-z-x", "6L", "1R"}, {"+x", "5L", "3L"}, {"+x+z+z", "3L", "5L"} },
    { {"+x-z-x", "6L", "1R"}, {"+x-z+x", "1L", "6R"}, {"-x+z+z", "5R", "3R"}, {"-x", "3R", "5R"} },
    { {"+x+z-x", "6R", "1L"}, {"+x+z+x", "1R", "6L"}, {"-x", "3R", "5R"}, {"-x+z+z", "5R", "3R"} },
    { {"+z+x+x", "2U", "4U"}, {"+z", "4U", "2U"}, {"+z+x+z", "1R", "6L"}, {"+z+x-z", "6L", "1R"} },
    { {"+z", "4U", "2U"}, {"+z+x+x", "2U", "4U"}, {"+z-x+z", "1L", "6R"}, {"+z-x-z", "6R", "1L"} },
    { {"+z+z+x", "5R", "3R"}, {"+z+z-x", "3L", "5L"}, {"-z", "2D", "4D"}, {"+z", "4U", "2U"} },
    { {"-z+x+x", "4D", "2D"}, {"-z", "2D", "4D"}, {"+z-x-z", "6R", "1L"}, {"+z-x+z", "1L", "6R"} },
    { {"-z", "2D", "4D"}, {"-z+x+x", "4D", "2D"}, {"+z+x-z", "6L", "1R"}, {"+z+x+z", "1R", "6L"} },
    { {"-z-x", "3D", "5D"}, {"-z+x", "5D", "3D"}, {"+x+x+z+z", "6U", "1D"}, {"+x+x", "1U", "6D"} },
    { {"+z-x", "3U", "5U"}, {"+z+x", "5U", "3U"}, {"+x+x", "1U", "6D"}, {"+x+x+z+z", "6U", "1D"} },
    { {"-x-z", "2R", "4R"}, {"+x+z", "4L", "2L"}, {"-z-x", "3D", "5D"}, {"+z+x", "5U", "3U"} },
    { {"+x+z", "4L", "2L"}, {"-x-z", "2R", "4R"}, {"+z-x", "3U", "5U"}, {"-z+x", "5D", "3D"} },
    { {"+z+z", "1D", "6U"}, {"+x+x+z+z", "6U", "1D"}, {"+x-z", "2L", "4L"}, {"+x+z", "4L", "2L"} },
    { {"-x+z", "4R", "2R"}, {"+x-z", "2L", "4L"}, {"-z+x", "5D", "3D"}, {"+z-x", "3U", "5U"} },
    { {"+x-z", "2L", "4L"}, {"-x+z", "4R", "2R"}, {"+z+x", "5U", "3U"}, {"-z-x", "3D", "5D"} },
    { {"+x+x+z+z", "6U", "1D"}, {"+z+z", "1D", "6U"}, {"-x-z", "2R", "4R"}, {"-x+z", "4R", "2R"} },
    { {"+z-x", "3U", "5U"}, {"+z+x", "5U", "3U"}, {"+x+x", "1U", "6D"}, {"+x+x+z+z", "6U", "1D"} },
    { {"-x-z", "2R", "4R"}, {"+x+z", "4L", "2L"}, {"-z-x", "3D", "5D"}, {"+z+x", "5U", "3U"} },
    { {"+x-z", "2L", "4L"}, {"-x+z", "4R", "2R"}, {"+z+x", "5U", "3U"}, {"-z-x", "3D", "5D"} },
    { {"-x+z", "4R", "2R"}, {"+x-z", "2L", "4L"}, {"-z+x", "5D", "3D"}, {"+z-x", "3U", "5U"} },
    { {"+x+z", "4L", "2L"}, {"-x-z", "2R", "4R"}, {"+z-x", "3U", "5U"}, {"-z+x", "5D", "3D"} },
    { {"+x+x+z+z", "6U", "1D"}, {"+z+z", "1D", "6U"}, {"-x-z", "2R", "4R"}, {"-x+z", "4R", "2R"} },
    { {"+z+z", "1D", "6U"}, {"+x+x+z+z", "6U", "1D"}, {"+x-z", "2L", "4L"}, {"+x+z", "4L", "2L"} },
    { {"-z-x", "3D", "5D"}, {"-z+x", "5D", "3D"}, {"+x+x+z+z", "6U", "1D"}, {"+x+x", "1U", "6D"} },
    { {"+x+z+z", "3L", "5L"}, {"-x+z+z", "5R", "3R"}, {"+z+x+x", "2U", "4U"}, {"+x+x+z", "4D", "2D"} }
};

// Hash map to retrieve state index
typedef struct {
	const char* key;
	int value;
} rotation_state_entry;

rotation_state_entry* rotation_state_map = NULL; // stb_ds hash table
void init_state_map() {
	for (int i = 0; i < ROTATION_STATE_COUNT; i++) {
		// printf("state[%d]=%s\n", i, rotation_states[i]);
		shput(rotation_state_map, rotation_states[i], i);
	}
}

Axis get_rotation_axis(const char* ra) {
	if (strcmp(ra, x_pos) == 0) return X_POSITIVE;
	if (strcmp(ra, x_neg) == 0) return X_NEGATIVE;
	if (strcmp(ra, z_pos) == 0) return Z_POSITIVE;
	if (strcmp(ra, z_neg) == 0) return Z_NEGATIVE;
	return INVALID;
}
const char* get_rotation_axis_str(Axis ra) {
	if (ra == X_POSITIVE) return x_pos;
	if (ra == X_NEGATIVE) return x_neg;
	if (ra == Z_POSITIVE) return z_pos;
	if (ra == Z_NEGATIVE) return z_neg;
	return "Invalid";
}

int get_next_rotation_state(const char* current_state, Axis Axis, RotationEntry* rot_info) {
	
	int index = shget(rotation_state_map, current_state);
	
	if (index == -1) {
		/* printf("ERROR: State not found!\n"); */
		return -1;
	}
	if (Axis < X_POSITIVE || Axis >= INVALID) {
		/* printf("ERROR: Invalid state Axis!\n"); */
		return -1;
	}
	*rot_info = rotation_table[index][Axis];
	return 0;
}

#if 0
char state[32];
char TG_face[16];
char LS_face[16];
char Axis[32];

void rotate_cube() {
	
	size_t len = strlen(Axis);
	if (len > 0 && Axis[len - 1] == '\n') {
		// Remove newline character if present
		Axis[len - 1] = '\0';
	}
			
	if (strcmp(Axis, "q") == 0) {
		exit(0);
	}
			
	Axis rot_axis = get_rotation_axis(Axis);
	RotationEntry rot_info;
	int result = get_next_rotation_state(state, rot_axis, &rot_info);
	if (result == 0) {
		printf("-> rot_info: (%s, %s, %s)\n", 
			   rot_info.state, rot_info.TG_face, rot_info.LS_face);
		strcpy(state, rot_info.state);
		strcpy(TG_face, rot_info.TG_face);
		strcpy(LS_face, rot_info.LS_face);
	}
}

int main() {
	init_state_map();
	
	strcpy(state, "i");
	strcpy(TG_face, "6D");
	strcpy(LS_face, "1U");
		
	while (true) {
		printf("--------------------------------");
		for (size_t i=0; i<strlen(state);i++) { printf("-");}
		printf("\n");
		printf("state:%s | TG_face:%s | LS_face:%s\n", state, TG_face, LS_face);
		printf("Enter Axis:");
		if (fgets(Axis, sizeof(Axis), stdin) != NULL) {
			rotate_cube();
		} else {
			printf("Error reading input!\n");
		}
	}
	return 0;
}

#endif 
