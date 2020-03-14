# Maze Router: Lee Algorithm
 Single Layer Maze Router using Lee Algorithm.

# Installing

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

# Executing the program
Before run the program, first check the config file, especilly the ```gui_path_command``` setting. Description is given in the config file.

After setting the config file, now the program can be executed. To do so, use the following command:

```./router path_to_input.nets```

In this case, the software will use a random seed (in fact, it is the current time) to define the behaviour of random number generator used in the program.

To define the seed, use the following command:

``` ./router path_to_input.nets abc```

In this case, the program will use the char code of the first character in the second argument as the seed. In another word, the ASCII code of character 'a' will be used to generate random number in the program.

If the ```gui_path_command``` is configed correctly in the config file, a browser window should pop-up, displaying a table. As the program runs, the table will be changed to illustrate how the software finding the solution for the maze routing. If ```gui_interresult``` is configed to 1 in the config file, the browser will show how the program apply the Lee algrothem to find the path for each net.

Althrough the Lee algorithm guarantees the best solution for a single net if exist, the best solution for multiple nets is unknown. Placing one net may block another net. This is problem is nP-hard. If the program found a way to connect all nets, the process will stop immediately; otherwire, the program will run for several times, depending on the amount of nets and the ```max_retry_index``` setting, then give the best result (which connects most nets).
