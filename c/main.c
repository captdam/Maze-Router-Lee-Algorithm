#define MAX_TRY_TIME 20

#define DEBUG 1
#define DEBUG_ROUTER_WAVE 1

#define SAVE_SILENT 1 //Do not pop up the result
#define SAVE_INTERRESULT 1 //Save intermediate result

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "map.c"
#include "parser.c"

void displayMap(Map);
uint8_t router(Map, mapdata_t);

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
#if(DEBUG)
	displayMap(emptyMap);
#endif

/*******************************************************************************
3 - Router routine
*******************************************************************************/
	
	Map workspaceMap = copyMap(emptyMap);
	router(workspaceMap,1);
	
	
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
	
	return 0;
}

//Give the map, return 1 if net placed, return 0 if fail.
uint8_t router(Map map, mapdata_t netID) {
	//Find source and dest
	mapdata_t srcX, srcY, destX, destY;
	uint8_t srcFound = 0;
	for (mapaddr_t y = 0; y < map.height; y++) {
		for (mapaddr_t x = 0; x < map.width; x++) {
			if ( getMapSlotType(map,x,y) == mapslot_net && getMapSlotValue(map,x,y) == netID ) {
				if (!srcFound) { //Find the first point: src
					srcX = x;
					srcY = y;
					srcFound = 1;
				}
				else {
					destX = x;
					destY = y;
					x = map.width; //All requirements found, stop search
					y = map.height;
				}
			}
		}
	}
#if(DEBUG)
	printf("--> SRC (%llu,%llu). DEST (%llu,%llu)\n",(unsigned long long int)srcX,(unsigned long long int)srcY,(unsigned long long int)destX,(unsigned long long int)destY);
#endif
	
	//Wave
	unsigned long long int placeWave;
	do {
		placeWave = 0; //How mush slots are placed in this iteration
		Map newMap = copyMap(map); //New wave should be read in next iteration; therefore, we read from old map and write to new map
		
		for (mapaddr_t y = 0; y < map.height; y++) {
			for (mapaddr_t x = 0; x < map.width; x++) {
				if (getMapSlotType(map,x,y) == mapslot_free) { //For all free slot
					
					mapaddr_t surroundingXY[4][2] = {{x,y-1},{x,y+1},{x-1,y},{x+1,y}}; //Up, down, right, left
					for (uint8_t i = 0; i < 4; i++) {
						mapaddr_t sx = surroundingXY[i][0], sy = surroundingXY[i][1];
						if (sx >= 0 && sx < map.width && sy >= 0 && sy < map.height) { //Must in the map <-- sx=surroundingX
							
							//Case 1 - Find surrounding slot is net src
							if (sx == srcX && sy == srcY) {
								setMapSlotWave(newMap,x,y,1);
								placeWave++;
#if(DEBUG_ROUTER_WAVE)
								printf("--> Found SRC: Set (%llu,%llu) to Wave 1.\n",(unsigned long long int)x,(unsigned long long int)y);
#endif
								break; //For sure, this is the cloest path if this slot is beside the src
							}
							
							//Case 2 - Find surrounding slot is a wave
							else if ( getMapSlotType(map,sx,sy) == mapslot_wave ) {
								mapdata_t surroundingWave = getMapSlotValue(map,sx,sy);
								
								//Current slot has not been write, write wave = surrounding wave + 1
								if (getMapSlotType(newMap,x,y) == mapslot_free) {
									setMapSlotWave(newMap,x,y,surroundingWave+1);
									placeWave++;
#if(DEBUG_ROUTER_WAVE)
									printf("--> Found Wave %llu (current empty): Set (%llu,%llu) to Wave %llu.\n",(unsigned long long int)surroundingWave,(unsigned long long int)x,(unsigned long long int)y,(unsigned long long int)surroundingWave+1);
#endif
								}
								
								//Current slot has been placed, compare
								else {
									mapdata_t selfWave = getMapSlotValue(map,x,y);
									if (selfWave > surroundingWave + 1) {
										setMapSlotWave(newMap,x,y,surroundingWave+1);
										/* placeWave++; */ //We just replace a wave, we did NOT place new wave
#if(DEBUG_ROUTER_WAVE)
										printf("--> Found Wave (current larger): Set (%llu,%llu) to Wave 1.\n",(unsigned long long int)x,(unsigned long long int)y,(unsigned long long int)surroundingWave+1);
#endif
									}
								}
							}
						}
					}
					
				}
				
				else if (x == destX && y == destY) { //For dest, check if it has been connected to a route
					mapaddr_t surroundingXY[4][2] = {{x,y-1},{x,y+1},{x-1,y},{x+1,y}}; //Up, down, right, left
					for (uint8_t i = 0; i < 4; i++) {
						mapaddr_t sx = surroundingXY[i][0], sy = surroundingXY[i][1];
						if (getMapSlotType(map,sx,sy) == mapslot_wave) {
							mapdata_t traceWave = getMapSlotValue(map,sx,sy);
							mapaddr_t traceX = sx, traceY = sy;
#if(DEBUG_ROUTER_WAVE)
							printf("--> Route found with distance of %llu!\n",(unsigned long long int)traceWave);
#endif							


							//Trace back
/*							do {
								
							} while ();*/
							return 1;
						}
					}
				}
			}
		}
		
		map = newMap;
		
		//Reach dest?
		/**/
		
#if(DEBUG)
		printf("> Placed %llu slots.\n",placeWave);
		displayMap(map);
#endif
	} while (placeWave);
	return 0; //Fail
}

void displayMap(Map map) {
	puts("--> Map:");
	printf("--> Size = %llu * %llu\n", (unsigned long long int)map.width, (unsigned long long int)map.height);
	for (mapaddr_t y = 0; y < map.height; y++, puts("")) {
		for (mapaddr_t x = 0; x < map.width; x++) {
			printf("%17llx",(unsigned long long int)getMapValueAt(map,x,y));
		}
	}
}


