#define MAX_TRY_TIME 20

#define DEBUG 1

#define SAVE_SILENT 1 //Do not pop up the result
#define SAVE_INTERRESULT 1 //Save intermediate result

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "map.c"
#include "net.c"
#include "parser.c"

mapdata_t router(Map, Net);

int main(int argc, char* argv[]) {

/*******************************************************************************
1 - Get user input: What file I am gonna to process
*******************************************************************************/

	if (argc < 2) {
		fputs("Missing argument: Input file\n",stderr);
		return -1;
	}
	
	time_t currentTime;
	time(&currentTime);
	struct tm* localTime = localtime(&currentTime);
	printf("Task start: %s\n",asctime(localTime));

/*******************************************************************************
2 - Read the empty map and netlist, empty map contains only obstructions
*******************************************************************************/

	printf("Reading netsfile: %s\n",argv[1]);

	Map emptyMap = parser(argv[1]);	
	printf("Map size = %llu * %llu\n", (unsigned long long int)emptyMap.width, (unsigned long long int)emptyMap.height);
#if(DEBUG)
	puts("Given map value:");
	for (mapaddr_t i = 0; i < emptyMap.height; i++) {
		for (mapaddr_t j = 0; j < emptyMap.width; j++) {
			printf("%17llx",(unsigned long long int)getMapValueAt(emptyMap,j,i));
		}
		puts("");
	}
	puts("Given map type:");
	for (mapaddr_t i = 0; i < emptyMap.height; i++) {
		for (mapaddr_t j = 0; j < emptyMap.width; j++) {
			printf("%17d",getMapSlotType(emptyMap,j,i));
		}
		puts("");
	}
#endif
return 0;

/*******************************************************************************
3 - Router routine
*******************************************************************************/
/*	unsigned long int priorityNetIndex = 0; //The failed net in last design will be placed first
	
	unsigned long int bestResultNetCount = 0;
	struct Map bestMap;
	
	for (unsigned long int currentTry = 0; currentTry < MAX_TRY_TIME; currentTry++) {
		printf("\nDesign %lu:\n",currentTry);
		
		//Create new empty design
		struct Map workMap = copyMap(emptyMap);
		
		//Place the priority net
		Net net1 = netlist[priorityNetIndex];
#if(DEBUG)
		printf("--> Place first net: (%lu,%lu) to (%lu,%lu)\n",net1.srcX,net1.srcY,net1.destX,net1.destY);
#endif
		uint8_t ok = router(workMap,net1);
		
		if (!ok) { //We cannot even place the first net: Next time, start from another random net, hope we can place as much as possible nets
			unsigned long int newPriority;
			do {
				newPriority = rand() % netlist.size;
			} while(newPriority == priorityNetIndex);
			priorityNetIndex = newPriority;
			continue;
		}
		
		//Place all reminding nets
		for (unsigned long int netIndex = 0; netIndex < netlist.size; netIndex++) {
			if (netIndex == priorityNetIndex) continue; //Do not place the first net again
			
			ok = router(workMap,net1);
			
			if (!ok) { //Failed: set the current net to be priority for next try
				priorityNetIndex = netIndex;
			}
		}
		
		//All nets placed
		
		
		
		
		
		
		
		
		
		
		
		
	}*/
}

mapdata_t router(Map map, Net net) {/*
	unsigned long int newWaveLength = 1;
	for (map_t i = 1;;i++) {
		if (i == MAP_T_HALF) { //Distance exceed the limit
			return i;
		}
		
		if (i == 1) { //First does: start from the src
			setMapSlotWave(map, net.srcX, net.srcY, 1);
		}
		
		else {
			for (unsigned long int x = 0; x < map.length; i++) {
				for (unsigned long int y = 0)
			}
		}
	}*/
}


