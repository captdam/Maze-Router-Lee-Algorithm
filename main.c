#define MAX_TRY_TIME 20

#define DEBUG 1
#define DEBUG_ROUTER_WAVE 0
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
	
	router(workspaceMap,1);
//	displayMap(workspaceMap);
	saveMap(workspaceMap,uiDelay,"Net 1 placed.");
//	return 0;
	
	router(workspaceMap,2);
//	displayMap(workspaceMap);
	saveMap(workspaceMap,uiDelay,"Net 2 placed.");
	
//	return 0;
	router(workspaceMap,3);
//	displayMap(workspaceMap);
	saveMap(workspaceMap,uiDelay,"Net 3 placed.");
	
	router(workspaceMap,4);
//	displayMap(workspaceMap);
	saveMap(workspaceMap,uiDelay,"Net 4 placed.");
	
	

	
	return 0;
}

//Give the map, return 1 if net placed, return 0 if fail. Map will be modified (mark slots used by net)//, or CONTAMINATED IF FAIL (Map slots is pass by reference)
uint8_t router(Map map, mapdata_t netID) {
	//Find source and dest
	mapdata_t srcX, srcY, destX, destY;
	uint8_t srcFound = 0;
	for (mapaddr_t y = 0; y < map.height; y++) { //Use (*map).height instead of map->height so I remember it is pass by reference
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
				
				else if (x == destX && y == destY) { //For dest
					mapaddr_t surroundingXY[4][2] = {{x,y-1},{x,y+1},{x-1,y},{x+1,y}};
					for (uint8_t i = 0; i < 4; i++) {
						mapaddr_t sx = surroundingXY[i][0], sy = surroundingXY[i][1];
						if (sx >= 0 && sx < map.width && sy >= 0 && sy < map.height) { //Must in the map <-- sx=surroundingX
							if (getMapSlotType(map,sx,sy) == mapslot_wave) { //If a route has reached
								
								mapdata_t traceWave = getMapSlotValue(map,sx,sy);
								mapaddr_t traceX = sx, traceY = sy; //Trade this as pointer when trace back
#if(DEBUG_ROUTER_WAVE)
								printf("--> Route found with distance of %llu!\n",(unsigned long long int)traceWave);
#endif							

								//Trace back
								while (traceWave+1) {
									setMapSlotUsedByNet(map,sx,sy,netID);
									printf("Mark (%llu,%llu) with wave %llu.\n",(unsigned long long int)sx,(unsigned long long int)sy,(unsigned long long int)traceWave);
									mapaddr_t traceSurroundingXY[4][2] = {{sx,sy-1},{sx,sy+1},{sx-1,sy},{sx+1,sy}};
									for (uint8_t j = 0; j < 4; j++) {
										mapaddr_t tsx = traceSurroundingXY[j][0], tsy = traceSurroundingXY[j][1];
										if (tsx >= 0 && tsx < map.width && tsy >= 0 && tsy < map.height) { //Must in the map <-- sx=surroundingX
											if ( getMapSlotType(map,tsx,tsy) == mapslot_wave && getMapSlotValue(map,tsx,tsy) == traceWave ) {
												sx = tsx;
												sy = tsy;
												break; //One route is enough
											}
										}
									}
									traceWave--;
								}
								
								//Clean map
								destroyMap(newMap); //Destroy the workspace map
								cleanMap(map); //Clean all wave
								return 1; //Success
							}
						}
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
