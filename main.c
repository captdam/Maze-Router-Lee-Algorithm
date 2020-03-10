#define MAX_TRY_TIME 20

#define DEBUG 1
#define DEBUG_ROUTER_WAVE 0
#define DEBUG_ROUTER_TRACE 1
#define DEBUG_ROUTER_INTERRESULT 0

#define SAVE_SILENT 1 //Do not pop up the result
#define SAVE_INTERRESULT 1 //Save intermediate result

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "map.c"
#include "xmap.c"
#include "parser.c"
#include "map2html.c"

unsigned long int uiDelay = 0;

void displayMap(Map);
uint8_t router(Map, mapdata_t);

int main(int argc, char* argv[]) {

/*******************************************************************************
1 - Get user input: What file I am gonna to process
*******************************************************************************/

	//What is the net file's name?
	if (!(argc > 1)) {
		fputs("Missing argument 1: Input net file name.\n",stderr);
		return -1;
	}
	
	//What is the interval of map file generation (wait for x seconds after generating map file)? If 0, do not generate map file.
	if (argc > 2) {
		if ( sscanf(argv[2]," %lu ",&uiDelay) > 0 ) {
			printf("GUI enabled, UI delay is %lu second(s).\n",uiDelay);
		}
		else {
			fputs("Argument 2 syntax error: UI delay should be an integer.\n",stderr);
			return -1;
		}
	}
	
	//If generate map file, what browser to use? (Firefox or Chrome)
	if (uiDelay) {
		if (!(argc > 3)) {
			fputs("Missing argument 3: Browser path.\n",stderr);
			return -1;
		}
		char cmd[ strlen(argv[3]) + strlen(" -new-window ./gui.html") + 3 ];
		strcpy(cmd,"\"");
		strcat(cmd,argv[3]);
		strcat(cmd,"\"");
		strcat(cmd," -new-window ./gui.html"); //This works with Firefox, but not with Chrome
		system(cmd);
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
	
	Map workspaceMap = copyMapAsNew(emptyMap);
	saveMap(workspaceMap,uiDelay,"Init.");
	displayMap(workspaceMap);
	
	router(workspaceMap,1);
	displayMap(workspaceMap);
	saveMap(workspaceMap,uiDelay,"Net 1 placed.");
	
	router(workspaceMap,2);
	displayMap(workspaceMap);
	saveMap(workspaceMap,uiDelay,"Net 2 placed.");

	router(workspaceMap,3);
	displayMap(workspaceMap);
	saveMap(workspaceMap,uiDelay,"Net 3 placed.");
	
	router(workspaceMap,4);
	displayMap(workspaceMap);
	saveMap(workspaceMap,uiDelay,"Net 4 placed.");
	
	

	
	return 0;
}

//Give the map, return 1 if net placed, return 0 if fail. Map will be modified (mark slots used by net)//, or CONTAMINATED IF FAIL (Map slots is pass by reference)
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
	Map newMap = copyMapAsNew(map); //New wave should be read in next iteration; therefore, we read from old map and write to new map
	do {
		placeWave = 0; //How mush slots are placed in this iteration
		copyMapM2M(newMap,map);
		
		for (mapaddr_t y = 0; y < map.height; y++) {
			for (mapaddr_t x = 0; x < map.width; x++) {
#if(DEBUG_ROUTER_WAVE)
				printf("--> Check Point (%llu,%llu):\n",(unsigned long long int)x,(unsigned long long int)y);
#endif
				
				if (getMapSlotType(map,x,y) == mapslot_free) { //For all free slot
					
					//Find the correct wave value that should be write to the current slot
					struct makeWaveDataXch {
						mapaddr_t srcX;
						mapaddr_t srcY;
						mapdata_t shouldBePlacedWave;
						
					};
					void makeWave(Map map, mapaddr_t x, mapaddr_t y, void* dataStruct) {
#if(DEBUG_ROUTER_WAVE)
						printf("----> Check neighbor (%llu,%llu): ",(unsigned long long int)x,(unsigned long long int)y);
#endif
						
						//Case 1 - Find surrounding slot is net src
						if (x == (*(struct makeWaveDataXch*)dataStruct).srcX && y == (*(struct makeWaveDataXch*)dataStruct).srcY ) {
							(*(struct makeWaveDataXch*)dataStruct).shouldBePlacedWave = 1;
#if(DEBUG_ROUTER_WAVE)
							fputs("Found SRC: Wave = 1.",stdout);
#endif
						}
						
						//Case 2 - Find surrounding slot is a wave
						else if (getMapSlotType(map,x,y) == mapslot_wave) {
							mapdata_t surroundingWave = getMapSlotValue(map,x,y);
							mapdata_t swp = surroundingWave + 1;
							if ( (*(struct makeWaveDataXch*)dataStruct).shouldBePlacedWave == 0 ) { //Unplaced
								(*(struct makeWaveDataXch*)dataStruct).shouldBePlacedWave = swp;
#if(DEBUG_ROUTER_WAVE)
								printf("Found nearby wave: Wave = %llu.",(unsigned long long int)swp);
#endif
							}
							else { //[R] Placed, then compare
								if ( (*(struct makeWaveDataXch*)dataStruct).shouldBePlacedWave > swp) {
									(*(struct makeWaveDataXch*)dataStruct).shouldBePlacedWave = swp;
#if(DEBUG_ROUTER_WAVE)
									printf(" > Found nearby wave (smaller): Wave = %llu.",(unsigned long long int)swp);
#endif
								}
							}
						}
						
#if(DEBUG_ROUTER_WAVE)
						puts("");
#endif
					}
					
					struct makeWaveDataXch dataXch;
					dataXch.srcX = srcX;
					dataXch.srcY = srcY;
					dataXch.shouldBePlacedWave = 0; //Unplaced, because we will not place zero-wave. so we use 0 as a flag
					applyNeighbor(map, x, y, makeWave, &dataXch);
					
					if (dataXch.shouldBePlacedWave) { //Placed
						setMapSlotWave(newMap,x,y,dataXch.shouldBePlacedWave);
						placeWave++;
					}
				}
				
				else if (x == destX && y == destY) { //For dest
					
					//Check if any path reaches the dest
					struct traceBackinitDataXch {
						mapaddr_t traceX;
						mapaddr_t traceY;
						mapdata_t traceWave;
						uint8_t pathReached;
					};
					void traceBackInit(Map map, mapaddr_t x, mapaddr_t y, void* dataStruct) {
						if (getMapSlotType(map,x,y) == mapslot_wave) {
							mapdata_t traceWave = getMapSlotValue(map,x,y);
							if ( (*(struct traceBackinitDataXch*)dataStruct).pathReached == 0 ) {
								(*(struct traceBackinitDataXch*)dataStruct).pathReached = 1;
								(*(struct traceBackinitDataXch*)dataStruct).traceWave = traceWave;
								(*(struct traceBackinitDataXch*)dataStruct).traceX = x;
								(*(struct traceBackinitDataXch*)dataStruct).traceY = y;
							}
							else { //[R] Multiple path reached, find the one with lowest cost
								if (traceWave < (*(struct traceBackinitDataXch*)dataStruct).traceWave) {
									(*(struct traceBackinitDataXch*)dataStruct).traceWave = traceWave;
									(*(struct traceBackinitDataXch*)dataStruct).traceX = x;
									(*(struct traceBackinitDataXch*)dataStruct).traceY = y;
								}
							}
						}
					}
					
					struct traceBackinitDataXch dataXch;
					dataXch.pathReached = 0;
					applyNeighbor(map, x, y, traceBackInit, &dataXch);
					
					if (dataXch.pathReached) {
#if(DEBUG)
						printf("--> Route found with distance of %llu!\n",(unsigned long long int)dataXch.traceWave);
#endif

						//Find a nearby slot that has the correct wave value
						struct traceBackDataXch {
							mapaddr_t nextX;
							mapaddr_t nextY;
							mapdata_t nextWave;
						};
						void traceBack(Map map, mapaddr_t x, mapaddr_t y, void* dataStruct) {
							if ( getMapSlotType(map,x,y) == mapslot_wave && getMapSlotValue(map,x,y) == (*(struct traceBackDataXch*)dataStruct).nextWave ) {
								printf("------> S: (%llu,%llu)\n",(unsigned long long int)x,(unsigned long long int)y);
								(*(struct traceBackDataXch*)dataStruct).nextX = x;
								(*(struct traceBackDataXch*)dataStruct).nextY = y;
							}
						}

						struct traceBackDataXch dataXch2;
						dataXch2.nextX = dataXch.traceX;
						dataXch2.nextY = dataXch.traceY;
						dataXch2.nextWave = dataXch.traceWave;
						
						while (dataXch2.nextWave) {
#if(DEBUG_ROUTER_TRACE)
							printf("----> Mark (%llu,%llu) with wave %llu.\n",(unsigned long long int)dataXch2.nextX,(unsigned long long int)dataXch2.nextY,(unsigned long long int)dataXch2.nextWave);
#endif
							dataXch2.nextWave--; //Multiple route may be found, so nextWave-- cannot be placed in the function
							setMapSlotUsedByNet(map, dataXch2.nextX, dataXch2.nextY, netID);
							applyNeighbor(map, dataXch2.nextX, dataXch2.nextY, traceBack, &dataXch2);
							
						}
						
						//Clean map
						destroyMap(newMap); //Destroy the workspace map
						cleanMap(map); //Clean all wave
						return 1; //Success
					}
				}
			}
		}
		
		copyMapM2M(map,newMap);
#if(DEBUG_ROUTER_INTERRESULT)
		printf("> Placed %llu slots.\n",placeWave);
		displayMap(map);
#endif
		saveMap(map,uiDelay,"Waving...");
	} while (placeWave);
	
#if(DEBUG_ROUTER_WAVE)
	puts("--> FAIL! Cannot place route");
#endif

	//Clean up map and quit
	destroyMap(newMap); //Destroy the workspace map
	cleanMap(map); //Clean all wave
	return 0; //Fail
}

//Display map in cmd
void displayMap(Map map) {
	printf("--> Map: (@%llx)\n",map.map); //Check memory leak
	printf("--> Size = %llu * %llu\n", (unsigned long long int)map.width, (unsigned long long int)map.height);
	for (mapaddr_t y = 0; y < map.height; y++, puts("")) {
		for (mapaddr_t x = 0; x < map.width; x++) {
			printf("%17llx",(unsigned long long int)getMapValueAt(map,x,y));
		}
	}
}
