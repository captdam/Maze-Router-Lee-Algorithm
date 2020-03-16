# Maze Router: Lee Algorithm
 Single Layer Maze Router using Lee Algorithm.

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
 
Then, copy the config file and GUI file to the same directory as the executable file:

```cp ./config.cfg ../bin/config.cfg```

```cp ./gui.html ../bin/gui.html```

## Executing the program
Before run the program, first check the config file, especially the ```gui_path_command``` setting. Description is given in the config file.

After setting the config file, now the program can be executed. To do so, use the following command:

```./router path_to_input.nets```

In this case, the software will use a random seed (in fact, it is the current time) to define the behaviour of random number generator used in the program.

To define the seed, use the following command:

``` ./router path_to_input.nets abc```

In this case, the program will use the char code of the first character in the second argument as the seed. In another word, the ASCII code of character 'a' will be used to generate random number in the program.

If the ```gui_path_command``` is configed correctly in the config file, a browser window should pop-up, displaying a table. As the program runs, the table will be changed to illustrate how the software finding the solution for the maze routing. If ```gui_interresult``` is configured to 1 in the config file, the browser will show how the program apply the Lee algorithm to find the path for each net.

Although the Lee algorithm guarantees the best solution for a single net if exist, the best solution for multiple nets is unknown. Placing one net may block another net. This is problem is nP-hard. If the program found a way to connect all nets, the process will stop immediately; otherwire, the program will run for several times, depending on the amount of nets and the ```max_retry_index``` setting, then give the best result (which connects most nets).

# Implementation

## Data structure

### Map-object structure

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

For the structure ```Map```, its child ```map``` is used to represent the slots of the map. ```map``` is a 1-D array, the index of the array is used to represent the x-y position of that slot. For example, to ```map[ y * Map.length + x ]``` is used to access the slot at ```(x,y)```. Each slot can be one of the following types:
- Free. Which means this slot is free to use for any net. All bits will be zero in this case.
- Wave. This is used when finding the shortest path from net source to net drain using the Lee algorithm. In this case, the first bit (MSB) will be zero, and the remaining bits represent the distance of that slot from the net source.
- Obstruction. Which means this slot can not be used for any purpose. In this case, the first bit (MSB) will be one, and the remaining bits are all zero.
- Used by net. This means the slot is occupied by a net. In this case, the first bit (MSB) will be one, and the remaining bits represent the net ID.

### Map-object method
One way to implement object-oriented method is to pass a pointer of the object structure to the function. This syntax is simular to object-oriented programming using Python.
```C
Return_type method(struct object_name* object, other_params…) {…}
```
