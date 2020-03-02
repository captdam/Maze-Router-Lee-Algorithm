#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 255

#ifndef DEBUG_PARSER
#define DEBUG_PARSER 1
#endif

//Read a file contains map size, obstructions and nets, return map struct and netlist by reference
void parser(char* filename, struct Map *map, struct Netlist *netlist) {
	
	/* NOTE:
	Linkedlist vs realloc array:
	Number of nets are unknown. To save all of them, linkedlist and array can be used.
	1 - Array: Read an element, push array. If more element, realloc the array size, push the new element.
	2 - Linked list: Read an element, save it in struct. If more element, alloc a new struct and link the new struct to old struct's next ptr.
	Realloc array is easy to write; however, the entire memory has to be copy to new location if overlap found (current segment is not big enough), which is an overhead.
	The parser will first read elements into linked list.
	After reading all elements, reformat the linked list into array for easy implementation for parent function.
	*/
	
	FILE* fp = fopen(filename,"r");
	if (!fp) {
		fputs("Cannot read file.\n",stderr);
		exit(-1);
	}
	
	char buffer[BUFFER_SIZE];
	unsigned long fileLine = 0;
	
	//Read the file, create map structure and netlist linked list
	struct ParserNetlist* firstNet = NULL;
	struct ParserNetlist** currentNetPtr = &firstNet;
	unsigned long int netlistSize = 0;
	
	while ( fgets(buffer,sizeof buffer, fp) != NULL) {
		printf("Read line %lu: ",fileLine);
		
		//First line: map size
		if (!fileLine) {
			unsigned long int mapSize;
			if ( sscanf(buffer," %lu ",&mapSize) > 0 ) {
#if(DEBUG_PARSER)
				printf("Get map size: %lu\n",mapSize);
#endif
				*map = createMap(mapSize,mapSize);
			}
			else {
				fputs("Input file bad structure: Cannot get map size.\n",stderr);
				exit(-1);
			}
		}
		
		//Second and so on: obstruction or net
		else {
			unsigned long int x1, y1, x2, y2;
			if ( sscanf(buffer," obstruction %lu %lu ",&x1,&y1) > 0 ) {
#if(DEBUG_PARSER)
				printf("Find obstruction at (%lu,%lu).\n",x1,y1);
#endif
				setMapValueAt(*map,x1,y1,(map_t)-1);
			}
			else if ( sscanf(buffer," net %lu %lu %lu %lu ",&x1,&y1,&x2,&y2) > 0 ) {
#if(DEBUG_PARSER)
				printf("Find net at (%lu,%lu) to (%lu,%lu).\n",x1,y1,x2,y2);
#endif
				*currentNetPtr = (struct ParserNetlist*) malloc(sizeof(struct ParserNetlist));
				(*currentNetPtr)->net = (struct Net*) malloc(sizeof(struct Net));
				((*currentNetPtr)->net)->srcX = x1;
				((*currentNetPtr)->net)->srcY = y1;
				((*currentNetPtr)->net)->destX = x2;
				((*currentNetPtr)->net)->destY = y2;
				currentNetPtr = &((*currentNetPtr)->next);
				*currentNetPtr = NULL;
				
				netlistSize++;
			}
		}
		
		fileLine++;
	}
	fclose(fp);
	
	//Restruct the netlist linked list into array for easy process
	(*netlist).size = netlistSize; //I use "(*struct).child" instead of "struct->child" so I know struct is pass by reference
	(*netlist).nets = (struct Net*) malloc( sizeof(struct Net) * netlistSize );
	currentNetPtr = &firstNet;
	for (unsigned long int i = 0; i < netlistSize; i++) {
		printf("%lu",i);
		printf("=%lu=",(*currentNetPtr)->net->srcX);
		
		(*netlist).nets[i].srcX = (*currentNetPtr)->net->srcX;
		(*netlist).nets[i].srcY = (*currentNetPtr)->net->srcY;
		(*netlist).nets[i].destX = (*currentNetPtr)->net->destX;
		(*netlist).nets[i].destY = (*currentNetPtr)->net->destY;
		puts("xx");
		struct ParserNetlist** justReadNetPtr = currentNetPtr;
		currentNetPtr = &((*currentNetPtr)->next);
		free(justReadNetPtr); //Free the entire linkedlist recursively
	}
	
	for (unsigned long int i = 0; i < netlistSize; i++) {
		printf("Net (%lu,%lu) to (%lu,%lu).\n",(*netlist).nets[i].srcX,(*netlist).nets[i].srcY,(*netlist).nets[i].destX,(*netlist).nets[i].destY);
	}
}