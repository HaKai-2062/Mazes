# Mazes

## Features
- Dockable window and maze is automatically adjusted according to available viewport space.
- Mazes aka labyrinth can be generated using 4 building algorithms: Recursive Backtrack, Kruskal, Prints, Wilson.
- 4 solving algorithms can be used to get path from start cell to end cell: DFS, BFS, Dijkstra, A(*) star.
- Maze dimensions including cell width and wall width can be tweaked.
- Colors of the maze can be tweaked and path drawn has color cycling by default. It can be further tweaked by user.
- Visualize tab includes cell weights which can be used to assign random weights. Useful for Dijkstra and A star alogirthms.

## Libraries
- [GLFW](https://github.com/glfw/glfw)
- [Glad (gl 3.3, OpenGL, Core profile)](https://glad.dav1d.de/)
- [imgui (docking branch)](https://github.com/ocornut/imgui/tree/docking)
- [implot](https://github.com/epezent/implot)

## Tools Used
- [CMake](https://cmake.org/)
- [Visual Studio](https://visualstudio.microsoft.com/)

## Building the project
Delete 'out' folder and 'GLFW', 'imgui-docking', 'implot' from 'vendor' if they exist. Ctrl+S to run the CMakelists.txt file located in the project folder. This will clone the repositories and try to build. In case of GLFW errors, simply re run the CMakelists.txt file.

## Maze Building Algorithms
### Recursive Backtrack and application delay

![RB](https://raw.githubusercontent.com/HaKai-2062/Mazes/main/res/github/gifs/2_RB.gif)

### Kruskal

![Kruskal](https://raw.githubusercontent.com/HaKai-2062/Mazes/main/res/github/gifs/3_Kruskal.gif)

### Prims

![Prims](https://raw.githubusercontent.com/HaKai-2062/Mazes/main/res/github/gifs/4_Prims.gif)

### Wilson

![Wilson](https://raw.githubusercontent.com/HaKai-2062/Mazes/main/res/github/gifs/5_Wilson.gif)


## Maze Solving Algorithms
### Depth first search

![DFS](https://raw.githubusercontent.com/HaKai-2062/Mazes/main/res/github/gifs/6_DFS.gif)

### Breadth first search

![BFS](https://raw.githubusercontent.com/HaKai-2062/Mazes/main/res/github/gifs/7_BFS.gif)

### Dijkstra

![Dijkstra](https://raw.githubusercontent.com/HaKai-2062/Mazes/main/res/github/gifs/8_Dijkstra.gif)

### A(*) star

![Astar](https://raw.githubusercontent.com/HaKai-2062/Mazes/main/res/github/gifs/9_Astar.gif)

## Miscellaneous
#### Dockable window. cell width and wall width can be tweaked for maze size and look.

![Intro](https://raw.githubusercontent.com/HaKai-2062/Mazes/main/res/github/gifs/1_Intro.gif)

### Path color cycle speed and cell weights being tweaked and heatmap for visualization

![Colors_Weights](https://raw.githubusercontent.com/HaKai-2062/Mazes/main/res/github/gifs/10_Colors_Weights.gif)
