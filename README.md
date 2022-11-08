 # What is this C project ?

In computer science, when you have "inputs" sharing the same grammar, it is useful to make them understandable by your programs. We can turn the input into an Abstract Syntax Tree, then go through the tree performing any needed process, this project is a basic example about it. 

The hardest part of the work, which are lexing and parsing are done using the well known tools Flex and Bison. We can write efficient, clear and readable programs using Flex and Bison, it's not easy to do the same job without them, especially in low level languages (C have no automatic memory management).

This project take Turtle programs as input, they are files.

**What is this software currently used for?** This project interprets the Turtle programs provided in input and print as output the instructions that a basic drawer automaton needs. For this exercise, the drawer automaton is represented by another program called the turtle-viewer.

In case of error, when the inputs are wrong, everything should be OK, the error is detected, no output is displayed on stdout (the standard output), only a basic explanation is displayed on stderr (the error output).

**What could be a real use for this type of software?** Reading configuration files to start a process.

# Ubuntu usage

If you don't know how to get an executable, try to follow this procedure :

- Open a **Terminal** in the downloaded  directory, for example using a right-click or the [cd](https://en.wikipedia.org/wiki/Cd_(command)) command
- Ubuntu assures you that a C/C++ compiler is available when you run the command `sudo apt install build-essential`

Execute the following installation commands :
```sh
sudo apt-get update 
sudo apt-get upgrade
sudo apt install cmake flex bison
```
If you plan to use the Turtle viewer also execute the following command :
```c
sudo apt install libsndio7.0
```
Compile your executable by executing the following instructions (you are still located in the downloaded directory) :
```sh
cmake .
make all
```
Your turltle executable is ready, it's locaded at **./turtle**. You can basically test it using `echo -e "print random(2, 2.5)" | ./turtle`, it should print a random number between 2 and 2.5 inclusive. Another syntax is to use `./turtle < program.turtle` if you filled the `program.turtle` file with `print random(2, 2.5)`.

The command`./turtle < ./default-hello.turtle` will interpret the `default-hello.turtle` file using the `turtle` executable showing you the result on the standard output (often named stdout).

When you perform some changes in the program source, you just have to execute `make all` to keep updated your Turtle executable.

# Windows usage
It's possible to download [Flex and Bison for Windows](https://github.com/lexxmark/winflexbison/releases/tag/v2.5.25), then to request a [JetBrains CLion](https://www.jetbrains.com/clion) demo, this IDE like some others will help you compiling your executable like as Ubuntu. Only the viewer isn't avaliable for Windows.

The versions i used during Windows development are :

- flex.exe 2.6.4
- bison (GNU Bison) 3.8.2

See the top of `CMakeLists.txt` file to manage the Flex.exe and Bison.exe paths.

# How to use the viewer ?
The turtle viewer is avaliable for  Ubuntu users, it requires **libsndio7.0**.
```sh
sudo apt install libsndio7.0  
chmod 700 ./turtle
chmod 700 ./turtle-viewer  
./turtle < ./default-hello.turtle | ./turtle-viewer
```
Where :
-  `./turtle` is your turtle interpreter (which is compiled using the given C source code)
- `./program.turtle` is your Turtle program that must be submited to your interpreter (samples draws are provided)
- `./turtle-viewer` is the provided viewer

If the Ubuntu permissions are OK ([chmod](https://fr.wikipedia.org/wiki/Chmod)), you can check my own programs using the following command :
```sh
./turtle < ./my-fougeres.turtle | ./turtle-viewer
```

# What to do ?

You can take this project as example, update its source code, and/or create your own works (turtle programs).

Take a look at my [Barnsley fern](https://github.com/michel-leonard/Turtle/blob/main/my-fougeres.turtle) drawing below, it only takes 100 lines of Turtle code to display it using the turtle-viewer.

![my Barnsley fern draw](https://raw.githubusercontent.com/michel-leonard/Turtle/main/default-image.png)

# Thank you

- my professors at University of Franche-Comté ❤️
- GitHub users reporting issues
