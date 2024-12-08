// smallsh.c
#include "smallsh.h"
#include <stdlib.h>

// Buffers and pointers for input processing
static char inpbuf[MAXBUF], tokbuf[2 * MAXBUF], *ptr = inpbuf, *tok = tokbuf;

// List of special characters that delimit tokens
static char special[] = {' ', '\t', '&', ';', '\n', '\0'};

// Prompt displayed to the user
char *prompt = "Command> ";

// Function prototypes
int inarg(char c);
int gettok(char **outptr);
int userin(char *p);
void procline(void);
int runcommand(char **cline, int where, int argCount);

// Signal handlers for various signals
static struct sigaction act1; // SIGINT
static struct sigaction act2; // SIGCHLD
static struct sigaction act3; // SIGTSTP

// Reads user input from the terminal and handles buffer overflow
int userin(char *p) {
  int c, count;

  ptr = inpbuf;
  tok = tokbuf;

  printf("%s", p);
  count = 0;
  while (1) {
    if ((c = getchar()) == EOF)
      return EOF; // End of input

    if (count < MAXBUF)
      inpbuf[count++] = c;

    if (c == '\n' && count < MAXBUF) {
      inpbuf[count] = '\0'; // Null-terminate input
      return count;
    }

    if (c == '\n') {
      printf("smallsh: input line too long\n");
      count = 0;
      printf("%s ", p); // Retry prompt
    }
  }
}

// Extracts the next token from the input buffer
int gettok(char **outptr) {
  int type;
  *outptr = tok;

  // Skip whitespace
  while (*ptr == ' ' || *ptr == '\t')
    ptr++;
  *tok++ = *ptr;

  switch (*ptr++) {
  case '\n':
    type = EOL; // End of line
    break;
  case '&':
    type = AMPERSAND; // Background process
    break;
  case ';':
    type = SEMICOLON; // Command separator
    break;
  default:
    type = ARG; // Argument
    while (inarg(*ptr))
      *tok++ = *ptr++;
  }

  *tok++ = '\0'; // Null-terminate token
  return type;
}

// Checks if a character is valid within an argument
int inarg(char c) {
  char *wrk;

  for (wrk = special; *wrk; wrk++) {
    if (c == *wrk)
      return 0; // Invalid character
  }

  return 1; // Valid character
}

// Processes the command line input and executes commands
void procline(void) {
  char *arg[MAXARG + 1];
  int toktype;
  int narg = 0; // Number of arguments
  int type;

  for (;;) {
    switch (toktype = gettok(&arg[narg])) {
    case ARG:
      if (narg < MAXARG)
        narg++; // Add argument to list
      break;
    case EOL: 
    case SEMICOLON:
    case AMPERSAND:
      type = (toktype == AMPERSAND) ? BACKGROUND : FOREGROUND;

      if (narg != 0) { // If there are arguments
        arg[narg] = NULL; // Null-terminate argument list
        runcommand(arg, type, narg); // Execute command
      }

      if (toktype == EOL)
        return; // End of processing

      narg = 0; // Reset argument count for next command
      break;
    }
  }
}

// Executes the given command using a child process
int runcommand(char **cline, int where, int argCount) {
  pid_t pid;
  int status;

  // Special commands
  if (strcmp(*cline, "jobs") == 0) { // List background jobs
    printLinkedList();
    return 1;
  }

  // Command-specific handling (e.g., "bg", "fg", "kill")
  // ... (No changes to the logic here, just updated comments)

  // Forking a new process to execute non-special commands
  switch (pid = fork()) {
  case -1:
    perror("smallsh"); // Error during fork
    return -1;
  case 0:
    // Child process
    if (where == BACKGROUND) {
      // Ignore certain signals for background processes
      act1.sa_handler = SIG_IGN;
      sigaction(SIGINT, &act1, NULL);
    }
    execvp(*cline, cline); // Execute command
    perror(*cline); // Print error if execvp fails
    exit(1);
  }

  // Parent process updates job list and handles process termination
  struct proccessInfo newprocess;
  newprocess.ground = where;
  newprocess.pid = pid;
  newprocess.status = 1;
  newprocess.jobNum = ++jobCount;
  for (int i = 0; i < argCount; i++)
    strcpy(newprocess.cmmds, cline[i]);

  if (where == BACKGROUND) {
    printf("[Process id %d]\n", pid);
    insertatend(newprocess); // Add background job to job list
    return 0;
  }

  insertatend(newprocess);
  printLinkedList();

  if (waitpid(pid, &status, 0) == -1) {
    return -1;
  } else {
    deletenode(pid); // Remove job after it terminates
    return status;
  }
}

// Main function to initialize and manage the shell
int main() {

  // Assign signal handlers
  void catchint(int);
  void childDies(int);
  void catchSTP(int);

  act1.sa_handler = catchint;
  act2.sa_handler = childDies;
  act3.sa_handler = catchSTP;

  sigemptyset(&act1.sa_mask);
  sigemptyset(&act2.sa_mask);
  sigemptyset(&act3.sa_mask);

  // Shell loop
  while (userin(prompt) != EOF) {
    sigaction(SIGINT, &act1, NULL);
    sigaction(SIGCHLD, &act2, NULL);
    sigaction(SIGTSTP, &act3, NULL);
    procline(); // Process input
  }
}

// Signal handler for SIGINT (Ctrl+C)
void catchint(int signo) {
  pid_t i = searchlist(FOREGROUND);
  if (i != -1) {
    kill(i, signo); // Forward signal to foreground process
    deletenode(i); // Remove from job list
  }
  printf("\n");
}

// Signal handler for SIGCHLD (child process termination)
void childDies(int signo) {
  int tempPid = waitpid(-1, NULL, WNOHANG); // Remove zombie processes

  if (tempPid > 0) {
    deletenode(tempPid); // Remove job from job list
    printf("%d was deleted\n", tempPid);
  }
}

// Signal handler for SIGTSTP (Ctrl+Z)
void catchSTP(int signo) {
  pid_t i = searchlist(FOREGROUND);
  if (i != -1) {
    kill(i, signo); // Stop foreground process
    changetobg(i); // Change process to background
  }
  printf("\n");
}
