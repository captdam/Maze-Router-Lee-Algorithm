#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//Date type used for each slot in the map
typedef unsigned long int map_t;
/* What data type should be use?
Bigger data type is useful if the map is large,
because this data is used to trace the distance from net source to drain.
Smaller data type saves memory space (and gives good cache performance),
and may be accelerated by SIMD or MMX instruction. 
*/

//Structure used to represents a map
struct Map {
	unsigned long int width;
	unsigned long int height;
	map_t* map;
};
/* About the map
Width and height represent the size of the map.
*map is a ptr to the map, which is a 2D array (but saved in 1D structure), saved in heap (created by malloc).
Map is of type map_t, defined at the beginning of the file.
Map[y*width+x] is:
	0:		This net is free to use
	(unsigned)-1:	Used
	Others:		Wave (distance to source)
*/

map_t getMapValueAt(struct Map, unsigned long int, unsigned long int);
void setMapValueAt(struct Map, unsigned long int, unsigned long int, map_t);

/************* Constructor *************/

//To create an empty map, all slots in this map are empty (not used)
struct Map createMap(unsigned long int width, unsigned long int height) {
	struct Map newMap;
	newMap.width = width;
	newMap.height = height;
	newMap.map = calloc(width * height, sizeof(unsigned long int));
	return newMap;
}

//To create a map exactly like the given map
struct Map copyMap(struct Map srcMap) {
	struct Map newMap;
	newMap.width = srcMap.width;
	newMap.height = srcMap.height;
	newMap.map = malloc( sizeof(unsigned long int) * newMap.width * newMap.height );
	for (unsigned long int i = 0; i < newMap.height; i++) {
		for (unsigned long int j = 0; j < newMap.width; j++) {
			setMapValueAt(newMap, j, i, getMapValueAt(srcMap, j, i));
		}
	}
	return newMap;
}

/************* Getter, setter, checker *************/

//Set obstructor

/************* Util *************/

//Get the content of a slot in the map, using x-y position
map_t getMapValueAt(struct Map readMap, unsigned long int x, unsigned long int y) {
	return readMap.map[ y * readMap.width + x ];
}

//Set the content of a slot in the map, using x-y position
void setMapValueAt(struct Map writeMap, unsigned long int x, unsigned long int y, map_t value) {
	writeMap.map[ y * writeMap.width + x ] = value;
}

