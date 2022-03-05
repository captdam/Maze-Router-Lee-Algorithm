```diff
- This repo was used for my school project. It is deactived because I have finished that course.
```

# Maze Router: Lee Algorithm

 The Lee Algorithm is a routing algorithm used to find shortest route from one point to another point in a maze. In VLSI and FPGA design, the Lee Algorithm guarantee to find the shortest path for a single net if it exists, but the cost of this algorithm is quite high.
 
This program is an implementation of the Lee Algorithm in C language for single layer universal cost maze map.

The code of this program has been tested on Windows 10 64-bit (compiled by MinGW GCC) and Ununtu 64-bit. It should work on other versions of Windows and Linux system. Web browser is required for GUI.


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

```cd src```

```gcc -o3 ./main.c -o ../bin/router```
 
Then, copy the config file and GUI file to the same directory as the executable file. In Linux, use the following command:

```cp ./config.cfg ../bin/config.cfg```

```cp ./gui.html ../bin/gui.html```

Now, there should be 3 files in the ```bin``` directory, the executable file ```router``` (or ```router.exe``` in Windows), the config file ```config.cfg``` and the GUI file ```gui.html```.

## Executing the program

Before executing the program, user should first check the config file, especially the ```gui_path_command``` setting. Description is given in the config file.

The program can be executed in 3 modes:

| Mod | ``` gui_path_command``` | ``` gui_interresult``` | Comment |
|---|---|---|---|
| 1 | path_to_browser | 0 | GUI shows how the router place nets in order |
| 2 | path_to_browser | 1 | GUI shows how the router place nets in order and propagate the wave |
| 3 | manual | done_not_care | GUI will not open automatically |

Note: In mode 3, GUI will not be opened automatically, but the result file will always be exported. Depending on the ```gui_ interresult``` setting, the program exports result every time the router place a net, or every time the router place a net and propagate the wave. This is helpful during debugging. If the user wants no GUI, it is recommended to set the ```gui_dely``` to 0. Therefore, the program will not pause every time when a result file (including intermediate result file) generated.

After setting the config file, now, the program can be executed. To do so, use the following command:

```./router path_to_input.nets```

The first argument of the program defines the input netsfile’s location. The net file is a human-readable text file contains map size, x-y position of obstruction and x-y position of net source and sink. An example of netsfile is given below:

```
20 //The first line defines the size of the maze map. In this case, a 10 by 10 square maze
20 x 15 //For rectangular maze, user a “x” to separate the width and height
obstruction 3 12 //x position of an obstruction, y position of that obstruction
obstruction 18 13
net 3 12 12 5 //Net source x-y position, net sink x-y position
```

If there is no second argument, the software will use a random seed (in fact, it is the current time) to define the behaviour of random number generator used in the program. To define the seed, use the following command:

``` ./router path_to_input.nets abc```

In this case, the program will use the char code of the first character in the second argument as the seed. In another word, the ASCII code of character 'a' will be used to generate random number in the program.

If the ```gui_path_command``` is configed correctly in the config file, a browser window should pop-up, displaying a table. As the program runs, the table will be changed to illustrate how the software finding the solution for the maze routing. If ```gui_interresult``` is configured to 1 in the config file, the GUI will show how the program propagate the wave.

Although the Lee algorithm guarantees the shortest path for a single net if exist, the best solution for multiple nets is unknown. Placing one net may block another net. This problem is nP-hard. If the program found a way to connect all nets, the process will stop immediately; otherwire, the program will run for several times, depending on the amount of nets and the ```max_retry_index``` setting, then give the best result (which connects most nets).

After the program finding the best solution, the best solution should be displayed in the GUI window. Meanwhile, the result will be saved as an HTML table in ```map.html``` in the same directory as the program. The result will be saved in file even if GUI is disabled. By doing so, user can use some text processing tool or software to read the result file for other programs.

# Implementation

## Program Architecture

This program can be divided into two parts, the back-end routing program and the front-end GUI. The back-end routing program is implemented in C, because program write in C has higher performance. When the given maze map and nets count are large, C program generally takes way less runtime compare to programs in other languages. The front-end GUI is implemented HTML with embedded CSS, because HTML/CSS GUI is extremely easy to write.

When the user executes the program with GUI enabled, the program will first open the GUI. Then, the GUI will constantly be polling the result file and display the maze map and the router’s progress in the GUI window. At the same time, the program will apply the Lee Algorithm and generate result file. In this way, when there is any progress done by the back-end router, the back-end router will export the result file, and the front-end GUI poll it and display it on the screen.

