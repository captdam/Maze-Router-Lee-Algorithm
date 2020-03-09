#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* This file contains subroutines that is not native map routine
 * but is useful for the routing routine
 * Consider map.c as a ganaral map libeary, and this is an router extension
*/

//Clean a map, remove all waves
void cleanMap(Map map) {
	for (mapaddr_t i = 0; i < map.height; i++) {
		for (mapaddr_t j = 0; j < map.width; j++) {
			if (getMapSlotType(map,j,i) == mapslot_wave)
				setMapSlotWave(map,j,i,0); //Wave 0 is free
		}
	}
}