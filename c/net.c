#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//Structure used to represents a net
typedef struct Net {
	mapaddr_t srcX;
	mapaddr_t srcY;
	mapaddr_t destX;
	mapaddr_t destY;
} Net;

//A link list used by the parser
typedef struct ParserNetlist {
	Net net;
	struct ParserNetlist* next;
} ParserNetlist;

//An array of net used by main program
typedef struct Netlist {
	mapdata_t size;
	Net* nets; //Array of net ptr
} Netlist;