```
----------                           ------------                     -------
| Router |  ---- Export result --->  | map.html |  ---- Polling --->  | GUI | 
----------                           ------------                     -------
```

## Data Structure

The old terminology ```slot``` is replaced by a more precise terminology ```tile``` in this document, but the old term is still used in the code.

The old terminology ```wave``` is replaced by a more precise terminology ```propagate``` in this document, but the old term is still used in the code.

### Map-object Structure

Although C is not an object-orientation programming language, object-orientation programming can be manually achieved. In this program, a ```Map``` object is used to represent the maze map. The ```Map``` structures contains three elements, the height of the map, the width of the map, and the tiles of the map. The structure of ```Map``` object is given below:
```C
typedef struct Map {
	mapaddr_t width;
	mapaddr_t height;
	mapdata_t* map;
} Map;
```

The map width and height are of type ```mapaddr_t``` and the tiles are of type ```mapdata_t```. User can define the type ```mapaddr_t``` and ```mapdata_t``` when compile the program to fit their own need.

Type ```mapaddr_t``` is used to describe the x-y coordinates of the map tiles. When define the data type, user can use ```unsigned short int```, ```unsigned int``` or ```unsigned long long int```. This data type should be big enough to describe the x-y coordinates of any tile in the map. A ```unsigned int``` or ```uint32_t``` should be good enough since most map should be less than 4G * 4G.

Type ```mapdata_t``` is used to describe the state of each map tile. This includes “free”, “obstruction”, propagate, and the ID of net using this tile. When define the data type, user can use ```unsigned short int```, ```unsigned int``` or ```unsigned long long int```. The bit length of this data type should be greater than twice of the map size or the amount of net, which is greater. User should be caution when define the data type. If this data type is too large, it consumes a lot of memory; if it is too small, it may overflow.
```
2 ^ bit_length( mapdata_t ) >= 2 * max( count( map_size ) , count( nets_size ) )
```

For the structure ```Map```, its child ```map``` is used to represent the tiles of the map. ```map``` is a array (logical 2D array, but use 1D array pointer), the index of the array is used to represent the x-y position of that tile. For example, to ```map[ y * Map.width + x ]``` is used to access the tile at ```(x,y)```.

Each tile can be one of the following states:
- Free. Which means this tile is free to use for any net. All bits will be zero in this case.
- Propagate. This is used when finding the shortest path from net source to net drain using the Lee algorithm. In this case, the first bit (MSB) will be zero, and the remaining bits represent the distance of that tile from the net source.
- Obstruction. Which means this tile can not be used for any purpose. In this case, the first bit (MSB) will be one, and the remaining bits are all zero.
- Used by net. This means the tile is occupied by a net. In this case, the first bit (MSB) will be one, and the remaining bits represent the net ID.

An ```emun``` is used to describe those 4 situations:
```C
typedef enum mapslot_type {
	mapslot_free,
	mapslot_wave,
	mapslot_obstruction,
	mapslot_net
} mapslot_type;
/*slot should be tile*/
```

### Map-object Method

One way to implement object-oriented method is to pass a pointer of the object structure to the function. This syntax is simular to object-oriented programming using Python.
```C
return_type method_name (struct object_type* object_name, params…) {…}
```

In this program, there are two constructors defined for the ```Map``` object in ```map.c```. These constructors will ask the system to allocate memory to hold the maze map and return a ```Map``` object.
```C
Map createMap(mapaddr_t width, mapaddr_t height);	//To create an empty map, all tiles in this map are empty (not used)
Map copyMapAsNew(Map srcMap);				//To create a map exactly like the given map
```

Because the program manually allocates (using ```malloc``` or ```calloc```) memory, a destructor is required to release the memory if a ```Map``` is no longer required. Otherwise, the program will suffer from memory leak issue.
```C
void destroyMap(Map targetMap); //Destroy a map, release the map tile memory space
```

The map object has a setter and a getter method used to set or get the value at any tile of the map according to its x-y position. These two methods are designed to be used internally; in another word, these are private functions, because the return value is raw unsigned number. There are two major reasons:
- The main program should interactive with the ```Map``` object at an abstract level. The main program does not understand how to process those raw number, and the main program does not need to understand it.
- If there is any change of the representation of the map tile, the code in main program must be changed as well. This is extremely time-consuming and error-prone.
```C
mapdata_t getMapValueAt(Map readMap, mapaddr_t x, mapaddr_t y);			//Get the content of a tile in the map, using x-y position
void setMapValueAt(Map writeMap, mapaddr_t x, mapaddr_t y, mapdata_t value);	//Set the content of a tile in the map, using x-y position
```

