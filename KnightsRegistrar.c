/*Cameron Bates
  COP3502C - C014
  Assignment #4
  July 6th, 2011
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ucfClass {
    char ID[10]; // class ID, ex: "COP-3502"
    char days[6]; // a combination of "MTWRF" (days)
    char time[21]; // class time, ex: "10:30 AM - 12:00 PM"
} class;

typedef struct ucfStudent {
    char lastName[21]; // student last name
    char firstName[21]; // student first name
    int ID; // student ID
    int laptopID; // serial number of the laptop the student picks up
    int errorCode; // flag to determine if they will make mistakes
    int numClasses; // number of classes the student will register
    class *classes; // array of said classes (2 be dynamically allocated)
    int enterTime; // time student arrived, in minutes, after 12:00 PM
    int timeLeft; // countdown timer to measure the 5 min. reg. process
    int timeRegistered; // Time student finished reg. and left Registrar
    struct ucfStudent *next;  // pointer to next student in queue
} student;

typedef struct laptopStack {
    int *id; //Dynamically allocated array of laptop IDs
    int top; //Used to check if
    int size; //Number of laptops
} stack;

typedef struct studentQueue {
    student *front; //Pointer that points to the front of the queue
    student *back; //Back of the queue
} queue;

//Stack functions
stack *initialize(int laptops);
int full(stack* stackPtr);
int empty(stack* stackPtr);
int push(stack* stackPtr, int value);
int pop(stack* stackPtr);
int top(stack* stackPtr);

//Queue functions
void init(queue *qPtr);
void enqueue(queue *qPtr, student *newStudent);
student* dequeue(queue* qPtr);
int emptyQ(queue *qPtr);
int front(queue *qPtr);

//Misc. functions
void printTime(FILE* fout, int t); //Prints the time in the correct format
student *check(queue *qPtr); //Used to check status of the first student in a queue without dequeueing
void decreaseTime(student *sPtr); //Decreases the time of all students in registration queue
student *insert(student *ll, student *node); //Inserts students in to an ordered linked list
void printList(FILE *fout, student *ll, int reg); //Prints list of students in linked list

int main()
{
    FILE *fin, *fout;
    int i, j, k;
    int size, id, days, students, time, reg = 0;
    student *studentLaptop = NULL;
    student *studentP;

    fin = fopen("KnightsRegistrar.in","r");
    fout = fopen("KnightsRegistrar.out","w");

    fscanf(fin, "%d", &size);

    //  Initialize both laptop stacks
    stack *lapStack = initialize(size);
    stack *delayStack = initialize(size);

    //  Push laptop IDs to main laptop stack
    for(i=0; i<size; i++)
    {
        fscanf(fin, "%d", &id);
        push(lapStack, id);
    }

    //  Read in how long the program runs for in days
    fscanf(fin, "%d", &days);

    //  Begin main for loop based on # of days
    for(i=1; i<=days; i++)
    {
        //  Read in # of students for current day
        fscanf(fin, "%d", &students);

        //  print out day banner
        fprintf(fout, "**********\n"
                      "Day %d:\n"
                      "**********\n\n", i);

        //  Create and initialize all queues needed for the day
        queue *outside = (queue*)malloc(sizeof(queue));
        init(outside);
        queue *laptopLine = (queue*)malloc(sizeof(queue));
        init(laptopLine);
        queue *returnLine = (queue*)malloc(sizeof(queue));
        init(returnLine);
        queue *registration = (queue*)malloc(sizeof(queue));
        init(registration);

        //  Create link list for the day and set it equal to null
        student *linkedList = (student*)malloc(sizeof(student));
        linkedList = NULL;

        //  Loop that fills in student information and enqueues them into the outside line
        for(j=0; j<students; j++)
        {
            student *newStudent = (student*)malloc(sizeof(student));
            fscanf(fin, "%d %s %s %d %d %d", &newStudent->enterTime, newStudent->lastName, newStudent->firstName,
                                          &newStudent->ID, &newStudent->numClasses, &newStudent->errorCode);

            newStudent->classes = (class*)malloc(newStudent->numClasses*sizeof(class));
            for(k=0; k<newStudent->numClasses;k++) //   Reads in class information
            {
                fscanf(fin, "%s %s", newStudent->classes[k].ID, newStudent->classes[k].days);
                fgets(newStudent->classes[k].time, 21, fin);
                strtok(newStudent->classes[k].time, "\n");
            }

            newStudent->laptopID = 0;
            newStudent->timeLeft = 0;
            newStudent->timeRegistered = 0;

            enqueue(outside, newStudent);
        }

        time = 0; reg = 0; //   Reset counters

        //  While loop that controls everything that happens per minute
        while((time < 301) || (!emptyQ(registration)) || (!emptyQ(returnLine)))
        {
            //  If the laptop return line is not empty,
            //  first take care of that.
            if(!emptyQ(returnLine))
            {
                student *student = check(returnLine); //    Look at first student in line

                if(student->errorCode == 0) //  If registration is successful, return laptop and leave
                {
                    student = dequeue(returnLine);
                    printTime(fout, time);
                    fprintf(fout, ":  %s %s successfully registered and returned laptop # %d.\n", student->firstName, student->lastName, student->laptopID);
                    student->timeRegistered = time;
                    push(delayStack, student->laptopID);
                    reg++;
                    linkedList = insert(linkedList, student); //    Insert registered student in to the ordered linked list
                }
                else // Else, student made an error and must start over
                {
                    student = dequeue(returnLine);
                    printTime(fout, time);
                    fprintf(fout, ":  %s %s made an error and must redo the registration.\n", student->firstName, student->lastName);
                    student->errorCode = 0;
                    student->timeLeft = 6;
                    enqueue(registration, student); //  Place student back in to registration queue
                }
            }



            //  If the laptop line is not empty,
            //  dequeue a student from the line and store them to a pointer
            if(!emptyQ(laptopLine))
            {
                if(studentLaptop==NULL)
                    studentLaptop = dequeue(laptopLine);
            }



            //  If there's a student waiting for a laptop,
            //  give them one and place them in the registration queue
            if(studentLaptop!=NULL)
            {
                if(studentLaptop->laptopID == 0)
                {
                    if(!empty(lapStack) && !empty(delayStack)) //   If there's a laptop in the delay stack, give them that laptop instead
                    {
                        studentLaptop->laptopID = pop(delayStack);
                        printTime(fout, time);
                        fprintf(fout, ":  %s %s has checked-out laptop # %d.\n", studentLaptop->firstName, studentLaptop->lastName, studentLaptop->laptopID);
                        studentLaptop->timeLeft = 6;
                        enqueue(registration, studentLaptop);
                        studentLaptop = NULL;

                    }
                    else if(!empty(lapStack)) //    Instead, give them one from the standard stack
                    {
                        studentLaptop->laptopID = pop(lapStack);
                        printTime(fout, time);
                        fprintf(fout, ":  %s %s has checked-out laptop # %d.\n", studentLaptop->firstName, studentLaptop->lastName, studentLaptop->laptopID);
                        studentLaptop->timeLeft = 6;
                        enqueue(registration, studentLaptop);
                        studentLaptop = NULL;

                    }
                }
            }



            //  If the registration line is not empty, decrease the time of all students in line
            //  and check to see if any are finished
            if(!emptyQ(registration))
            {
                student *studentPtr = check(registration); //   Check first student in line
                decreaseTime(studentPtr);   //  Decrease time of all students in line

                //  Check to see how many students finished registering at the same time
                while(studentPtr!=NULL)
                {
                    if(studentPtr->timeLeft <= 0) //    If finished registering, enter laptop return line
                    {
                        printTime(fout, time);
                        fprintf(fout, ":  %s %s finished registering and entered the laptop return line.\n", studentPtr->firstName, studentPtr->lastName);
                        studentPtr = dequeue(registration);
                        enqueue(returnLine, studentPtr);
                    }

                    studentPtr = studentPtr->next; //   Increment pointer
                }
            }



            //  If there's a laptop in the delay stack,
            //  place it in the standard stack
            if(!empty(delayStack))
            {
                push(lapStack, pop(delayStack));
            }



            //  Check to see if any students are waiting outside.
            //  If there are, place them in the laptop line.
            int inLine = 1;
            studentP = NULL;
            while(inLine!=0)
            {
                studentP = check(outside); //   Check first student in line.
                if(studentP!=NULL)
                {
                    if(time == studentP->enterTime) //  If enterTime is the current time, dequeue that student
                    {
                        studentP = dequeue(outside);
                        if(studentP!=NULL)
                        {
                            enqueue(laptopLine, studentP);
                            printTime(fout, time);
                            fprintf(fout, ":  %s %s has arrived at the Registrar and entered the laptop line.\n", studentP->firstName, studentP->lastName);
                        }
                    }
                    else{ inLine = 0; } //  No more students
                }
                else{ inLine = 0; } //  No more students
            }

            time++; //  Increment time
        }

        //  Print out students that registered
        fprintf(fout, "\n*** Day %d:  UCF Daily Registration Report ***:\n\n", i);
        printList(fout, linkedList, reg); //Prints out ordered linked list

        if(i != days)
            fprintf(fout, "\n");

        //  Free memory
        free(outside); free(laptopLine);
        free(registration); free(returnLine);
        free(linkedList);
    }

    //  Free lapstop stack
    free(lapStack->id);
    free(lapStack);

    return 0;
}

// Initializes a stack
stack *initialize(int laptops)
{
    stack *temp = (stack*)malloc(sizeof(stack));
    temp->id = (int*)malloc(laptops*sizeof(int));
    temp->top = -1;
    temp->size = laptops;

    return temp;
}

//  Checks to see if a stack is full
int full(stack *stackPtr){ return (stackPtr->top == stackPtr->size-1); }

//  Checks to see if a stack is empty
int empty(stack *stackPtr){ return(stackPtr->top == -1); }

//  Adds an item to a stack
int push(stack* stackPtr, int value)
{
    if(full(stackPtr))
        return 0;

    stackPtr->id[stackPtr->top+1] = value;
    (stackPtr->top)++;
    return 1;
}

//  Pops the top item off of a stack
int pop(stack *stackPtr)
{
    int returnVal;

    if(empty(stackPtr))
        return -1;

    returnVal = stackPtr->id[stackPtr->top];
    (stackPtr->top)--;

    return returnVal;
}

//  Returns to the top of a stack
int top(stack *stackPtr)
{
    if(empty(stackPtr))
        return -1;

    return stackPtr->id[stackPtr->top];
}

//  Initializes a queue to NULL
void init(queue *qPtr)
{
    qPtr->front = NULL;
    qPtr->back = NULL;
}

//  Adds a student to the end of a queue
void enqueue(queue* qPtr, student* newStudent)
{
    newStudent->next = NULL;

    if(qPtr->back!=NULL)
        qPtr->back->next = newStudent;

    qPtr->back = newStudent;

    if(qPtr->front == NULL)
        qPtr->front = newStudent;
}

//  Checks if a queue is empty
int emptyQ(queue *qPtr){ return qPtr->front == NULL; }

//  Converts time integer and prints out proper time format
void printTime(FILE* fout, int t)
{
	int hour = t/60, min = t%60;

	if (hour == 0){ fprintf(fout, "12");}

	else { fprintf(fout, "%d", hour); }

	fprintf(fout, ":%d%d PM", (min/10), (min%10));
}

//  Dequeues student out of a queue
student *dequeue(queue *qPtr)
{
    student *temp;

    if(qPtr->front == NULL)
        return NULL;

    temp = qPtr->front;

    qPtr->front = qPtr->front->next;

    if(qPtr->front == NULL)
        qPtr->back = NULL;

    temp->next = NULL;

    return temp;
}

//  Decreases time for all students
void decreaseTime(student *sPtr)
{
        while(sPtr!=NULL)
        {
            sPtr->timeLeft--;
            sPtr = sPtr->next;
        }
}

//  Places front of queue in to a student pointer
student *check(queue *qPtr)
{
    if (qPtr->front != NULL)
        return qPtr->front;
    else
        return NULL;
}

//  Takes a student node and inserts it into the linked list.
student *insert(student *ll, student *node)
{
    node->next = NULL;
    student *ptr = ll;
    student *temp;


    if(ll == NULL)
        return node;

    if(strcmp(node->lastName, ll->lastName) < 0 )
    {
        node->next = ll;
        return node;
    }

    while((ptr->next != NULL) && (strcmp(node->lastName, ptr->next->lastName) > 0))
        ptr = ptr->next;

    if(ptr->next != NULL)
    {
        if( strcmp(node->lastName, ptr->next->lastName) == 0 )
        {
            if( strcmp(node->firstName, ptr->next->firstName) < 0 )
            {
                temp = ptr->next;
                ptr->next = node;
                node->next = temp;
            }
            else
            {
                node->next = ptr->next->next;
                ptr->next = node;
            }
        }
        else
        {
            temp = ptr->next;
            ptr->next = node;
            node->next = temp;
        }
    }
    else
    {
        ptr->next = node;
    }

    return ll;
}

//  Runs through the student linked list and prints out
//  every successful registration.
void printList(FILE *fout, student *ll, int reg)
{
     student *help = ll;
     student *del;
     int i;

     fprintf(fout, "The Registrar received %d registrations as follows:\n\n", reg);

     while(help!=NULL) //   Run through students and print out registration information
     {
         fprintf(fout, "%s, %s, ID # %d\n", help->lastName, help->firstName, help->ID);
         fprintf(fout, "\tTime Registered:  "); printTime(fout, help->timeRegistered);
         fprintf(fout, "\n\tClasses:\n");

         for(i = 0; i < help->numClasses; i++)
            fprintf(fout, "\t| %-8s | %-5s |%-20s |\n", help->classes[i].ID, help->classes[i].days, help->classes[i].time);

        del = help;
        help = help->next;

        //  Free memory
        free(del->classes);
        free(del);
     }

     fprintf(fout, "\n");
}
