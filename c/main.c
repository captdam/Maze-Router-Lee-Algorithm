#define MAX_TRY_TIME 3

#define DEBUG 1

#define SAVE_SILENT 1 //Do not pop up the result
#define SAVE_INTERRESULT 1 //Save intermediate result

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "map.c"
#include "net.c"
#include "parser.c"

int main(int argc, char* argv[]) {

/*******************************************************************************
1 - Get user input: What file I am gonna to process
*******************************************************************************/

	if (argc < 2) {
		fputs("Missing argument: Input file\n",stderr);
		return -1;
	}

/*******************************************************************************
2 - Read the empty map and netlist, empty map contains only obstructions
*******************************************************************************/

	printf("Reading netsfile: %s\n",argv[1]);

	struct Map emptyMap;
	struct Netlist netlist;
	parser(argv[1],&emptyMap,&netlist);
	
	printf("Map size = %lu * %lu\n", emptyMap.width, emptyMap.height);
	for (unsigned long int i = 0; i < emptyMap.height; i++) {
		for (unsigned long int j = 0; j < emptyMap.width; j++) {
			printf("\t0x%08x",getMapValueAt(emptyMap,j,i));
		}
		printf("\n");
	}
}