There are some methods that are designed for main program to interact with the ```Map``` object. One thing needs to mention is, although these methods accept the ```Map``` object by value, the ```Map``` object contains a pointer to the map tiles. Therefore, the map tiles are pass by reference in fact. Modify the map tiles inside method affect the map tiles outside of the method.
```C
void copyMapM2M(Map destMap, Map srcMap);						//To copy the content of one map to another, without create a new one, map should be same size
void setMapSlotObstruction(Map writeMap, mapaddr_t x, mapaddr_t y);			//Set a tile to be obstruction
void setMapSlotUsedByNet(Map writeMap, mapaddr_t x, mapaddr_t y, mapdata_t netID);	//Set a tile to be used by a net
void setMapSlotWave(Map writeMap, mapaddr_t x, mapaddr_t y, mapdata_t wave);		//by propagate (distance)
void setMapSlotFree(Map writeMap, mapaddr_t x, mapaddr_t y);				//free
mapslot_type getMapSlotType(Map checkMap, mapaddr_t x, mapaddr_t y);			//Get map tile state (free, wave, obstruction, used by net)
mapdata_t getMapSlotValue(Map checkMap, mapaddr_t x, mapaddr_t y);			//After check the map tile type, get the value of it, either netID or propagate distance, or 0 for free/obstruction
void cleanMap(Map map);									//Clean a map, remove all propagates
```

### Map-object Neighbor Wrap Method

In the Lee Algorithm, there are lot of operation that requiring the accessing of 4 neighborhood tiles of a given tile. Therefore, the ```Map``` object provides a wrap method which accept another custom method. In this wrap method, the wrap method will apply the custom method to all 4 neighborhood tiles of the given tile. To pass and retrieve data between the main function and the custom method, the wrap method accepts a pointer to a custom structure. Furthermore, the wrap method check for boundary as well. For example, if the given tile's x position is 0, the wrap will not apply the custom function to its left neighbor.
```C
void applyNeighbor(Map map, mapaddr_t selfX, mapaddr_t selfY, void (*function)(), void *functionData);
```

There is an example of using the wrap method:
```C
struct xDataXch { //The custom structure used to pass and retrieve information
	mapaddr_t data1;
	uint8_t data2;
};
void xFunction(Map map, mapaddr_t x, mapaddr_t y, void* dataStruct) { //The custom method
	(*(struct xDataXch*)dataStruct).data1 = doSomething1();
	(*(struct xDataXch*)dataStruct).data2 = doSomething2();
}

struct xDataXch dataXch; //Init the custom structure
dataXch.data2 = 0;
applyNeighbor(map, 2, 3, xFunction, &dataXch); //Apply the wrap method at map(2,3)

if (dataXch.data2) { //Using the data retrieved from the custom method
	doSomething3(dataXch.data1);
}
```

The wrap also comes with a random factor. In this wrap method, the wrap applies the custom method on all 4 neighbor tiles. However, if there is any conflict, the result comes from the last or first applied tile will win. Sometime, this may lead to a poor result. Consider the following situation:

<img src="https://raw.githubusercontent.com/captdam/Maze-Router-Lee-Algorithm/master/docs/randompath_x.jpg" alt="Fail without random" width="300px">

The router has successfully route net 1, net 2 and net3; but cannot route net 4. For net 1, assume the left-bottom is source and the right-up is the sink. As the map shows, vertical path has higher priority than horizontal path when the router place route for net 1. However, this blocks the path for net 4. If the router can place the path as the rad lines, all 4 nets can be successfully placed.

<img src="https://raw.githubusercontent.com/captdam/Maze-Router-Lee-Algorithm/master/docs/randompath_v.jpg" alt="Fail without random" width="300px">

With a random factor added, the router is able to place all 4 nets. The random factor can be set in the config file. Set the ```neighbor_random_index``` to zero will disable the randomization.

## Lee router

This section describes the Lee router. The Lee router is a router used to route a single net. For the algorithm used to route the entire nets set, refer to the next section, the ```Main Program (Back-end router)```.

The router method is in ```main.c```. The router will return true (1) if a route for the given net is found, and the route will be marked in the map. If no route can be found, this method returns false (0).
```C
uint8_t router(Map map, mapdata_t netID);
```

### Step 1 - Finding Source and Sink

The router method accepts two parameters, the map and the net ID.

