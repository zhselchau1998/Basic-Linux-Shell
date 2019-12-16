/****************|Header|****************
Name:		Zach Selchau
Professor: 	John Carroll
Class:		CS 570, Operating Systems
Assignment:	Project 4
Description:	This end goal og this
		assignment is to create a
		lexical analyzer to the
		specifications given in
		the Programming Assignment
		1 specifications. A more
		complex version of what we
		did in Program 0.
****************|Header|****************/

#include "getword.h"

#define NL	10	// '\n'
#define SPACE	32	// ' '
//#define PND	35	// '#'
#define ANPS	38	// '&'
#define LT	60	// '<'
#define GT	62	// '>'
#define BKSL	92	// '\'
#define PIPE	124	// '|'


//getword function
/*	Argument(s):	Character pointer
	Return value:	-Length of the first 'word' encountered in the char*
			as an int
			-if a \n is encountered returns the int 0
			-if an EOF is encountered then returns the int -1
			
	Mock code:	-while (leading spaces) getchar()
				*gets rid of leading spaces
			-if (EOF) then (return -1)
			-else if ('\') then (increment and contunue)
			-else if (NL || SPACE || metacharacter) then
					(replace current char w/ NULL
					&& ungetc()
					&& return length)
	
	Sources used:	home/cs/carroll/cssc0088/Zero/getword.c
				~masc0000/CbyDiscovery/ch2/inout2.c
				man 3 ungetc
*/

int BKSLCheck;

int getword( char* w){

	int ungetcCheck = 0;
	int length = 0;
	int s = getchar();
	
	//Leading spaces
	while( s == SPACE ) s = getchar();
	
	while( s != EOF  && s != SPACE && s != NL && /*s != PND &&*/
	       s != ANPS && s != LT    && s != GT && s != PIPE){
	       	
		if( s == BKSL ){
			s = getchar();	//BKSL case
			BKSLCheck = 1;
		}
		if( s == NL  || s == EOF) break;//Enter/EOF case
		
		length++;			//General case
		*w = s;
		w++;
		
		if( length > STORAGE - 2 ) break; //Buffer overflow check
		
		s = getchar();
	}
	
	//metacharacter cases
	if( length == 0 ){	//Must be no word to count metacharacter
		
		if( s == LT ){		//< case
			length++; 
			*w = s;
			w++;
			s = getchar();
			//Checking for double jeopardy
			if( s != LT && s != GT && s != PIPE && s != ANPS /*&& s != PND*/ ){
				ungetc( s, stdin);
				ungetcCheck = 1;
			}
		}
		else if( s == GT ){	//> case
			length++; 
			*w = s;
			w++;
			s = getchar();
			
			if( s == ANPS ){	//>& case
				length++; 
				*w = s;
				w++;
				s = getchar();
			}
			else if( s == GT ){	//>> case
				length++; 
				*w = s;
				w++;
				s = getchar();
				
				if( s == ANPS ){	//>>& case
					length++; 
					*w = s;
					w++;
					s = getchar();
				}
			}
			//Checking for double jeopardy
			if( s != LT && s != GT && s != PIPE && s != ANPS /*&& s != PND*/ ){
				ungetc( s, stdin);
				ungetcCheck = 1;
				}
		}
		else if( s == PIPE || /*s == PND ||*/ s == ANPS){	//Other metachar case
			length++; 
			*w = s;
			w++;
			s = getchar();
			//Checking for double jeopardy
			if( s != LT && s != GT && s != PIPE && s != ANPS /*&& s != PND*/ ){
				ungetc( s, stdin);
				ungetcCheck = 1;
				}
		}
	}
	
	
	*w = 0;			//Terminating the string
	w -= length;		//Resetting the char*
	
	if( length == 0 && s == EOF ) return -1;			//EOF case
	else if( length == 4 && strcmp( w, "done" ) == 0 ){		//[done] case
		ungetc( s, stdin );
		return -1;
	 
	}else{	//ungetc if it is a special character			//General case
		if( (s == NL && length != 0 && ungetcCheck == 0) ||
	    	    (s == LT || s == GT || s == PIPE || s == ANPS /*|| s == PND*/) 
		    && ungetcCheck == 0)
			ungetc( s, stdin );
		return length;
	}
	       
}
	
	
	
