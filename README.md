# CSEC 476 Final Project

## Group members

* Adam Waldie
* Christopher Brooks

## Description

This project consists of two components. First, a client and server which can send and receive instructions and data. Second, an IDB file containing a fully annotated disassembly of the client. The purpose of this project is to gain a better understanding of reversing Windows programs.

# Compiling

Until I have a better solution like a makefile.

```
cd client
g++ client.cpp -o client -lws2_32
cd ..\server
g++ server.cpp -o server -lws2_32
```