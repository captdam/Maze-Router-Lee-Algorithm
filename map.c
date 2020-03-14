#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//Date type used for each slot in the map
typedef unsigned long int mapdata_t;

/* What data type should be use?
 * Bigger data type is useful if the map is large,
 * because this data is used to trace the distance from net source to drain and obstruction info.
 * Smaller data type saves memory space (and gives good cache performance),
 * and may be accelerated by SIMD or MMX instruction. 
 * MAX ROUTE DISTANCE FROM SOURCE TO DRAIN IS mapdata_t / 2 - 1
 * MAX NUMBER OF NETS IS mapdata_t / 2 - 1
*/

#define MAPDATA_T_HALF (~(((mapdata_t)-1)>>1)) //~0b0111111...=0b10000000...

//Data type used to represent the x-y position of map
typedef unsigned long int mapaddr_t;

/* What data type should be used?
 * This data type should be able to hold the width and height of the map.
 * Generally speaking, the length of this data type should be half of the map_t for NxN table.
 * But, it is safer to make this larger, it has a very small impact on performance and memory consumption.
*/

//Structure used to represents a map
typedef struct Map {
	mapaddr_t width;
	mapaddr_t height;
	mapdata_t* map;
} Map;

/* About the map
 * Width and height represent the size of the map.
 * *map is a ptr to the map, which is a 2D array (but saved in 1D structure), saved in heap (created by malloc).
 * Map is of type mapdata_t, defined at the beginning of the file.
 * Map[y*width+x] is:
	0x8000...		This slot is an obstruction. (MAPDATA_T_HALF)
	0x8000...~(unsigned)-1	This slot is used by net (netID = this - 0x8000...)	
	0:			This net is free to use
	Others:			Wave (distance to source)
*/

/* NOTE IMPORTANT: /////////////////////////////////////////////////////////////
 * Map data (slots) is saved in HEAP and access by POINTER
 * This means, passing Map by VALUE acturally passing pointer to map slots.
 * Modify map slots in function WILL affect the map slots data out of the function.
*///////////////////////////////////////////////////////////////////////////////

mapdata_t getMapValueAt(Map, mapaddr_t, mapaddr_t);
void setMapValueAt(Map, mapaddr_t, mapaddr_t, mapdata_t);

/************* Constructor *************/

//To create an empty map, all slots in this map are empty (not used)
Map createMap(mapaddr_t width, mapaddr_t height) {
	Map newMap;
	newMap.width = width;
	newMap.height = height;
	newMap.map = calloc(width * height, sizeof(mapdata_t));
	if (newMap.map == NULL) {
		fputs("SYSTEM ERROR\tCannot alloc memory.",stderr);
	}
	return newMap;
}

//To create a map exactly like the given map
Map copyMapAsNew(Map srcMap) {
	Map newMap;
	newMap.width = srcMap.width;
	newMap.height = srcMap.height;
	newMap.map = malloc( sizeof(mapdata_t) * newMap.width * newMap.height );
	if (newMap.map == NULL) {
		fputs("SYSTEM ERROR\tCannot alloc memory.",stderr);
	}
	for (mapaddr_t i = 0; i < newMap.height; i++) {
		for (mapaddr_t j = 0; j < newMap.width; j++) {
			setMapValueAt(newMap, j, i, getMapValueAt(srcMap, j, i));
		}
	}
	return newMap;
}

//To copy the content of one map to another, without create a new one, map should be same size
void copyMapM2M(Map destMap, Map srcMap) {
	if (destMap.width != srcMap.width || destMap.height != srcMap.height) {
		fputs("Map not aligned.\n",stderr);
		exit(-2);
	}
	for (mapaddr_t i = 0; i < destMap.height; i++) {
		for (mapaddr_t j = 0; j < destMap.width; j++) {
			setMapValueAt(destMap, j, i, getMapValueAt(srcMap, j, i));
		}
	}
}

//Destroy a map, release the map slot memory space
void destroyMap(Map targetMap) {
	free(targetMap.map);
}

/************* Getter, setter, checker *************/

//Set a slot to be obstruction
void setMapSlotObstruction(Map writeMap, mapaddr_t x, mapaddr_t y) {
	setMapValueAt(writeMap,x,y,MAPDATA_T_HALF);
}

//Set a slot to be used by a net
void setMapSlotUsedByNet(Map writeMap, mapaddr_t x, mapaddr_t y, mapdata_t netID) {
	setMapValueAt(writeMap,x,y,MAPDATA_T_HALF|netID);
}

void setMapSlotWave(Map writeMap, mapaddr_t x, mapaddr_t y, mapdata_t wave) {
	setMapValueAt(writeMap,x,y,(MAPDATA_T_HALF-1)&wave);
}

void setMapSlotFree(Map writeMap, mapaddr_t x, mapaddr_t y) {
	setMapValueAt(writeMap,x,y,0);
}

//Get map slot attribute (free, wave, obstruction, used by net)
typedef enum mapslot_type {mapslot_free, mapslot_wave, mapslot_obstruction, mapslot_net} mapslot_type;
mapslot_type getMapSlotType(Map checkMap, mapaddr_t x, mapaddr_t y) {
	mapdata_t value = getMapValueAt(checkMap,x,y);
	if (!value)				return mapslot_free;
	else if (value == MAPDATA_T_HALF)	return mapslot_obstruction;
	else if (value&MAPDATA_T_HALF)		return mapslot_net;
	else					return mapslot_wave;
}

//After check the map slot type, get the value of it, either netID or wave, or 0 for free/obstruction
mapdata_t getMapSlotValue(Map checkMap, mapaddr_t x, mapaddr_t y) {
	return getMapValueAt(checkMap,x,y) & (MAPDATA_T_HALF-1);
}

/************* Util *************/

//Get the content of a slot in the map, using x-y position
mapdata_t getMapValueAt(Map readMap, mapaddr_t x, mapaddr_t y) {
	return readMap.map[ y * readMap.width + x ];
}

//Set the content of a slot in the map, using x-y position
void setMapValueAt(Map writeMap, mapaddr_t x, mapaddr_t y, mapdata_t value) {
	writeMap.map[ y * writeMap.width + x ] = value;
}