For each net, there is one source and one sink. The router will scan the entire map and find the x-y positions for both net source and sink according to the net ID paramter.

### Step 2 - Propagating

Starting from the net source, the router begins to propagate. All unmarked (free) map tiles near (connected by edge, but not corner) the source will be marked "propagate 1”, all unmarked map slots near “propagate 1” will be marked as “propagate 2”, and so on. The process will continue until the net sink is reached, or no more map slots can be marked.
```
run = 0;
markedTiles = [source];
repeat {
	run++;
	markedTiles = markedTiles.neighbor;
	foreach (markedTiles)
		markOnMap(markedTiles.elements,run);
} until( markedTiles.count == 0 || markedTiles.includes(sink) );
```

This method requires query, or flex-length array to story marked tiles in each run. It is easy to implement in high-level language such as JavaScript, Java or Python; however, in C, there is no such flex data structure. There are few solutions:
- Using fixed-length very big array. This solution consumes a lot of memory. The term “very big” may not be big enough.
- Using linked list. Hard to program, easy to have memory leak.

Therefore, instead of finding neighbor of marked tiles (by using marked tiles position), the program finds tiles that are neighbor of marked tiles (by fully scan the map).
```
repeat {
	markCount = 0;
	foreach(mapTiles) {
		if (mapTiles.element is source.neighbor) {
			markOnMap(mapTiles.element,1);
			markCount++;
		}
		else if (mapTiles.element is markedTiles.element.neighbor) {
			markOnMap(mapTiles.element, markedTiles.element.value + 1);
			markCount++;
		}
	}
} until (markCount == 0 || sink.is_marked);
```

The second method requires to scan the entire map in each run, but the first method (the queue/linked list method) only access those tiles modified by last run and their neighbor. In another word, the first method accesses less tiles than the second method in each run. Especially in the first run, the first method only access 4 tiles (neighbor of source), but the second method accesses all tiles in the map.

However, when the map is crowd, the tiles accessed by first method will be just slightly less than the second method. In fact, flex-length array (such as linked list and hash map) has overhead. Assume the program is using linked list to implement the queue. When reading the array, the CPU needs to read the ```next``` pointer, then go to fetch the value by pointer. After each run, the program needs to walk through the linked list to destroy (release memory of) the array. Furthermore, linked list is usually saved in HEAP across different memory sectors, which means higher cache miss rate. For the second method, the CPU only need to perform a read and a pointer register increase operation. (In fact, some CPUs can increase pointer and read value in one cycle, e.g. ```LD Rd, Z+``` in AVR). The map is saved in HEAP but all in one continuous space, which means higher cache hit rate.

### Step 3 - Tracing Back

After the propagate reaches the net sink, the program begins to trace back from the sink to the source to establish the route. When doing this, there may be multiple possible shortest routes, all of them has the same length. In this case, the program uses a random number to determine which path to use. Refer to the ``` Implementation --> Data Structure --> Map-object Neighbor Wrap Method``` section.

## Main Program (Back-end Router)

The main program is in ```mian.c```.

### Step 1 - Initialization and Read Config

When the user executes the program, the first thing the program will do is to initialize itself, including checking the user input from command line, set the random seed, and load the config file.

The user must give at least one argument in the command line to the program, which is the input file’s path. If this argument is missed, the program will exit and print error information in ```stderr```.

Second argument is optional, which is used to set the seed for random number generator. If the user does not provide this argument, the program will use current time as the seed of the random number generator. If the user provides this argument, the first character of this option will be used. For example, if user’s command is ```./router path_to_net_file.net abc```, then the ASCII code for character ‘a’ will be used. In another word, 0x61 is used as the seed.

If the user gives more than 2 arguments in the command line, those extra arguments will be ignored.

The program load config from the config file ```config.cfg``` which is in the same directory as the program. The config file is used to define the behavior of the program, such as GUI, delay, random factor etc. If the config file is missed or cannot be read by the program, the program will exit and print error information in ```stderr```.

In the config file, if the ``` gui_path_command``` config is set correctly and not “manual”, the program will call the operating system to open web browser to display GUI. No matter the GUI is enabled or disabled or misconfigured, the program always export result file.

### Step 2 - Generate Empty Map

The second step of the program is to read the net file and generate the empty map. The empty map is a map that contains all the obstructions and source and sinks of all nets.

A parser (in ```parser.c```) is used to read the input net file. The parser will return a ```Map``` object that contains the empty map.

