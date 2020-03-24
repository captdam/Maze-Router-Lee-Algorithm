# Maze Router: Lee Algorithm

 The Lee Algorithm is a routing algorithm used to find shortest route from one point to another point in a maze. In VLSI and FPGA design, the Lee Algorithm guarantee to find the shortest path for a single net if it exists, but the cost of this algorithm is quite high.
 
This program is an implementation of the Lee Algorithm in C language for single layer universal cost maze map.


# Using the program

## Installing

This program is writing in C and compiled by GCC. The followig files are required to compile the program:
- ```mian.c```: This is the main code, including config file reader and router function.
- ```map.c```: This file contains map class, which is used to store the maze map.
- ```xmap.c```: This file contains extra functions to work with the map.
- ```map2html.c```: This is used to export map to HTML for GUI.
- ```parser.c```: This is used to read netsfile.

The following files are required to run the program:
- ```config.cfg```: This file defines the behaviour of the program.
- ```gui.html```: This HTML file is used for GUI.

Assume all the files are saved in the following directory tree:
```
router_root
  |- bin
  |--- (empty)
  |- src
  |--- main.c
  |--- map.c
  |--- xmap.c
  |--- map2html.c
  |--- parser.c
  |--- config.cfg
  |--- gui.html
```

To install this program, first go to the router_root/src directory, and using GCC to compile the C files:

```gcc -o3 ./main.c -o ../bin/router```
 
Then, copy the config file and GUI file to the same directory as the executable file. In Linux, use the following command:

```cp ./config.cfg ../bin/config.cfg```

```cp ./gui.html ../bin/gui.html```

Now, there should be 3 files in the ```bin``` directory, the executable file ```router``` (or ```router.exe``` in Windows), the config file ```config.cfg``` and the GUI file ```gui.html```.

## Executing the program

Before run the program, first check the config file, especially the ```gui_path_command``` setting. Description is given in the config file.

After setting the config file, now, the program can be executed. To do so, use the following command:

```./router path_to_input.nets```

In this case, the software will use a random seed (in fact, it is the current time) to define the behaviour of random number generator used in the program.

To define the seed, use the following command:

``` ./router path_to_input.nets abc```

In this case, the program will use the char code of the first character in the second argument as the seed. In another word, the ASCII code of character 'a' will be used to generate random number in the program.

If the ```gui_path_command``` is configed correctly in the config file, a browser window should pop-up, displaying a table. As the program runs, the table will be changed to illustrate how the software finding the solution for the maze routing. If ```gui_interresult``` is configured to 1 in the config file, the browser will show how the program apply the Lee algorithm to find the path for each net.

Although the Lee algorithm guarantees the shortest path for a single net if exist, the best solution for multiple nets is unknown. Placing one net may block another net. This problem is nP-hard. If the program found a way to connect all nets, the process will stop immediately; otherwire, the program will run for several times, depending on the amount of nets and the ```max_retry_index``` setting, then give the best result (which connects most nets).

After the program finding the best solution, the best solution should be displayed in the GUI window. Meanwhile, the result will be saved as an HTML table in ```map.html``` in the same directory as the executable file. The result will be saved in file even if GUI is disabled. By doing so, user can use some text processing tool or software to read the result file for other programs.

# Implementation

## Program Architecture

This program can be divided into two parts, the back-end routing program and the front-end GUI. The back-end routing program is implemented in C, because program write in C has higher performance. When the given maze map and nets count are large, C program generally takes way less runtime compare to programs in other languages. The front-end GUI is implemented HTML with embedded CSS, because HTML/CSS GUI is extremely easy to write.

When the user executes the program with GUI enabled and intermediate result exportation enabled, the program will first open the GUI. Then, the GUI will constantly be polling the result file and display the maze map and the router’s progress in the GUI window. At the same time, the program will apply the Lee Algorithm and generate result file. In this way, when there is any progress done by the back-end router, the back-end router will export the result file, and the front-end GUI poll it and display it on the screen.

However, the front-end/back-end architecture brings a critical issue. The GUI is running in web browser, which means the GUI is running in a sandbox. The GUI may read file, but it cannot write file. In another word, the back-end and front-end is communicate in a single-way link. Because the GUI cannot send signal to the router, the router may modify the result file while the GUI is reading it.

The original solution is to have a JavaScript checker on the front-side. Every time when the front-end GUI reading the result file, the checker will first check the integrity of the result file. If this file is not modified by the back-end during reading, the result will be displayed; if it is modified, the checker discard it.

This solution works fine few years ago, when CORS in Firefox is not strict as today. However, for modern web browser, this solution is not allowed for security reason. If a JavaScript program can read a file from local file system, it can send the local file to a remote server. This means, critical information saved in local file (for example, the ```/etc/passwd```) can be stolen. Therefore, the checker solution will not work.

To accompany with the CORS policy, the only solution is to create a local web server on the user’s machine so an HTML ```Access-Control-Allow-Origin``` can be send alone with the GUI file. However, it is not worth to pack such a complex web server with the program.

