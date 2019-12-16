/*
Name: Zach Selchau

Professor: John Carroll

Class: CS 570, Operating Systems

Assignment: Assignment 4

File: p2.c

Due Date: April 29, 2019

Description:	This file is the source code for 
		my slightly less basic linux shell. 
		This file will produce an executable 
		file, p2, which will execute the 
		commands: cd, !!, !# commands, and 
		anything executable by execvp(). p2 
		will also have the ability to run 
		processes in the background as
		well as redirect input and output.
		p2 also has the ability to refrense 
		last word in the previous line with
		the !$ keyword and singular piping
		is now supported. Commenting is also
		now supported when passing in a
		command line argument to p2 with the
		'#' character surrounded by whitespace.
		
Pseudo Code:	Change process group
		Check for args to p2
			if there are then change input file
		loop
			set/reset global variables
			print prompt
			call parse
			check for errors and termination cases
				continue; or break; respectively
			handle built in commands (cd, and !!)
				continue;
			fflush and fork
			if child then check if piping
				if piping then set up I/O for piping and execvp
					check for I/O redirection and set up for piping
					exit(0);
				if not then check I/O normally and execvp
					exit(0);
			if parent then check if need to wait
				if yes wait for started child;
				else print child pid and continue;
		kill the process group
		print "p2 terminated"
		exit

Exit values:	0 - Success
		1 - Command line input file error
		2 - Could not fork a child
		NOTE: Subsequent values only occur in child processes after forked
		3 - Could not find the input file specified
		4 - The output file already exists. Will not overwrite a file
		5 - The output file does not exist. Cannot append to a file that DNE
		6 - Could not execute the command specified
*/

#include "p2.h"

//Global Variables
char*	newArgv[MAXITEM];	//The argument list for the program being executed
int	newArgc = 0;		//The size of the new argument list
					//NOTE: newArgv stores references to s in main

char* 	infilePtr = NULL;	//String of the input file
char*	outfilePtr = NULL;	//String of the output file
char* 	errfilePtr = NULL;	//String of the error file
int 	appendFiles = 0;	//1 when appending to files (>>, >>&), 0 when not apending to files

int	pipefilePtr = -1;	//Index for start of newArgv of pipe
int	pipeIntArr[2];

int 	waitCheck = 1;		//1 is wait, 0 is do not wait

int	bangCheck = 0;		//1 is !!, 0 is no !!

int 	argcCheck = 0;		//1 is argc > 1, else 0

char*	cmdHistory[MAXITEM * 10];	//List of last 10 commands
						//NOTE: for each command "offset + (lineCounter - 1) * MAXITEM"
char	cmdHistoryStorage[STORAGE * MAXITEM * 10];	//Storage of last 10 commands
int	cmdHistoryLength[10];	//Tracks the bufferSize of last 10 commands

char*	lastWord = NULL;	//Tracks the last word of the last command for !$

int 	lineCounter = 1;	//Keeps track of input line 

int 	BKSLCheck = 0;		//Tracksif backslash in word 1 is yes, 0 is no
int	BKSLMemory[MAXITEM];	//Remebers which words had backslashes in them
extern int BKSLCheck;		//Make BKSLCheck global