The parser first read the first line of the input net file.
- If this line contains two numbers n and m, separated by a "x", the parser will create a rectangle map with size n by m.
- if this line contains only one number n, the parser will create a square map with size n by n.

After the parser creating the map, the parser begins to search for nets and obstructions line by line.
- If a line contains only two numbers, these two number are considered x-y position of an obstruction. In this case, the obstruction will be marked on the map. 
- If a line contains four numbers, the fist two number are considered x-y position for the net source, the last two number are considered x-y position for the net sink. In this case, this net will be assigned an ID (starting from 1) and marked on the map. When marking the map, there is no indication of net sink or source, because electrons can travel in both direction in route; therefore, it is not required to identify the net source and sink.

For the netsfile, comments are not allowed. Number in comments may misleading the parser.

The parser also counts for the number of nets, and return the net count to main program by reference.

The following graph shows an empty map:

<img src="https://raw.githubusercontent.com/captdam/Maze-Router-Lee-Algorithm/master/docs/emptymap.jpg" alt="Empty map Init map" width="300px">

### Step 3 - Router

Now, it is time to route the nets on the maze map. However, before the router begins work, the program will shuffle the net list. In another word, the order of placing nets will be different in each try.

The shuffle is done by using a random number, the random factor ```priority_random_index``` can be set in the config file. Set this value to 0 will disable it. There is a very small chance that the order remains the same after shuffle due to the random number generator behavior.

Then, the program will call the router method to route the maze map, one net at a time. If the router can successfully route all nets, the program will export that map as the solution and quit. If the router cannot place all nets, the program will retry for several times until the retry limit is hit or the router successfully placed all the nets. If the retry limit is hit, the program will export the result which placed most of the nets. The retry limit is determine by both the nets count and the ```max_retry_index``` in the config file.

When the program retries, the program will shuffle the net list, and:
- If the first net cannot be placed (for example, the source or net is separated by obstruction), the fist net to place in next try will be a random net.
- Otherwise, the fist net to place in the next try will be the fail net of last try, because that net is the most difficult one to place.

The reason of shuffling the net list is to find a relatively good order of placing maximum amount of net. Assume there is 5 nets, net 2 go from the most top to most bottom; net 3 go from most left to most right. In another word, net 2 and net 3 will interlock each other. Although there is some better algorithm to find the good order, such as recursive walking; using random number is the most simple way.

| Try | Placement order | Comment |
| --- | --- | --- |
| Try 1 | 3->5->1->2 | Cannot place net 2, retry. 3 nets placed |
| Try 2 | 2->1->3 | Cannot place net 3, retry. 2 nets placed |
| ... | ... | ... |
| Try X | 1->5->4->2->3 | Cannot place net 3, retry. 4 nets placed |
| ... | ... | ... |
| Try Z | ... | Retry limit reached. Give up |

Try X placed maximum amount of nets, using Try X as result.

## GUI

### GUI

The GUI interface is an HTML file ```gui.html```.

As describe in ``` Implementation --> Program Architecture``` section, the front-end GUI of this program is a webpage. The back-end router program will route the net and generate map file; the front-end GUI will read the map file and display it on screen.

However, the front-end/back-end architecture brings a critical issue. The GUI is running in web browser, which means the GUI is running in a sandbox. The GUI may read file, but it cannot write file. In another word, the back-end router GUI and the front-end GUI are communicating in a one-way link. The issue is that, the GUI cannot send signal to the router; therefore, the router may modify the result file while the GUI is reading it. 

The original solution is to have a JavaScript checker on the front-side. Every time when the front-end GUI reading the result file, the checker will first check the integrity of the result file. If this file is not modified by the back-end router during reading, the result will be displayed; if it is modified, the checker discards it.

This solution works fine few years ago, when CORS in Firefox is not strict as today (At that time, there was CORS, but Firefox allows JavaScript program in local HTML file to read files in the same directory or subdirectories). However, nowadays, this solution is no longer available due to security reason. If a JavaScript program can read a file from local file system, it can send the local file to a remote server. This means, if a user downloads and runs an HTML file, critical information saved in downloading directory can be stolen (for example, a user may have a key file downloaded from another website). Therefore, the checker solution will not work.

To accompany with the CORS policy, the only solution is to create a local web server on the user’s machine so an HTTP ```Access-Control-Allow-Origin``` can be send with the result file. However, it is not worth to pack such a complex web server with the program.

The final solution is to let the back-end router to export the result as a ```<table>``` in HTML file and let the front-end GUI to use a ```<iframe>``` tag to embed the result HTML file.