The final solution is to let the back-end router export result as a table in HTML file, and let the front-end GUI use a ```<iframe>``` tag to embed the result HTML file. In this way, the front-end GUI can display the maze map, and there is no JavaScript reading the result file to breach the CORS rule. There still is chance that the back-end router modifies the result file while the front-end GUI read it. In this case, the false file with bad HTML syntax will be read and provided a blank screen in the GUI.

In experiment, Chrome seems to have poor performance than Firefox in Win10; Firefox in Linux seems to ignore the false result file because no blank screen has been reported on Linux. This is not confirmed, because the hardware used on Windows and Linux test machine is different.

```
----------                           ------------                     -------
| Router |  ---- Export result --->  | map.html |  ---- Polling --->  | GUI | 
----------                           ------------                     -------
```

## Data Structure

### Map-object Structure

Although C is not an object-orientation programming language, object-orientation programming can be manually achieved. In this program, a ```Map``` object is used to represent the maze map. The ```Map``` structures contains three elements, the height of the map, the width of the map, and the slots of the map. The structure of is given below:
```C
typedef struct Map {
	mapaddr_t width;
	mapaddr_t height;
	mapdata_t* map;
} Map;
```

The map width and height are of type ```mapaddr_t``` and the slots are of type ```mapdata_t```. User can define the type ```mapaddr_t``` and ```mapdata_t``` when compile the program to fit their own requirement.

Type ```mapaddr_t``` is used to describe the x-y position of the map slots. When define the data type, user can use ```unsigned short int```, ```unsigned int```, ```unsigned long int``` or ```unsigned long long int```. This data type should be big enough to describe the x-y position of any slot in the map. It is recommended to use ```unsigned long long int```, because this is the safeties data type, and this attribute has minor impact on performance and memory consumption.
```
2 ^ bit_length( mapaddr_t ) >= count( map_size )
```

Type ```mapdata_t``` is used to describe the content of each map slot. This includes “free”, “obstruction”, “wave”, and the ID of net using this slot. When define the data type, user can use ```unsigned short int```, ```unsigned int```, ```unsigned long int``` or ```unsigned long long int```. The length of this data type should be greater than twice of the map size or the amount of net, which is greater. User should be caution when define the data type. If this data type is too large, it consumes a lot of memory; if it is too small, it will overflow.
```
2 ^ bit_length( mapdata_t ) >= 2 * max( count( map_size ) , count( nets_size ) )
```

For the structure ```Map```, its child ```map``` is used to represent the slots of the map. ```map``` is a 1-D array, the index of the array is used to represent the x-y position of that slot. For example, to ```map[ y * Map.length + x ]``` is used to access the slot at ```(x,y)```.

Each slot can be one of the following types:
- Free. Which means this slot is free to use for any net. All bits will be zero in this case.
- Wave. This is used when finding the shortest path from net source to net drain using the Lee algorithm. In this case, the first bit (MSB) will be zero, and the remaining bits represent the distance of that slot from the net source.
- Obstruction. Which means this slot can not be used for any purpose. In this case, the first bit (MSB) will be one, and the remaining bits are all zero.
- Used by net. This means the slot is occupied by a net. In this case, the first bit (MSB) will be one, and the remaining bits represent the net ID.

An ```emun``` is used to describe those 4 situations:
```C
typedef enum mapslot_type {
	mapslot_free,
	mapslot_wave,
	mapslot_obstruction,
	mapslot_net
} mapslot_type;
```

### Map-object Method

One way to implement object-oriented method is to pass a pointer of the object structure to the function. This syntax is simular to object-oriented programming using Python.
```C
Return_type method(struct object_name* object, other_params…) {…}
```

In this program, there are two constructors defined for the ```Map``` object in ```map.c``` and ```xmap.c```. These constructors will ask the system to allocate memory to hold the maze map and return a ```Map``` object.
```C
Map createMap(mapaddr_t width, mapaddr_t height);	//To create an empty map, all slots in this map are empty (not used)
Map copyMapAsNew(Map srcMap);				//To create a map exactly like the given map
```

Because the program manually allocates (using ```malloc``` or ```calloc```) memory, a destructor is required to release the memory if a ```Map``` is no longer required. Otherwise, the program will suffer from memory leak issue.
```C
void destroyMap(Map targetMap); //Destroy a map, release the map slot memory space
```

The map object has a setter and a getter method used to set or get the value at any slot of the map according to its x-y position. These two methods are designed to be used internally; in another word, these are private functions, because the return value is raw unsigned number. There are two major reasons:
- The main program should interactive with the ```Map``` object at an abstract level. The main program does not understand how to process those raw number, and the main program does not need to understand it.
- If there is any change of the representation of the map slot, the code in main program must be changed as well. This is extremely time-consuming and error-prone.
```C
mapdata_t getMapValueAt(Map readMap, mapaddr_t x, mapaddr_t y);			//Get the content of a slot in the map, using x-y position
void setMapValueAt(Map writeMap, mapaddr_t x, mapaddr_t y, mapdata_t value);	//Set the content of a slot in the map, using x-y position
```

