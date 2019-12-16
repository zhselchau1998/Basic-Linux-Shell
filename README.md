### Basic Linux Shell
---
## About This Project
This project was an assignment from my OS class at SDSU. For this assignment we were tasked with creating a basic linux shell in the C programming language. We were provided the makefile, getword.h, and p2.h. We had to create the README.md, getword.c, and p2.c

## Design Decisions
- I have decided to treat the input "echo hi>&" as an error since I interpret the >& as a metacharacter and it doesnt give a filename to put stdout and stderr.

- I have also decided to have !! store the most recent command that is not empty. So an input of "echo hi\n" followed by "\n" > followed by "!!" will output:

>hi
>
>hi
>p2 terminated
>
>I did this because I do not interpret and empty line as a command and I think that this behaviour is more user friendly.

- In the input case of: echo hi <\&. I interpret the final \& as a metacharacter, which means this is a syntax error since the '>' wasn't given a filename.

- In the case of two pipes in one line, I interpret that as a syntax error within parse. Just as < followed by nothing would be interpretted as a syntax error

- As a result of the way I implemented the history command, history commands past 10 commands overwrite past commands and is liable to break down after it reaches that point.
	*NOTE* p2 will not crash and will functions normally except for the ![1-9] commands. And these commands may still work under certain circumstances, just not all circumstances.

- Another result of my implemetation of history, only the number directly after the ! in a history command is interpreted. So !1 == !1000 by my program's logic

- In the case of both > and >> appearing on the same line, I interpreted that as a syntax error exactly as if > and > were on the same line.

##Research Resources
* In Class Discussion
* Course Reader:
	- ~cs570/exec1.c page 3
	- ~cs570/fork2.c page 5
	- ~cs570/pipe.c page 6
	- ~cs570/dup2.c page 8
	- ~cs570/sighandler.c page 10
	- ~cs570/environ.c page 16
	- ~Assignment 2 pseudo code page 94
* Man pages
	- man 2 dup2
	- man 3 execvp
	- man 3 getenv
	- man 2 chdir
	- man 2 fork
	- man 2 wait
	- man 2 access
	- man 2 signal
	- man 2 setpgid
	- man 2 getpgrp
	- man 2 killpgid
* Outside sources
	- [Pointers Refresher](https://www.geeksforgeeks.org/pointers-in-c-and-c-set-1-introduction-arithmetic-and-array/)
	- [strcpy Refresher](https://www.geeksforgeeks.org/strcpy-in-c-cpp/)
	- [setpgid Learning](https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.4.0/com.ibm.zos.v2r4.bpxbd00/rtsetp.htm)
	
	
