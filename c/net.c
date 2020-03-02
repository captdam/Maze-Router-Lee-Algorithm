#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//Structure used to represents a net
struct Net {
	unsigned long int srcX;
	unsigned long int srcY;
	unsigned long int destX;
	unsigned long int destY;
};

//A link list used by the parser
struct ParserNetlist {
	struct Net* net;
	struct ParserNetlist* next;
};

//An array of net used by main program
struct Netlist {
	unsigned long int size;
	struct Net* nets; //Array of net ptr
};