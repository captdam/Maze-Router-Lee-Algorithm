#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//Read a file contains map size, obstructions and nets, return the map
Map parser(char* filename, mapdata_t *netCount) {
	
	FILE* fp = fopen(filename,"r");
	if (!fp) {
		fputs("Cannot read file.\n",stderr);
		exit(-1);
	}
	
	char buffer[BUFFER_SIZE];
	unsigned long long int fileLine = 0;
	Map map;
	mapdata_t netID = 1;
	uint8_t mapCreated = 0;
	
	while ( fgets(buffer,sizeof buffer, fp) != NULL) {
#if(DEBUG_PARSER)
		printf("Read line %llu: ",fileLine);
#endif
		
		//First line: map size
		if (!mapCreated) {
			unsigned long long int mapSizeX, mapSizeY;
			if ( sscanf(buffer," %llu x %llu ",&mapSizeX,&mapSizeY) > 1 ) {
#if(DEBUG_PARSER)
				printf("Get map size: %llu * %llu.",mapSizeX,mapSizeY);
#endif
				map = createMap((mapaddr_t)mapSizeX,(mapaddr_t)mapSizeY);
				mapCreated = 1;
			}
			else if ( sscanf(buffer," %llu ",&mapSizeX) > 0 ) {
#if(DEBUG_PARSER)
				printf("Get map size: %llu.",mapSizeX);
#endif
				map = createMap((mapaddr_t)mapSizeX,(mapaddr_t)mapSizeX);
				mapCreated = 1;
			}
			else {
#if(DEBUG_PARSER)
				fputs("Skip",stdout);
#endif
			}
		}
		
		//Second line and so on: obstruction or net
		else {
			unsigned long long int x1, y1, x2, y2;
			if ( sscanf(buffer," obstruction %llu %llu ",&x1,&y1) > 0 ) {
#if(DEBUG_PARSER)
				printf("Find obstruction at (%llu,%llu).",x1,y1);
#endif
				setMapSlotObstruction(map,(mapaddr_t)x1,(mapaddr_t)y1);
			}
			else if ( sscanf(buffer," net %llu %llu %llu %llu ",&x1,&y1,&x2,&y2) > 0 ) {
#if(DEBUG_PARSER)
				printf("Find net %llu at (%llu,%llu) to (%llu,%llu).",(unsigned long long int)netID,x1,y1,x2,y2);
#endif
				setMapSlotUsedByNet(map,(mapaddr_t)x1,(mapaddr_t)y1,netID);
				setMapSlotUsedByNet(map,(mapaddr_t)x2,(mapaddr_t)y2,netID);
				netID++;
			}
			else {
#if(DEBUG_PARSER)
				fputs("Skip",stdout);
#endif
			}
		}

#if(DEBUG_PARSER)
		puts("");
#endif
		fileLine++;
	}
	
	fclose(fp);
	*netCount = netID - 1;
	return map;
}
