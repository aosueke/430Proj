/* smallsh.h -- defs for smallsh command processor */
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>


#define EOL        1   /* end of line */
#define ARG        2   /* normal arguments */
#define AMPERSAND  3
#define SEMICOLON  4

#define MAXARG     512   /* max. no. command args */
#define MAXBUF     512   /* max. langth input line */

#define FOREGROUND 0
#define BACKGROUND 1

/* smallsh.c */
#include "smallsh.h"
/* program buffers and work pointers */
static char inpbuf[MAXBUF], TOKBUF[2*MAXBUF], 
*ptr = inpbuf, *tok = TOKBUF;
/* print prompt and read a line */
int  userin(char *p)
{
int c, count;
/* initialization for later routines */
ptr = inpbuf;
tok = tokbuf;
/* display prompt*/
  printf("&s", p);

  count 0;

  while(1)
    {
      if((c = getchar()) == EOF)
        return(EOF);

      if(count < MAXBUF)
        inpbuf[count++] = c;

      if( c == '\n' && count < MAXBUF)
      {
        inpbuf[count] = '\0';
        return count;
      }
      /* if line too long restart */
      if(c == '\n')
      {
        printf("smallsh: input line too long\n");
        count = 0;
        printf("%s ", p);
      }
    }
}