There are some methods that are designed for main program to interact with the ```Map``` object. One thing needs to mention is, although these methods accept the ```Map``` object by value, the ```Map``` object contains a pointer to the map slots. Therefore, the ```Map``` object is pass by reference in fact. Modify the map slots inside method affect the map slots outside of the method.
```C
void copyMapM2M(Map destMap, Map srcMap);						//To copy the content of one map to another, without create a new one, map should be same size
void setMapSlotObstruction(Map writeMap, mapaddr_t x, mapaddr_t y);			//Set a slot to be obstruction
void setMapSlotUsedByNet(Map writeMap, mapaddr_t x, mapaddr_t y, mapdata_t netID);	//Set a slot to be used by a net
void setMapSlotWave(Map writeMap, mapaddr_t x, mapaddr_t y, mapdata_t wave);		//by wave (distance)
void setMapSlotFree(Map writeMap, mapaddr_t x, mapaddr_t y);				//free
mapslot_type getMapSlotType(Map checkMap, mapaddr_t x, mapaddr_t y);			//Get map slot attribute (free, wave, obstruction, used by net)
mapdata_t getMapSlotValue(Map checkMap, mapaddr_t x, mapaddr_t y);			//After check the map slot type, get the value of it, either netID or wave, or 0 for free/obstruction
void cleanMap(Map map);									//Clean a map, remove all waves
```

### Map-object Neighbor Wrap method

In the Lee Algorithm, there are lot of operation that requiring the accessing of 4 neighborhood slots of a given slot. Therefore, the ```Map``` object provides a wrap method which accept another custom method. In this wrap method, the wrap method will apply the custom method to all 4 neighborhood slots of the given slot. To pass and retrieve data between the main function and the custom method, the wrap method accepts a pointer to a custom structure. Furthermore, the wrap method check for boundary as well. For example, if the given slot's x position is 0, the wrap will not apply the custom function to its left neighbor.
```C
void applyNeighbor(Map map, mapaddr_t selfX, mapaddr_t selfY, void (*function)(), void *functionData);
```

There is an example of using the wrap method:
```C
struct traceBackinitDataXch { //The custom structure used to pass and retrieve information
	mapaddr_t traceX;
	mapaddr_t traceY;
	mapdata_t traceWave;
	uint8_t pathReached;
};
void traceBackInit(Map map, mapaddr_t x, mapaddr_t y, void* dataStruct) { //The custom method
	if (getMapSlotType(map,x,y) == mapslot_wave) {
		mapdata_t traceWave = getMapSlotValue(map,x,y);
		if ( (*(struct traceBackinitDataXch*)dataStruct).pathReached == 0 ) {
			(*(struct traceBackinitDataXch*)dataStruct).pathReached = 1;
			(*(struct traceBackinitDataXch*)dataStruct).traceWave = traceWave;
			(*(struct traceBackinitDataXch*)dataStruct).traceX = x;
			(*(struct traceBackinitDataXch*)dataStruct).traceY = y;
		}
		else { //Multiple path reached, find the one with lowest cost
			if (traceWave < (*(struct traceBackinitDataXch*)dataStruct).traceWave) {
				(*(struct traceBackinitDataXch*)dataStruct).traceWave = traceWave;
				(*(struct traceBackinitDataXch*)dataStruct).traceX = x;
				(*(struct traceBackinitDataXch*)dataStruct).traceY = y;
			}
		}
	}
}

struct traceBackinitDataXch dataXch; //Init the custom structure
dataXch.pathReached = 0;
applyNeighbor(map, 2, 3, traceBackInit, &dataXch); //Apply the wrap method at map(2,3)

if (dataXch.pathReached) {Something();} //Using the data retrieved from the custom method
```

The wrap also comes with a random factor. In this wrap method, the wrap applies the custom method on all 4 neighbor slots. However, if there is any conflict, the result comes from the last or first applied slot will win. Sometime, this may lead to a poor result. Consider the following situation:

<img src="https://raw.githubusercontent.com/captdam/Maze-Router-Lee-Algorithm/master/docs/randompath_x.jpg" alt="Fail without random" width="300px">

The router has successfully route net 1, net 2 and net3; but cannot route net 4. Assume the router are finding route from up-left to bottom-right. As the map shows, vertical path has higher priority than horizontal path when the router place route for net 1. However, this blocks the path for net 4. If the router can place the path as the read lines shows, all 4 nets can be successfully placed.

<img src="https://raw.githubusercontent.com/captdam/Maze-Router-Lee-Algorithm/master/docs/randompath_v.jpg" alt="Fail without random" width="300px">

With a random factor added, the length of path for net 1, net 2 and net 3 is not changed; but the router is able to place the path for net 4 as well.

