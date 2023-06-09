# CSEC 476 Final Project

## Group members

* Adam Waldie
* Christopher Brooks

## Description

This project consists of two components. First, a client and server which can send and receive instructions and data. Second, an IDB file containing a fully annotated disassembly of the client. The purpose of this project is to gain a better understanding of reversing Windows programs.

## Commands

Commands are input at the server and sent to the client to solicit a response or action. Arguments should be delimited with a `?` character.

`shutdown` - shuts down the connection  
`inform` - asks the client for information about its host device  
`proc` - asks for a list of (32-bit) processes running on the client device  
`upload?<filepath>` - asks client to send the file if it exists  
`download?<filename>?<url>` - tells the client to download a file from `URL` and name it `filename` 

## Usage

Start the server program, it will bind to `0.0.0.0:1337` by default. Start the client program with `client.exe <address>` to connect to the server at `<address>:1337`. Once the client connects to the server, you can issue commands from the server.

Note: the application is very buggy and sometimes the sockets will just crash or something?

## Compiling

Binaries compiled on 64-bit Windows 11 with `g++.exe (MinGW.org GCC-6.3.0-1) 6.3.0`.

```
g++ src\client\client.cpp -o client -lws2_32 -lwininet -lpsapi -liphlpapi
g++ src\server\server.cpp -o server -lws2_32
```
