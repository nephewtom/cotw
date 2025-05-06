#include <cstring>
#include <stdio.h>

extern "C" {
#include "sds/sds.h"
}

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

const char* x_pos = "+x";
const char* x_neg = "-x";
const char* z_pos = "+z";
const char* z_neg = "-z";

const char* axis[4] = { x_pos, x_neg, z_pos, z_neg };

const char* cReset = "\033[0m"; // color reset

const char* lCyan = "\033[0;36m";
const char* lRed = "\033[0;31m";
const char* lYellow = "\033[0;33m";
const char* lPurple = "\033[0;35m";
const char* lGreen = "\033[0;32m";


#define LOG(fmt, ...)													\
    printf("%s:%d | %s%s()%s - " fmt "\n", __FILE__, __LINE__, lGreen, __func__, cReset, ##__VA_ARGS__)

void print_table(sds** contents) {

	// TODO: indent correctly contents
	sds t = contents[0][1];
	int size = (int)sdslen(t); // not used
	size = 8;
	// printf("size: %d\n", size);	
	// printf("Table contents\n");
	
	printf("| %-*s |", size, "state");
	for (int i=0; i<4; i++) {
		printf(" %-*s |", size, axis[i]);
	}
	printf("\n");
	for (int j=0; j<5; j++) {
		printf("+");
		for (int i=0; i<size+2; i++) {
			printf("-");
		}
	}
	printf("+\n");
	for (size_t j = 0; j < arrlenu(contents); ++j) {
		sds* row = contents[j];
		for (size_t i = 0; i < arrlenu(row); ++i) {
			if (i == 0) {
				printf("| %-*s ", size, row[i]);}
			else {
				printf("| %-*s ", size, row[i]);
			}
		}
		printf("|\n");
	}
}




void init_contents(sds**& contents);
void init_states(int iteration, sds*& states, sds**& contents);
void print_states(sds*& states);
void run_iteration(int iteration, sds*& states, sds**& contents);
void simplify(int iteration, sds& result);

int main() {

	sds** table = NULL; // table contents
	init_contents(table);
	
	sds* one_rotation = NULL;
	sds* two_rotations = NULL;
	sds* three_rotations = NULL;
	
	init_states(1, one_rotation, table);
	print_states(one_rotation);
	run_iteration(1, one_rotation, table);
	print_table(table);

	init_states(2, two_rotations, table);
	print_states(two_rotations);
	run_iteration(2, two_rotations, table);
	print_table(table);
	
	init_states(3, three_rotations, table);
	print_states(three_rotations);
	run_iteration(3, three_rotations, table);
	print_table(table);
	
	
	return 0; 
}

typedef struct {
    const char *rotations;
    const char *replacement;
} Equivalence;

Equivalence equivalences2rot[] = {
    {"-x-x", "+x+xD"},
    {"-z-z", "+z+zD"},
};
Equivalence equivalences3rot[] = {
	{"+x+x+x", "-x"},
	{"-x-x-x", "+x"},
	{"+z+z+z", "-z"},
	{"-z-z-z", "+z"},

	{"-x-x+z", "+x+x+zD"},
	{"-x-x-z", "+x+x-zD"},
	{"-z-z+x", "+z+z+xD"},
	{"-z-z-x", "+z+z-xD"},

	{"-x-z-x", "+x+z+xD"},
	{"-x-z+x", "+x+z-xD"},
	{"+x-z-z", "+x+z+zD"},
	{"-x+z-x", "+x-z+xD"},
	{"-x+z+x", "+x-z-xD"},
	{"-x-z-z", "-x+z+zD"},
	{"+z-x-x", "+z+x+xD"},
	{"-z-x-z", "+z+x+zD"},
	{"-z-x+z", "+z+x-zD"},
	{"-z+x-z", "+z-x+zD"},
	{"-z+x+z", "+z-x-zD"},
	{"-z-x-x", "-z+x+xD"},
};

void init_contents(sds**& contents) {
	
	sds* row = NULL;
	sds id = sdsnew("i");
	arrput(row, id);
	
	LOG("Setting contents with identity states");
	for (int i=0; i<4; i++) {
		sds result = sdsnew(axis[i]);		
		arrput(row, result);
	}
	arrput(contents, row);	
}

void init_states(int iteration, sds*& states, sds**& contents) {

	LOG("%siteration: %d%s", lRed, iteration, cReset);
	if (iteration == 1) {

		for (int i=0; i<4; i++) {
			sds ax = sdsnew(axis[i]);
			arrput(states, ax);
		}	
		size_t size = arrlenu(states);
		LOG("Setting states with axis => size: %zu", size);
		return;
	}
	
	size_t start = 0 , char_threshold = 0;
	if (iteration == 2) {
		start = 1;
		char_threshold = 4;
	} else if (iteration == 3) {
		start = 5;
		char_threshold = 6;
	}

	LOG("Loop start: %zu | threshold chars: %zu", start, char_threshold);
	
	for (size_t j = start; j < arrlenu(contents); ++j) {
		sds *row = contents[j];
		for (size_t i = 1; i < arrlenu(row); ++i) {

			// printf("row[i]: %s", row[i]);
			if (iteration == 3 and row[i][6] == 'D') {
				// printf(" -> skip!\n");
				continue;
			}
			// printf("\n");
			
			if (sdslen(row[i]) != char_threshold) { 
				continue;
			}
			sds rotation = sdsnew(row[i]);
			arrput(states, rotation);
		}
	}
	size_t size = arrlenu(states);
	LOG("Setting states with contents => size: %zu", size);
}

void print_states(sds*& states) {

	sds msg = sdsnew("states: ");
	
	for (size_t i = 0; i < arrlenu(states); ++i) {
		msg = sdscatfmt(msg, "%s%s%s ", lCyan, states[i], cReset);
	}
	LOG("%s", msg);
	sdsfree(msg);
}

void run_iteration(int iteration, sds*& states, sds**& contents) {

	for (size_t j = 0; j < arrlenu(states); ++j) { // Loop over the existing rotation states
		
		sds* row = NULL;
		sds state = sdsnew(states[j]);
		// printf("- state: %s -> ", state);
		arrput(row, state); // append the state to the start of the row

		for (int i=0; i<4; i++) {
			sds result = sdsdup(state);
			result = sdscat(result, axis[i]); // append the axis rotation to the rotation state
			if  (i==0)  {
				// printf("result: ");
			}

			// printf("| %s ", result);
			simplify(iteration, result);
			
			if (i == 3) {
					// printf("|\n");
				}
				arrput(row, result);
			}
			arrput(contents, row); // append the row to the table contents
		}
		LOG("table after iteration: %d", iteration);
	}

	void simplify(int iteration, sds& result) {

		if (strstr(result, "+x-x") != 0 || strstr(result, "-x+x") != 0 ||
			strstr(result, "+z-z") != 0 || strstr(result, "-z+z") != 0) {
		
			if (iteration == 1) {
				// reduce to unity
				result = sdscpy(result, "i");
				// printf("(i) ");
			}
			if (iteration == 2 || iteration == 3) {
				// remove cancelling terms
				sdsrange(result, 0, sdslen(result) - 5);
				// printf("(rm) ");
			}
		}

		if (iteration == 1) {
			for (size_t i = 0; i < sizeof(equivalences2rot) / sizeof(equivalences2rot[0]); ++i) {
				if (strcmp(result, equivalences2rot[i].rotations) == 0) {
					result = sdscpy(result, equivalences2rot[i].replacement);
					break;
				}
			}
		}
	
		if (iteration == 2) {		
			for (size_t i = 0; i < sizeof(equivalences3rot) / sizeof(equivalences3rot[0]); ++i) {
				if (strcmp(result, equivalences3rot[i].rotations) == 0) {
					result = sdscpy(result, equivalences3rot[i].replacement);
					break;
				}
			}
		}
	
		if (iteration == 3) {
			if (strcmp(result, "+x+x+x+x") == 0 || strcmp(result, "-x-x-x-x") == 0 ||
				strcmp(result, "+z+z+z+z") == 0 || strcmp(result, "-z-z-z-z") == 0) {				
				result = sdscpy(result, "i");
			}				
		}
	}
