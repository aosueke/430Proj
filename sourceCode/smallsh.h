#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "joblist.h"

#define EOL 1
#define ARG 2
#define AMPERSAND 3
#define SEMICOLON 4

#define MAXARG 512
#define MAXBUF 512

#define FOREGROUND 1
#define BACKGROUND 0
