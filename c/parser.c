#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 255

#ifndef DEBUG
#define DEBUG 1
#endif

//Read a file contains map size, obstructions and nets, return the map
Map parser(char* filename) {
	
	FILE* fp = fopen(filename,"r");
	if (!fp) {
		fputs("Cannot read file.\n",stderr);
		exit(-1);
	}
	
	char buffer[BUFFER_SIZE];
	unsigned long long int fileLine = 0;
	Map map;
	mapdata_t netID = 1;
	
	while ( fgets(buffer,sizeof buffer, fp) != NULL) {
		printf("Read line %llu: ",fileLine);
		
		//First line: map size
		if (!fileLine) {
			unsigned long long int mapSize;
			if ( sscanf(buffer," %llu ",&mapSize) > 0 ) {
#if(DEBUG_PARSER)
				printf("Get map size: %llu.",mapSize);
#endif
				map = createMap((mapaddr_t)mapSize,(mapaddr_t)mapSize);
			}
			else {
				fputs("Input file bad structure: Cannot get map size.\n",stderr);
				exit(-1);
			}
		}
		
		//Second line and so on: obstruction or net
		else {
			unsigned long long int x1, y1, x2, y2;
			if ( sscanf(buffer," obstruction %llu %llu ",&x1,&y1) > 0 ) {
#if(DEBUG)
				printf("Find obstruction at (%llu,%llu).",x1,y1);
#endif
				setMapSlotObstruction(map,(mapaddr_t)x1,(mapaddr_t)y1);
			}
			else if ( sscanf(buffer," net %llu %llu %llu %llu ",&x1,&y1,&x2,&y2) > 0 ) {
#if(DEBUG)
				printf("Find net %llu at (%llu,%llu) to (%llu,%llu).",(unsigned long long int)netID,x1,y1,x2,y2);
#endif
				setMapSlotUsedByNet(map,(mapaddr_t)x1,(mapaddr_t)y1,netID);
				setMapSlotUsedByNet(map,(mapaddr_t)x2,(mapaddr_t)y2,netID);
				netID++;
			}
			else {
				fputs("Input file bad structure: Cannot understant input file.\n",stderr);
				exit(-1);
			}
		}
		
		puts("");
		fileLine++;
	}
	
	fclose(fp);
	return map;
}