int parse( char* w ){
	char* 	wordBuffer[MAXITEM];	//Storage for every word on the line
	int 	bufferSize = 0;		//Size of storage
	char* 	oldW = w;		//Saving to reset later
	
	int 	c;	//Return value of getword
	int 	i;	//Defining counter in for loop
	int	offset = 0;	//Offset variable used as history offset
	
	//Deliminating line
	for(;;){ 	
		c = getword( w );
		
		if( c == -1 && w == oldW) return -1;	//If reached a done/EOF then exit
		else if ( c == 0 ) break;	//If reached the newline then start parsing
		else if ( c == -1 && strcmp(w, "done") == 0 ) c = 4;
		else if ( c == -1 ) break;	//EOF reached with stuff on the line

		BKSLMemory[bufferSize] = BKSLCheck;	//Store backslash presence
		BKSLCheck = 0;				//Reset backslash checker

		strcpy(&cmdHistoryStorage[offset + ((lineCounter - 1) % 10) * STORAGE * MAXITEM], w);	//Store the word in permenant memory
		cmdHistory[bufferSize + ((lineCounter - 1) % 10) * MAXITEM] = 
			&cmdHistoryStorage[offset + ((lineCounter - 1) % 10) * STORAGE * MAXITEM];	//Save a pointer to start of this word
		offset += c + 1;	//Follow w
		
		wordBuffer[bufferSize++] = w;	//Store the word
		w += c + 1;			//Increment the word pointer
	}
	
	if(w == oldW) return 1;		//Check if empty line
	
	w = oldW;			//Resetting s in main
	wordBuffer[bufferSize] = NULL;	//Null terminating the storage
	
	cmdHistory[bufferSize + ((lineCounter - 1) % 10) * MAXITEM] = NULL;	//NULL terminate
	cmdHistoryLength[(lineCounter-1) % 10] = bufferSize;			//Save the bufferSize
	
	//Previous command checks
	if( *(wordBuffer[0]) == '!' && *(wordBuffer[0] + 1) > 47 && *(wordBuffer[0] + 1) < 58){	//If found a !# *NOTE*: only checks the character directly after '!'
		offset = (*(wordBuffer[0] + 1) - 49) % 10;	//Save the number directly after '!'
		
		if( cmdHistory[offset * MAXITEM] == NULL )	//Error Case, invalid !#
			return 0;
		
		for( i = 0; i<=cmdHistoryLength[offset]; i++){	//Retrive the specified line and save it into wordBuffer
			wordBuffer[i] = cmdHistory[i + offset * MAXITEM];
			cmdHistory[i + ((lineCounter - 1) % 10) * MAXITEM] = wordBuffer[i];
			}
		bufferSize = cmdHistoryLength[offset];		//Retrieve the size of the buffer
		cmdHistoryLength[((lineCounter - 1) % 10)] = bufferSize;	//Save the size in history
	}
	
	if( strcmp(wordBuffer[0], "!!") == 0 ){		//!! Case, treat as $(lineCounter-1)
		offset = (lineCounter - 2) % 10;		//-2 because the array is 0 based
		for( i = 0; i<=cmdHistoryLength[offset]; i++){	//Retrive the specified line and save it into wordBuffer
			wordBuffer[i] = cmdHistory[i + offset * MAXITEM];
			cmdHistory[i + ((lineCounter - 1) % 10) * MAXITEM] = wordBuffer[i];
			}
		bufferSize = cmdHistoryLength[offset];		//Retrieve the size of the buffer
		cmdHistoryLength[((lineCounter - 1) % 10)] = bufferSize;	//Save the size in history
	}
	
	//Parsing line
	for(i=0; i<bufferSize; i++){
		
		if( strcmp(wordBuffer[i], "<") == 0 ){ 		//< Case
			//Error Cases
			if( i+1 == bufferSize ) return 0;		//Nothing following <
			if( strcmp(wordBuffer[i+1], "<") == 0 ||	//Nonsense following >&
			    strcmp(wordBuffer[i+1], ">") == 0 ||
			    strcmp(wordBuffer[i+1], ">>") == 0 ||
			    strcmp(wordBuffer[i+1], "#") == 0 ||
			    strcmp(wordBuffer[i+1], "&") == 0 ||
			    strcmp(wordBuffer[i+1], ">&") == 0 ||
			    strcmp(wordBuffer[i+1], ">>&") == 0 ) return 0;
			if( infilePtr != NULL ) return 0;		//Already encountered <
			
			infilePtr = wordBuffer[i+1];	//Set the input file to next word
			i++;				//Increment to account for next word
			
			if( strcmp(infilePtr, "!$") == 0)	//!$ Case
				infilePtr = lastWord;
			
		}else if( strcmp(wordBuffer[i], ">") == 0 || strcmp(wordBuffer[i], ">>") == 0 ){ 	//> and >> Case
			//Error Cases
			if( i+1 == bufferSize ) return 0;		//Nothing following >
			if( strcmp(wordBuffer[i+1], "<") == 0 ||	//Nonsense following >&
			    strcmp(wordBuffer[i+1], ">") == 0 ||
			    strcmp(wordBuffer[i+1], ">>") == 0 ||
			    strcmp(wordBuffer[i+1], "#") == 0 ||
			    strcmp(wordBuffer[i+1], "&") == 0 ||
			    strcmp(wordBuffer[i+1], ">&") == 0 ||
			    strcmp(wordBuffer[i+1], ">>&") == 0 ) return 0;
			if( outfilePtr != NULL ) return 0;		//Already encountered > || >&
			
			if( strcmp(wordBuffer[i], ">>") == 0 ) appendFiles = 1;
			
			outfilePtr = wordBuffer[i+1];	//Set the output file to next word
			i++;				//Increment to account for next word
			
			if( strcmp(outfilePtr, "!$") == 0){	//!$ Case
				outfilePtr = lastWord;
				cmdHistory[i + ((lineCounter - 1) % 10) * MAXITEM] = lastWord;	//Replace !$ with lastWord in history
			}
			
		}else if( strcmp(wordBuffer[i], ">&") == 0 || strcmp(wordBuffer[i], ">>&") == 0 ){	//>& and >>& Case
			//Error Cases
			if( i+1 == bufferSize ) return 0;		//Nothing following >&
			if( strcmp(wordBuffer[i+1], "<") == 0 ||	//Nonsense following >&
			    strcmp(wordBuffer[i+1], ">") == 0 ||
			    strcmp(wordBuffer[i+1], ">>") == 0 ||
			    strcmp(wordBuffer[i+1], "#") == 0 ||
			    strcmp(wordBuffer[i+1], "&") == 0 ||
			    strcmp(wordBuffer[i+1], ">&") == 0 ||
			    strcmp(wordBuffer[i+1], ">>&") == 0)return 0;
			if( outfilePtr != NULL ) return 0;		//Already encountered > || >&
			
			if( strcmp(wordBuffer[i], ">>&") == 0 ) appendFiles = 1;
			
			outfilePtr = wordBuffer[i+1];	//Set the output file to next word
			errfilePtr = wordBuffer[i+1];	//Set the error file to next word
			i++;				//Increment to account for next word
			
			if( strcmp(outfilePtr, "!$") == 0){ 	//!$ Case
				outfilePtr = lastWord;
				errfilePtr = lastWord;
				cmdHistory[i + ((lineCounter - 1) % 10) * MAXITEM] = lastWord;	//Replace !$ with lastWord in history
			}
			
		}else if( strcmp(wordBuffer[i], "!$") == 0 ){	// !$ Case
			newArgv[newArgc++] = lastWord;
			cmdHistory[i + ((lineCounter - 1) % 10) * MAXITEM] = lastWord;	//Replace !$ with lastWord in history
		
		}else if( strcmp(wordBuffer[i], "|") == 0 && BKSLMemory[i] == 0 ){	// | Case
			if( pipefilePtr != -1) return 0;
			pipefilePtr = newArgc+1;	//Set the pipe file to next word
			newArgv[newArgc++] = NULL;
		
		}else if( i+1 == bufferSize && strcmp(wordBuffer[i], "&") == 0 )	//& Case
			waitCheck = 0;
		
		else if( strcmp(wordBuffer[i], "#") == 0 && argcCheck )	//# Case
			break;
		
		else	//General Case
			newArgv[newArgc++] = wordBuffer[i];	//Save the word in new argument list	
	}
	
	lastWord = cmdHistory[bufferSize - 1 + ((lineCounter - 1) % 10) * MAXITEM];
	newArgv[newArgc] = NULL;				//Null terminate the argument list
	return 1;
}

