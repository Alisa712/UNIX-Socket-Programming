# UNIX-Socket-Programming
## Implementation:
Implemented a model of computational offloading where a single client offloads Tylor Series computation to a AWS server, which in turn distributes the load over three backend servers: serverA, serverB and serverC; Client and AWS communicates over TCP, and AWS and three servers communicates over UDP. The server supports two type of computations: DIV and LOG

## Running Project:
Specified in Makefile. Because I used threads and it would create concurrent processes, you don't need to open 5 terminals to run the aws server and three backend servers. You can do this:

Open one terminal:

```
$ make all
```

```
$ make servers
```

Open a second terminal:


```
$ make aws
```

Open a third terminal:

```
$ ./client <function> <val>
```

to do the test.

OR 

```
$ make all
```

```
$ make test
```

Then two terminals (one for AWS and one for backend servers) will be opened, and you can type:

```
$ ./client <function> <val>
```

 to test.

OR

```
$ make all
```


```
make testinput COMMANDS=<function> VALUES=<val> 
```


## Code files description
### client.cpp
Establish TCP connection to AWS, give the function type and value that needs to be computed.

### servers.cpp
Three servers to perform calculations:

- Backend-Server A: calculating square
- Backend-Server B: calculating cube
- Backend-Server C: calculating 5th power

### aws.cpp
Communicate with client (TCP) and three servers (UDP), take the input from client and presents the computed final result for the client.


## Format of   all   the   messages   exchanged
All messages exchanged are in char array.

## Any idiosyncrasy of this project
All follow the project description, and runs well in my test cases.
However, because AWS and the three servers communicates over UDP, if any backend server crashed, AWS would not be able to detect it. 

## Reused code
Beej’s socket programming tutorial
http://beej.us/guide/bgnet/output/print/bgnet_USLetter.pdf

Referred some code in order to get fimilar with the process and argument type.

