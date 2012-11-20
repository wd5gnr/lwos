/*! 
 * \file sample.c
 * \author Al Williams
 * \date 30 Oct 2012
 * \brief Simple example of LWOS features
 *
 * To build:
 * gcc -g -o sample sample.c lwos.c
 */
#include <stdio.h>
#include "lwos.h"
#include <stdlib.h>
#include <time.h>


//! This is a user task that updates the \a task_tick
//! used for sleeping.
//! Note that whatever task updates the tick can't use timer wait
//! In this case, the tick is simply a 1 second count
int task0(Task *tcb)
{
  // timer task
  time_t t1,*t;
  // Get a thread local time_t
  t=task_storage(time_t);
  // Get the current time
  time(&t1);
  // If in a new second, update everything
  if (t1!=*t) {
    task_add_tick(1);
    *t=t1;
  }
  // got to the next priority level
  task_yield_next();
}


//! This is a wait variable (like a semaphore)
//! The idea is that task1 will go as fast as possible
//! for 10 times and then wait on the semaphore
//! Then task2 will run once per second for 10 seconds
//! and unlock the wait variable 
TASKWAIT_t task1lock=0;

//! A simple task that prints something 10 times
//! and then blocks 
int task1(Task *tcb)
{
  int *p;
  // Get a task local int
  p=task_storage(int);
  printf("Task1\n");
  // keep track of how many times
  *p+=1;
  if (*p>10) 
    {
      *p=0;
      task_wait(task1lock); // block after 10
    }
  else
    task_yield();
}

//! A simple task that prints once per second
//! It also wakes up task 1 periodically
int task2(Task *tcb)
{
  int *p;
  // Get an integer
  p=task_storage(int);
  printf("Task2 %d\n",task_tick);
  *p+=1;
  if  (*p>=10)
    {
      // time to wake up task1 
      *p=0;
      task_release(task1lock);
    }
  // sleep for about 1 second
  task_sleep(1);
}

// The task table
TASK_TABLE
TASK_DEF(TASK_READY,task0)
TASK_DEF(TASK_READY,task1)
TASK_DEF(TASK_READY,task2)
TASK_TABLE_END   

