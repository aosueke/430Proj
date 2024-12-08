#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

struct proccessInfo{
	pid_t pid;  // Process ID
	int status;  // Process status (running or stopped)
    int ground;  // Process type (foreground or background)
    int jobNum;  // Job number
    char cmmds[100];  // Command associated with the process
    int cmdCount;  // Number of commands in the job
};

struct node {
   struct proccessInfo data;  // Data of the node (process info)
   struct node *next;  // Pointer to the next node
};

struct node *head = NULL;  // Head pointer for the linked list
struct node *current = NULL;  // Pointer to traverse the list

int jobCount = 0;  // Keeps track of the total number of jobs

// Function to print the contents of the job list (all jobs)
void printLinkedList()
{
    if(head == NULL)  // Check if the list is empty
    {
        printf("list is empty\n");
        return;
    }
    struct node *cur = head;
    int count = 0;
    printf("Job#\tPid\tStatus\tGround\t\tCommand\n");  // Print job details

    // Traverse through the list and print each job's details
    while(cur != NULL)
    {
    	char tempS1[20] = "";
    	char tempS2[20] = "";

    	// Set status text based on process status
    	if(cur->data.status == 0){
    		strcpy(tempS1, "Stopped");
    	}else{
    		strcpy(tempS1, "Running");
    	}

    	// Set ground text based on process type (foreground/background)
    	if(cur->data.ground == 1){
    		strcpy(tempS2, "Foreground");
    	}else{
    		strcpy(tempS2, "Background");
    	}

        // Print the job details
        printf("%d\t%d\t%s\t%s\t%s\n", cur->data.jobNum, cur->data.pid, tempS1, tempS2, cur->data.cmmds);

        count++;  // Increment the job count
        cur = cur->next;  // Move to the next node
    }
}

// Insert a job at the beginning of the list
void insertatbegin(struct proccessInfo data){
   struct node *lk = (struct node*) malloc(sizeof(struct node));  // Allocate memory for the new node
   lk->data = data;  // Assign data to the new node

   // Point the new node to the old first node
   lk->next = head;

   // Set the head pointer to the new node
   head = lk;
}

// Insert a job at the end of the list
void insertatend(struct proccessInfo data){
   struct node *lk = (struct node*) malloc(sizeof(struct node));  // Allocate memory for the new node
   lk->data = data;

   if(head == NULL)  // If the list is empty, insert at the beginning
   {
       insertatbegin(data);
       return;
   }
   struct node *linkedlist = head;

   // Traverse the list to find the last node
   while(linkedlist->next != NULL)
      linkedlist = linkedlist->next;

   // Point the last node to the new node
   linkedlist->next = lk;
}

// Insert a job after a specific node in the list
void insertafternode(struct node *list, struct proccessInfo data){
   struct node *lk = (struct node*) malloc(sizeof(struct node));  // Allocate memory for the new node
   lk->data = data;
   lk->next = list->next;  // Link the new node to the next node
   list->next = lk;  // Link the previous node to the new node
}

// Delete the job at the beginning of the list
void deleteatbegin(){
   head = head->next;  // Move the head pointer to the next node
}

// Delete the job at the end of the list
void deleteatend(){
   struct node *linkedlist = head;
   while (linkedlist->next->next != NULL)  // Traverse to the second last node
      linkedlist = linkedlist->next;

   // Remove the last node
   linkedlist->next = NULL;
}

// Delete a job with a specific PID
void deletenode(pid_t key){
   struct node *temp = head, *prev;
   if (temp != NULL && temp->data.pid == key) {
      head = temp->next;  // If the node to be deleted is the first node
      return;
   }

   // Traverse through the list to find the node to delete
   while (temp != NULL && temp->data.pid != key) {
      prev = temp;
      temp = temp->next;
   }

   // If the key is not found, return
   if (temp == NULL) return;

   // Remove the node
   prev->next = temp->next;
}

// Search for a job by its ground value (foreground or background) and return the corresponding PID
pid_t searchlist(int key){
   struct node *temp = head;
   while(temp != NULL) {
      if (temp->data.ground == key) {  // Check if the job's ground matches the input key
         return temp->data.pid;  // Return the PID
      }
      temp = temp->next;  // Move to the next node
   }
   return -1;  // Return -1 if no job is found
}

// Change the status of the foreground process to stopped and move it to the background
int contBgStopFg()
{
    struct node* temp = head;
    if(temp == NULL){  // If the list is empty, return
        return -1;
    }

    // Traverse the list to find the processes to update
    while (temp != NULL)
    {
        if (temp->data.ground == 0){  // If it's a background process
            if(temp->data.status == 0){  // If the background process is already stopped
                temp = temp->next;
                continue;
            }
            temp->data.status = 1;  // Change the status to running
            kill(temp->data.pid, SIGCONT);  // Resume the process
        }
        else if(temp->data.ground == 1){  // If it's a foreground process
            temp->data.status = 0;  // Stop the foreground process
            temp->data.ground = 0;  // Change the ground value to background
            kill(temp->data.pid, SIGSTOP);  // Stop the process
        }
        temp = temp->next;  // Move to the next process
    }
    return 0;  // Return success
}

// Change a stopped background process to running
pid_t bgToRun(int jobnum)
{
    if(head == NULL){  // If the list is empty, return error
        printf("list is empty and nothing to delete\n");
        return -1;
    }

    struct node* cur = head;  // Point the pointer to the first node

    // Traverse the list to find the job with the specified job number
    while(cur->data.jobNum != jobnum || cur->next != NULL)
    {
        cur = cur->next;
    }

    if(cur->data.jobNum == jobnum){
        cur->data.status = 1;  // Change the job status to running
        return cur->data.pid;  // Return the PID of the job
    }
    return -1;  // Return -1 if job is not found
}

// Change a background process to foreground
pid_t bgTofg(int jobnum)
{
    if(head == NULL){  // If the list is empty, return error
        printf("list is empty and nothing to delete\n");
        return -1;
    }

    struct node* cur = head;  // Point the pointer to the first node

    // Traverse the list to find the job with the specified job number
    while(cur->data.jobNum != jobnum)
    {
        if(cur->next == NULL){
            break;
        }
        cur = cur->next;
    }

    if(cur->data.status == 0){  // If the background job was stopped
        cur->data.status = 1;  // Change the status to running
        cur->data.ground = 1;  // Change the job to foreground
        printLinkedList();  // Print the updated list
        return cur->data.pid;  // Return the PID
    }

    cur->data.ground = 1;  // Change to foreground if the job is running
    printLinkedList();  // Print the updated list
    return -1;  // Return -1 if job was already running
}

// Move a job with a specific PID to the background
void changetobg(pid_t i)
{
    struct node *temp = head;
    while(temp != NULL) {
        if (temp->data.pid == i) {
            temp->data.ground = 0;  // Change the job to background
            temp->data.status = 0;  // Change the status to stopped
        }
        temp = temp->next;  // Move to the next job
    }
}
