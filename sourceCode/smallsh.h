#include <signal.h>
#include <stdlib.h>
#include <string.h>

#define MAX_JOBS 10  /* Maximum number of jobs */

/* Job states */
#define RUNNING 0
#define STOPPED 1
#define TERMINATED 2

/* Structure for jobs */
typedef struct {
    int job_id;
    pid_t pgid;   // Process group ID
    int state;    // RUNNING, STOPPED, or TERMINATED
    char command[MAXBUF]; // Command line
} job_t;

job_t jobs[MAX_JOBS];  // Array of jobs
int job_count;         // Number of jobs
