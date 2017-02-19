#UAB CS432 - Spring 2017

##Compiling

###Assignment 3 and above

```
icc -std=c++1y -O3 main.cpp [Board Size] [# of Rounds] [# of Threads]
```

or

```
g++ -std=c++1y -O3 main.cpp [Board Size] [# of Rounds] [# of Threads]
```

###Assignment 2

You may compile with the same as in Assignment 3+, but as this version is a serial program no thread arguments are processed

###Assignment 1

You may compile with the same as in Assignment 2, however no arguments are accepted at compile time and are instead accepted as user input.