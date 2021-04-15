# Basic C2
## About
This code is a work in progress, and is still messy and unoptimized. This repo currently functions as a backup of a working version of this C2.
I'm creating this project because i want to improve my knowledge on malware and it's inner workings.

Currently the project has a client written in c/c++, to keep the executable as small as possible, and a server written in Python which supports multiple bots.

## Compiling
Currently only works in a Windows environment
```
g++ -std=gnu++11 client.cpp -o client.exe -s -lws2_32 -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libstdc++ -static-libgcc
```

## TODO
* Process injection
* AV evasion
* Write a dropper
* Add intelligence based on which host it's run
* Server - client communication encryption (possibly over different protocols)
* Network propagation
* Polymorphic code?
* Enhance server client heartbeat and improve reliability
* Broadcast commands to all active connections
* Expand functionality
  * Keylogging
  * Webcam and microphone control
  * Read clipboard
  
## Current functionality
### Server
* Connect to bots individually
* Connect and disconnect from bots
```
Server commands:
  list - List current active connections
  select [id] - Select session of active connections and drop into a shell
  delcon [id] - Delete an active session (this will not close the client on remote host)
```
### Client
* Automatic callback to server with host information
* Automatic persistence (RunKey)
* File upload/download
#### Available commands
```
Available remote commands:
  cd - Change current working directory
  ls/dir - List files and directories from current working directory
  whoami - Get username of the remote host
  pwd - Get current directory
  hostname - Get hostname of the remote host
  cleanpersist - Clean up persistence backdoors
  exec [cmd command] - Execute cmd commands directly on remote host
  bg - Background the current remote shell and return to server
  quit - Close the current connection (Also closes process on remote host)
File upload/download:
  get - Download file from current working directory to the server
    Example: get passwords.txt
put - Upload file from server to the current working directory @ client
    Example: put secretbackdoor.exe
```
