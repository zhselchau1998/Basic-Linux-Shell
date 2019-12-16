/*
Name: Zach Selchau

Professor: John Carroll

Class: CS 570, Operating Systems

Assignment: Assignment 4

File: p2.h

Description:	This file contains the documentation
		for the functions in p2.c
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "getword.h"

#define MAXITEM 100 //Max num of words per line

/*
int parse();

arguments: 	char pointer (input from stdin)
returns:	1 if success, 0 if failure, -1 if EOF
description:	attempts to parse the char*. If it 
		is ambiguous, failure. If EOF is encountered
		return -1. else success. Stores the
		line in the global buffer.
*/
int parse(char* w);

/*
void myHandler();

arguments: 	int (signal)
returns:	void
description:	provides a handler for SIGTERM
*/
void myHandler(int signum);
