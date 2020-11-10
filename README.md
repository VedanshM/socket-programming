# Socket Programming

## Steps to build and run server

+ To build: `gcc server.c -o server`
+ Execute `server` in directory containing files to be shared.  
  eg if desired directory is same as residing directory of `server`, run :  
  `./server`

## Steps to build and run client

+ To build: `gcc client.c -o client`
+ Execute `client` in directory in which files are to be placed.  
  eg if that directory if residing directory of `client`, run  
  `./client [file1] [file2] [file3] ...`  
+ In above command the arguments of the executable `client` are the files to be downloaded.

## Overview

+ The client requests the server program a list of files from server progran's directory that will be downloaded to client's directory.
+ The communication involves sending an integer which is the total number of files and then file name.  
+ If the file exists then after acknowledging it, the transfer is initiated else an error message is transferred.
+ The transfer starts by first sending the file size followed by the stream of bytes which is basically the file.
+ Once a file is downloaded next file is transferred by the same procedure if available.
