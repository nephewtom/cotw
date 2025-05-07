#include "stdio.h"
#include <cstring>
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

// Template version for integer arrays
template <typename T>
void printArray(T* array) {
    for (int i = 0; i < arrlen(array); ++i) {
        printf("%d|", array[i]);
    }
    printf("\n");
}

// Specialization for char* arrays (strings)
template <>
void printArray(const char** array) {
    for (int i = 0; i < arrlen(array); ++i) {
        printf("%s|", array[i]);
    }
    printf("\n");
}

int main(void)
{
	int *int_array = NULL;
	
	arrput(int_array, 2);
	arrput(int_array, 3);
	arrput(int_array, 5);
	printf("* Array of integers: ");
	printArray(int_array);
	printf("arrlenu: %zu\n", arrlenu(int_array));
	
	const char **str_array = NULL;
	arrput(str_array, "hello");
	arrput(str_array, " ");
	arrput(str_array, "world");
	arrput(str_array, "!");
	printf("* Array of const char*: ");
	printArray(str_array);
	printf("arrlenu: %zu\n", arrlenu(str_array));
	
	arrput(str_array, "Hey!");
	printArray(str_array);
	printf("arrlenu: %zu\n", arrlenu(str_array));

	printf("Cleaning it!\n");
	arrfree(str_array);
	printArray(str_array);
	printf("arrlenu: %zu\n", arrlenu(str_array));

	printf("\n* Starting over:\n");
	arrput(str_array, "zero");
	arrput(str_array, "one");
	arrput(str_array, "three");
	printArray(str_array);
	printf("arrlenu: %zu\n", arrlenu(str_array));
	
	arrins(str_array, 2, "two");
	printArray(str_array);
	printf("arrlenu: %zu\n", arrlenu(str_array));

	printf("** Using arraddnindex to append unitialized space at the end:\n");
	size_t index = arraddnindex(str_array, 2);
	printf("index: %zu\n", index);
	printf("arrlenu: %zu\n", arrlenu(str_array));
	str_array[index] = "six";
	str_array[index+1] = "seven";
	printArray(str_array);	
	
	printf("** Using arrinsn to insert at an index:\n");	
	arrinsn(str_array, index, 2);
	printf("arrlenu: %zu\n", arrlenu(str_array));
	str_array[index] = "four";
	str_array[index+1] = "five";
	printArray(str_array);	

	printf("** Using arraddnptr to insert unitialized space at the end:\n");	
	const char **new_elements = arraddnptr(str_array, 2);
	printf("arrlenu: %zu\n", arrlenu(str_array));
	new_elements[0] = "eight";
	new_elements[1] = "nine";
	printArray(str_array);
	printf("arrlenu: %zu\n", arrlenu(str_array));
	
	arrfree(str_array);
	return 0;
}
