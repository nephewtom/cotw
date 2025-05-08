#include <stdio.h>
#include <string.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

// Rotation Events
#define ROTATION_EVENT_COUNT 4
const char* rotation_events[ROTATION_EVENT_COUNT] = { "+x", "-x", "+z", "-z" }; 

typedef enum {
	X_POSITIVE=0, X_NEGATIVE, Z_POSITIVE, Z_NEGATIVE, INVALID
} rotation_axis;

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
	const char* tg_face; // touching ground face
	const char* ls_face; // looking sky face
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
		printf("state[%d]=%s\n", i, rotation_states[i]);
		shput(rotation_state_map, rotation_states[i], i);
	}
}

rotation_axis get_rotation_axis(const char* ra) {
	if (strcmp(ra, x_pos) == 0) return X_POSITIVE;
	if (strcmp(ra, x_neg) == 0) return X_NEGATIVE;
	if (strcmp(ra, z_pos) == 0) return Z_POSITIVE;
	if (strcmp(ra, z_neg) == 0) return Z_NEGATIVE;
	return INVALID;
}
const char* get_rotation_axis_str(rotation_axis ra) {
	if (ra == X_POSITIVE) return x_pos;
	if (ra == X_NEGATIVE) return x_neg;
	if (ra == Z_POSITIVE) return z_pos;
	if (ra == Z_NEGATIVE) return z_neg;
	return "Invalid";
}

int get_next_rotation_state(const char* current_state, rotation_axis axis, RotationEntry* rot_info) {
	
	int index = shget(rotation_state_map, current_state);
	
	if (index == -1) {
		/* printf("ERROR: State not found!\n"); */
		return -1;
	}
	if (axis < X_POSITIVE || axis >= INVALID) {
		/* printf("ERROR: Invalid state axis!\n"); */
		return -1;
	}
	*rot_info = rotation_table[index][axis];
	return 0;
}

#if 0
char state[32];
char tg_face[16];
char ls_face[16];
char axis[32];

void rotate_cube() {
	
	size_t len = strlen(axis);
	if (len > 0 && axis[len - 1] == '\n') {
		// Remove newline character if present
		axis[len - 1] = '\0';
	}
			
	if (strcmp(axis, "q") == 0) {
		exit(0);
	}
			
	rotation_axis rot_axis = get_rotation_axis(axis);
	RotationEntry rot_info;
	int result = get_next_rotation_state(state, rot_axis, &rot_info);
	if (result == 0) {
		printf("-> rot_info: (%s, %s, %s)\n", 
			   rot_info.state, rot_info.tg_face, rot_info.ls_face);
		strcpy(state, rot_info.state);
		strcpy(tg_face, rot_info.tg_face);
		strcpy(ls_face, rot_info.ls_face);
	}
}

int main() {
	init_state_map();
	
	strcpy(state, "i");
	strcpy(tg_face, "6D");
	strcpy(ls_face, "1U");
		
	while (true) {
		printf("--------------------------------");
		for (size_t i=0; i<strlen(state);i++) { printf("-");}
		printf("\n");
		printf("state:%s | tg_face:%s | ls_face:%s\n", state, tg_face, ls_face);
		printf("Enter axis:");
		if (fgets(axis, sizeof(axis), stdin) != NULL) {
			rotate_cube();
		} else {
			printf("Error reading input!\n");
		}
	}
	return 0;
}

#endif 
