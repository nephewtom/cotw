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

void printContents(sds** contents) {

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

void print_states(sds*& states) {

	printf("states: ");
	for (size_t i = 0; i < arrlenu(states); ++i) {
		printf("%s ", states[i]);
	}
	
	printf("\n\n");
}

void init_contents(sds**& contents) {
	
	sds* row = NULL;
	sds id = sdsnew("i");
	arrput(row, id);
	
	printf("\nInit contents with identity states\n");
	for (int i=0; i<4; i++) {
		sds result = sdsnew(axis[i]);		
		arrput(row, result);
	}
	arrput(contents, row);	
}

void init_batch1(sds*& states) {
	
	printf("Init states with axis[n]\n");
	for (int i=0; i<4; i++) {
		sds ax = sdsnew(axis[i]);
		arrput(states, ax);
	}
}

void first_iteration(sds**& contents, sds*& states) {

	printf("first_iteration()\n");
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
			
			if (strcmp(result, "+x-x") == 0 || strcmp(result, "-x+x") == 0 ||
				strcmp(result, "+z-z") == 0 || strcmp(result, "-z+z") == 0) {
				// printf("(i) ");
				result = sdscpy(result, "i");
			}
			
			if (i == 3) {
				// printf("|\n");
			}
			arrput(row, result);
		}
		arrput(contents, row); // append the row to the table contents
	}
}

void init_batch2(sds**& contents, sds*& states) {
	
	printf("\nInit states with contents\n");
	for (size_t j = 1; j < arrlenu(contents); ++j) {
		sds* row = contents[j];
		for (size_t i = 1; i < arrlenu(row); ++i) {
			
			if ( strcmp(row[i], "i") != 0) {
				sds rotation = sdsnew(row[i]);
				arrput(states, rotation);
			}
		}
	}
}

void second_iteration(sds**& contents, sds*& states) {
	printf("second_iteration()\n");
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
			
			if (strstr(result, "+x-x") != 0 || strstr(result, "-x+x") != 0 ||
				strstr(result, "+z-z") != 0 || strstr(result, "-z+z") != 0) {
				// printf("(rm) ");
				sdsrange(result, 0, sdslen(result) - 5);
			}
			
			if (i == 3) {
				// printf("|\n");
			}
			arrput(row, result);
		}
		arrput(contents, row); // append the row to the table contents
	}
}

void init_batch3(sds**& contents, sds*& states) {
	
	for (size_t j = 5; j < arrlenu(contents); ++j) {
		sds* row = contents[j];
		for (size_t i = 1; i < arrlenu(row); ++i) {
			
			if (sdslen(row[i]) < 6) { continue; }
			sds rotation = sdsnew(row[i]);
			arrput(states, rotation);
		
		}
	}
}

void third_iteration(sds**& contents, sds*& states) {

	printf("third_iteration()\n");
	for (size_t j = 0; j < arrlenu(states); ++j) { // Loop over the existing rotation states
		
		sds* row = NULL;
		sds state = sdsnew(states[j]);
		printf("- state: %s -> ", state);
		arrput(row, state); // append the state to the start of the row

		for (int i=0; i<4; i++) {
			sds result = sdsdup(state);
			result = sdscat(result, axis[i]); // append the axis rotation to the rotation state
			if  (i==0)  {
				printf("result: ");
			}
			printf("| %s ", result);
			
			if (strstr(result, "+x-x") != 0 || strstr(result, "-x+x") != 0 ||
				strstr(result, "+z-z") != 0 || strstr(result, "-z+z") != 0) {
				printf("(rm) ");
				sdsrange(result, 0, sdslen(result) - 5);
			}

			if (strcmp(result, "+x+x+x+x") == 0 || strcmp(result, "-x-x-x-x") == 0 ||
				strcmp(result, "+z+z+z+z") == 0 || strcmp(result, "-z-z-z-z") == 0) {				
					result = sdscpy(result, "i");
			}
			
			if (i == 3) {
				printf("|\n");
			}
			arrput(row, result);
		}
		arrput(contents, row); // append the row to the table contents
	}
	
}

int main() {

	sds** contents = NULL; // table contents
	init_contents(contents);
	
	sds* states1 = NULL;
	init_batch1(states1);
	print_states(states1);
	first_iteration(contents, states1);
	printContents(contents);

	sds* states2 = NULL;
	init_batch2(contents, states2);
	print_states(states2);
	second_iteration(contents, states2);
	printContents(contents);
	
	sds* states3 = NULL;
	init_batch3(contents, states3);
	print_states(states3);
	third_iteration(contents, states3);
	printContents(contents);
		
	return 0; 
}