void myHandler(int signum){
	//printf("recieved SIGTERM (%d)", signum);	//Debug statement
}

int main(int argc, char* argv[]){
	//Declarations
	int cPid;	//PID of child
	int gcPid;	//PID of grandchild
	
	char*	lastExec[MAXITEM];		//Stores the last argument list
	int	lastArgc = 0;			//Size of last argument list
	char	lastInfile[STORAGE];		//String of the last input file
	char	lastOutfile[STORAGE];		//String of the last output file
	char 	lastErrfile[STORAGE];		//String of the last error file
	char	lastStorage[STORAGE * MAXITEM];	//Storage area for past arguments
	int 	i;				//Incrementing value in for loop
	
	int inputFile;	//Used as argument and reused as input redirection
	int outputFile; //Used as output redirection and reused as error redirection
	int iFFlags = O_RDONLY;			//Read only for input files
	int oFFlags = O_WRONLY | O_CREAT;	//Write only and creating for output files
	int aoFFlags = O_WRONLY | O_APPEND;	//Write only and appending for output files
	
	int c;	//Return value of parse
	char s[STORAGE * MAXITEM];	//Storage for the imput line
	
	(void) setpgid(0, 0);			//Separate p2 into it's own group
	(void) signal(SIGTERM, myHandler);	//Catch SIGTERM so it doesn't go to tcsh
	
	//Check for any arguments
	if( argc > 1 ){	
		//Get the input file
		if( (inputFile = open(argv[1], iFFlags, S_IRUSR | S_IWUSR)) < 0 ){
			//Error encountered
			perror("Could not open file");
			exit(1);
		}
		
		(void) dup2(inputFile, STDIN_FILENO);	//Redirect input of p2
		(void) close(inputFile);		//Clean up after yourself
		
		argcCheck = 1;	//Tell parse to check comments
	}

	//Looping the terminal
	for(;;){
		//Resetting global vars
		newArgc = 0;
		infilePtr = NULL;
		outfilePtr = NULL;
		errfilePtr = NULL;
		pipefilePtr = -1;
		waitCheck = 1;
		bangCheck = 0;
		appendFiles = 0;
		
		
		if(argc <= 1)
			(void) printf("%%%d%% ", lineCounter);	//Prompt
		
		c = parse(s);	//Parse the function
		
		if( c == 0 ){ 	//Handle syntax error and cont
			perror("Syntax Error");
			continue;
		}
		
		if( newArgc == 0 && c == -1 ){	//Exit condition
			//printf("\n");
			break;
		}
		
		if( bangCheck == 1 ){	//!! Was given
			//Load up last command
			for( i = 0; i<lastArgc; i++)
				newArgv[i] = lastExec[i];
			newArgc = lastArgc;
			
			//I/O flags
			if( strcmp(lastInfile, "") != 0 ) infilePtr = lastInfile;
			if( strcmp(lastOutfile, "") != 0 ) outfilePtr = lastOutfile;
			if( strcmp(lastErrfile, "") != 0 ) errfilePtr = lastErrfile;
			
		}else{	//!! wasn't given
			//Copy over s into some memory
			for( i = 0; i < STORAGE * MAXITEM; i++)
				lastStorage[i] = s[i];
			
			//Store the a copy of this newArgv
			for( i = 0; i < newArgc; i++){
				lastExec[i] = lastStorage + (newArgv[i] - newArgv[0]);
			}
			
			lastArgc = newArgc;
			lastExec[lastArgc] = NULL;
			
			//I/O flags
			if( infilePtr!=NULL ) strcpy(lastInfile, infilePtr);
			else strcpy(lastInfile, "");
			if( outfilePtr!=NULL ) strcpy(lastOutfile, outfilePtr);
			else strcpy(lastOutfile, "");
			if( errfilePtr!=NULL ) strcpy(lastErrfile, errfilePtr);
			else strcpy(lastErrfile, "");
		}
		
		if( newArgc == 0 ) continue;	//If newArgv is empty then continue
		
		if( strcmp(newArgv[0], "cd") == 0 ){	//cd command
		
			if( newArgc == 1 ){	//No argument(s) given
				if( chdir(getenv("HOME")) == -1 )	//Try to change directory
					perror("Could not change directory");	//chdir failed
				
			}else if( newArgc > 2 ){	//Ambiguous if more than one argument
				perror("Too many arguments passed to cd, will not change directory");
			
			}else{			//Arguemnt(s) given to cd
				if( chdir(newArgv[1]) == -1 ){	//Try to change directory
					perror("Could not change directory");	//chdir failed
					continue;
				}
				lineCounter++;
			}
			continue;
		}
		
		
		(void) fflush(stdout);	//Force out all output and errors before forking
		(void) fflush(stderr);
		
		cPid = fork();		//Fork a child
		
		if( cPid == -1 ){	//Forking error
			perror("Could not fork");
			exit(2);
			
		}else if( cPid == 0 ){ 	//Child
		
			if(pipefilePtr != -1){	//Pipeing
				pipe(pipeIntArr);
				
				(void) fflush(stdout);	//Force out all output and errors before forking
				(void) fflush(stderr);
				
				gcPid = fork();		//Fork a grandchild
				
				if(gcPid == -1){	//Forking error
					perror("Could not fork");
					exit(2);
				}else if(gcPid == 0){	//Grandchild
				
					if(infilePtr != NULL){	//Redirecting input
			
						if(access (infilePtr, F_OK) == -1){
							perror("Grandchild cannot find the input file");
							exit(3);
						}
				
						inputFile = open(infilePtr, iFFlags, S_IRUSR | S_IWUSR);
						(void) dup2(inputFile, STDIN_FILENO);	//Redirect input
						(void) close(inputFile);		//Cleanup
					}
					
					(void) dup2(pipeIntArr[1], STDOUT_FILENO);	//Redirect output
					(void) close(pipeIntArr[0]);	//Close pipe references
					(void) close(pipeIntArr[1]);
					
					//Execute the command
					if( execvp(newArgv[0], newArgv) == -1 ){
						perror("Grandchild could not execute that program\n");
						exit (6);
					}
					
				}else{	//Original Child
				
					if(outfilePtr != NULL && appendFiles == 0){	//Redirecting output with >
			
						if(access (outfilePtr, F_OK) != -1){
							perror("Child cannot overwrite existing files");
							exit(4);
						}
						outputFile = open(outfilePtr, oFFlags, S_IRUSR | S_IWUSR);
						(void) dup2(outputFile, STDOUT_FILENO);	//Redirect output
				
						if(errfilePtr != NULL)	//Redirecting stderr
							(void) dup2(outputFile, STDERR_FILENO);//Redirect stderr
					
						close(outputFile);	//Cleanup
					}else if(outfilePtr != NULL && appendFiles == 1){	//Redirecting output with >>
			
						if(access (outfilePtr, F_OK) == -1){
							perror("Child cannot access output file");
							exit(4);
						}
						outputFile = open(outfilePtr, aoFFlags, S_IRUSR | S_IWUSR);
						(void) dup2(outputFile, STDOUT_FILENO);	//Redirect output
				
						if(errfilePtr != NULL)	//Redirecting stderr
							(void) dup2(outputFile, STDERR_FILENO);//Redirect stderr
					
						close(outputFile);	//Cleanup
					}
					
					(void) dup2(pipeIntArr[0], STDIN_FILENO);	//Redirect input
					
					(void) close(pipeIntArr[0]);	//Close pipe references
					(void) close(pipeIntArr[1]);
					if( execvp(newArgv[pipefilePtr], (newArgv + pipefilePtr)) == -1 ){
						perror("Child could not execute that program\n");
						exit (6);
					}
					
				}
				
			}
			
			if(infilePtr != NULL){	//Redirecting input
			
				if(access (infilePtr, F_OK) == -1){
					perror("Cannot find the input file");
					exit(3);
				}
				
				inputFile = open(infilePtr, iFFlags, S_IRUSR | S_IWUSR);
				(void) dup2(inputFile, STDIN_FILENO);	//Redirect input
				(void) close(inputFile);		//Cleanup
			}
			
			if(outfilePtr != NULL && appendFiles == 0){	//Redirecting output with >
			
				if(access (outfilePtr, F_OK) != -1){
					perror("Cannot overwrite existing files");
					exit(4);
				}
				outputFile = open(outfilePtr, oFFlags, S_IRUSR | S_IWUSR);
				(void) dup2(outputFile, STDOUT_FILENO);	//Redirect output
				
				if(errfilePtr != NULL)	//Redirecting stderr
					(void) dup2(outputFile, STDERR_FILENO);//Redirect stderr
					
				close(outputFile);	//Cleanup
			}else if(outfilePtr != NULL && appendFiles == 1){	//Redirecting output with >>
			
				if(access (outfilePtr, F_OK) == -1){
					perror("Cannot access the output file");
					exit(4);
				}
				outputFile = open(outfilePtr, aoFFlags, S_IRUSR | S_IWUSR);
				(void) dup2(outputFile, STDOUT_FILENO);	//Redirect output
				
				if(errfilePtr != NULL)	//Redirecting stderr
						(void) dup2(outputFile, STDERR_FILENO);//Redirect stderr
					
				close(outputFile);	//Cleanup
			}
			
			//Execute the command
			if( execvp(newArgv[0], newArgv) == -1 ){
				perror("Could not execute that program\n");
				exit (6);
			}
			
			if(waitCheck == 0 && infilePtr == NULL){
				inputFile = open("/dev/null", iFFlags, S_IRUSR | S_IWUSR);
				(void) dup2(inputFile, STDIN_FILENO);
				(void) close(inputFile);
				waitCheck = 1;
			}
			
			exit(0);
			
		}else{ 			//Parent
			if(waitCheck == 1){ 
				for(;;){	//Find the child this line started and cleave it
					pid_t thisPid;
					
					thisPid = wait(NULL);
					
					if(thisPid == cPid) break;
				}
				
			}else (void) printf("%s [%d]\n", newArgv[0], cPid);
			
			lineCounter++;
			
		}	
	}
	
	(void) killpg( getpgrp(), SIGTERM );	//Coppied verbatim from lecture notes
	if(argc <= 1)(void) printf("p2 terminated.\n");
	exit(0);
	return 0;
}
