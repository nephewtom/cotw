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
	size = 10;
	// printf("size: %d\n", size);	
	// printf("Table contents\n");
	
	// Prints header
	printf("| %-*s |", size, "state");
	for (int i=0; i<4; i++) {
		printf(" %-*s |", size, axis[i]);
	}
	printf("\n");
	for (int j=0; j<5; j++) {
		printf("|");
		for (int i=0; i<size+2; i++) {
			printf("-");
		}
	}
	printf("|\n");

	// Loop contents and prints rows
	for (size_t j = 0; j < arrlenu(contents); ++j) {
		sds* row = contents[j];
		for (size_t i = 0; i < arrlenu(row); ++i) {
			
			if (i == 0) { // first column is the state
				printf("| %-*s ", size, row[i]);
			}
			else {
 				size_t len = sdslen(row[i]);
				char last = row[i][len - 1];
				if (last == 'E') {
					printf("| %-*.*s ", size, (int)(len - 1), row[i]); // print all but the last char
				} else {
					printf("| %-*s ", size, row[i]);
				}
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
	sds* four_rotations = NULL;
	
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
	
	init_states(4, four_rotations, table);
	print_states(four_rotations);
	run_iteration(4, four_rotations, table);
	print_table(table);
	
	return 0; 
}

typedef struct {
    const char *rotations;
    const char *replacement;
} Equivalence;


// E is used to mark that the combination is a equivalent to other.
// E stand for duplicate, so it does not have to be considered in next iteration.
Equivalence equivalences2rot[] = {
    {"-x-x", "+x+xE"},
    {"-z-z", "+z+zE"},
};
Equivalence equivalences3rot[] = {
	{"+x+x+x", "-x"},
	{"-x-x-x", "+x"},
	{"+z+z+z", "-z"},
	{"-z-z-z", "+z"},

	{"-x-x+z", "+x+x+zE"},
	{"-x-x-z", "+x+x-zE"},
	{"-z-z+x", "+z+z+xE"},
	{"-z-z-x", "+z+z-xE"},

	{"-x-z-x", "+x+z+xE"},
	{"-x-z+x", "+x+z-xE"},
	{"+x-z-z", "+x+z+zE"},
	{"-x+z-x", "+x-z+xE"},
	{"-x+z+x", "+x-z-xE"},
	{"-x-z-z", "-x+z+zE"},
	{"+z-x-x", "+z+x+xE"},
	{"-z-x-z", "+z+x+zE"},
	{"-z-x+z", "+z+x-zE"},
	{"-z+x-z", "+z-x+zE"},
	{"-z+x+z", "+z-x-zE"},
	{"-z-x-x", "-z+x+xE"},
};

Equivalence equivalences4rot[] = {
	{"+x+x+z+x", "-z-x"},	   
	{"+x+x+z-x", "-z+x"},
	{"+x+x-z+x", "+z-x"},
	{"+x+x-z-x", "+z+x"},
	{"+x+x-z-z", "+x+x+z+zE"},
	{"+x+z+x+x", "-x-z"},
	{"+x+z+x+z", "-z-x"},
	{"+x+z+x-z", "+z+x"},
	{"+x+z-x-x", "-x-z"},
	{"+x+z-x+z", "+z-x"},
	{"+x+z-x-z", "-z+x"},
	{"+x+z+z+x", "+z+z"},
	{"+x+z+z-x", "+x+x+z+zE"},
	{"+x+z+z+z", "+x-z"},
	{"+x-z+x+x", "-x+z"},
	{"+x-z+x+z", "-z+x"},
	{"+x-z+x-z", "+z-x"},
	{"+x-z-x-x", "-x+z"},
	{"+x-z-x+z", "+z+x"},
	{"+x-z-x-z", "-z-x"},
	{"-x+z+z+x", "+x+x+z+zE"},
	{"-x+z+z-x", "+z+z"},
	{"-x+z+z+z", "-x-z"},
	{"+z+x+x+x", "+z-x"},
	{"+z+x+x+z", "+x+x"},
	{"+z+x+x-z", "+x+x+z+zE"},
	{"+z+x+z+x", "-x-z"},
	{"+z+x+z-x", "+x+z"},
	{"+z+x+z+z", "-z-x"},
	{"+z+x-z+x", "+x-z"},
	{"+z+x-z-x", "-x+z"},
	{"+z+x-z-z", "-z-x"},
	{"+z-x+z+x", "-x+z"},
	{"+z-x+z-x", "+x-z"},
	{"+z-x+z+z", "-z+x"},
	{"+z-x-z+x", "+x+z"},
	{"+z-x-z-x", "-x-z"},
	{"+z-x-z-z", "-z+x"},
	{"+z+z+x+x", "+x+x+z+zE"},
	{"+z+z+x+z", "-x-z"},
	{"+z+z+x-z", "-x+z"},
	{"+z+z-x-x", "+x+x+z+zE"},
	{"+z+z-x+z", "+x-z"},
	{"+z+z-x-z", "+x+z"},
	{"-z+x+x+x", "-z-x"},
	{"-z+x+x+z", "+x+x+z+zE"},
	{"-z+x+x-z", "+x+x"},	
};

Equivalence equivalences5rot[] = {
	{"+x+x+z+z+x", "+x+z+z"},
	{"+x+x+z+z-x", "-x+z+z"},
	{"+x+x+z+z+z", "+z+x+x"},
	{"+x+x+z+z-z", "+x+x+z"},
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
	} else if (iteration == 4) {
		start = 15;
		char_threshold = 8;
	}

	LOG("Loop start: %zu | threshold chars: %zu", start, char_threshold);
	size_t contents_size = arrlenu(contents);
	LOG("contents_size: %zu", contents_size);	
	for (size_t j = start; j < contents_size; ++j) {
		sds *row = contents[j];
		for (size_t i = 1; i < arrlenu(row); ++i) {

			// printf("row[i]: %s", row[i]);
			if (iteration == 3 and row[i][6] == 'E') {
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
	LOG("Setting states with contents => state_size: %zu", size);
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
		for (size_t i = 0; i < sizeof(equivalences4rot) / sizeof(equivalences4rot[0]); ++i) {
			if (strcmp(result, equivalences4rot[i].rotations) == 0) {
				result = sdscpy(result, equivalences4rot[i].replacement);
				break;
			}
		}
	}
	if (iteration == 4) {
		for (size_t i = 0; i < sizeof(equivalences5rot) / sizeof(equivalences5rot[0]); ++i) {
			if (strcmp(result, equivalences5rot[i].rotations) == 0) {
				result = sdscpy(result, equivalences5rot[i].replacement);
				break;
			}
		}
	}
}