In this architecture, the result file can be embedded in the GUI directly by the DOM. There is no JavaScript program involved in this process; therefore, the CORS rule is not breached. To do this, the back-end router needs to export the result as a displayable HTML file so it can be directly displayed. This method is saved in ```map2html.c```.

However, there is still chance of write-while-read. In this case, the result file will have a bad HTML syntax. What the user see will be a blank screen in the GUI. This will affect the user experience, but it is not fatal.


### Setting the GUI

Generally speaking, the back-end router is extremely fast. To illustrate the progress of the router, the router has to implement a delay function at each step; otherwise, the result will be found instantly.

By default, the back-end router exports the result at a rate of around 2.5 results per second. In acother word, paue for 400 miniseconds every times. If the map size is large, the rate my drop a little bit. The rate can be changed by editing the ``` gui_delay ``` config.

The front-end GUI is constantly polling the result file. To ensure the front-end GUI polls all the progress, the rate of polling should be equal or higher to double of the rate of back-end GUI (according to the Nyquist theorem). By default, the rate is 5 sampling per second (sample every 200 miniseconds).

If the user wants to modify the sampling rate of the front-end GUI, the user needs to modify the ```gui.html``` file. In this file, modify the value of ```refresh```:
```html
refresh = 200; //Modify this line if gui_delay in config.cfg is smaller than 400
setInterval( () => {
document.getElementById("maploader1").src += '';
},refresh);
```

On some low-performance machine, user may experience heavy lag in the GUI. In this case, the user should slow down both rates.

# Testbench

## File1

The first test file conatins a 15-by-15 maze map, with 22 obstructions and 4 nets to route.
```
15

obstruction 3 12
obstruction 3 11
obstruction 3 10
obstruction 3 3
obstruction 3 4
obstruction 3 5
obstruction 3 6
obstruction 4 6
obstruction 5 6
obstruction 9 10
obstruction 10 10
obstruction 11 10
obstruction 11 9
obstruction 11 8
obstruction 6 2
obstruction 7 2
obstruction 8 2
obstruction 8 5
obstruction 9 5
obstruction 10 5
obstruction 11 5
obstruction 12 5

net 3 12 12 5
net 3 6 10 10
net 8 5 8 2
net 3 3 3 10
```

This is how the map looks like before placement:

<img src="https://raw.githubusercontent.com/captdam/Maze-Router-Lee-Algorithm/master/docs/1_init.png" alt="Empty map Test 1" width="600px">

The program successfully place all nets:

<img src="https://raw.githubusercontent.com/captdam/Maze-Router-Lee-Algorithm/master/docs/1_final.png" alt="Empty map Test 1" width="600px">

## File2

The maze in the second test file is also 15 by 15; however, this one comes with 38 obsturctions and 6 nets to route.
```
15

obstruction 0 9
obstruction 1 9
obstruction 2 9
obstruction 2 3
obstruction 2 4
obstruction 2 5
obstruction 3 3
obstruction 3 4
obstruction 3 5
obstruction 5 13
obstruction 6 13
obstruction 7 13
obstruction 8 13
obstruction 6 8
obstruction 7 8
obstruction 8 8
obstruction 6 9 
obstruction 7 9
obstruction 8 9
obstruction 6 10
obstruction 7 10
obstruction 8 10
obstruction 6 1
obstruction 7 1
obstruction 10 3
obstruction 11 3
obstruction 12 3
obstruction 10 4
obstruction 11 4
obstruction 12 4
obstruction 10 5
obstruction 11 5
obstruction 12 5
obstruction 12 8
obstruction 12 9
obstruction 12 10
obstruction 12 11
obstruction 12 12

net 3 3 10 4
net 7 1 7 13
net 2 9 10 5
net 3 4 10 3
net 2 5 6 1
net 12 8 12 12
```

This is how the map looks like before placement:

<img src="https://raw.githubusercontent.com/captdam/Maze-Router-Lee-Algorithm/master/docs/2_init.png" alt="Empty map Test 2" width="600px">

By analysis this maze, it is clear to say that, it is impossible to place all nets on this map. Looking on the top-left obstrcution and top-right obstruction, if net 1 and 4 is placed at the beginning, one end of net 5 or net 3 will be enclosed.

Now, run the program. The program cannot place all nets, so it retries. After 12 itreations, the program gives up and returns the best result it can found:

<img src="https://raw.githubusercontent.com/captdam/Maze-Router-Lee-Algorithm/master/docs/2_final.png" alt="Empty map Test 2" width="600px">

