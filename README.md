#UAB CS432 - Spring 2017

##Compiling

###Assignment 3 and above

```
icc -std=c++1y -O3 -pthread main.cpp 
```

or

```
g++ -std=c++1y -O3 -pthread main.cpp 
```

###Assignment 2

```
icc -std=c++1y -O3 main.cpp 
```

or

```
g++ -std=c++1y -O3 main.cpp 
```

###Assignment 1

You may compile with the same as in Assignment 2, however no arguments are accepted at compile time and are instead accepted as user input.


##Running

###Assignment 3 and above

```
./a.out [Board Size] [# of Rounds] [# of Threads]
```

###Assignment 2

```
./a.out [Board Size] [# of Rounds]
```
