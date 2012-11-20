/*  &copy; Copyright 2012 by Al Williams (al.williams@awce.com) 
    Distributed under the terms of the GNU Lesser General Public License */
/*! 
 * \file lwos.h
 * \author Al Williams
 * \date 30 Oct 2012
 * \brief Header for LWOS programs
 *
 * Lightweight OS by Al Williams (al.williams@awce.com)
 * A very simple "OS" written in ordinary C that provides
 * cooperative multitasking and is simple enough to port
 * to very small microcontrollers that have a  C compiler
 * 
 * The lwos file has its own main() (although you
 * can suppress this by defining LWOS_NOMAIN.

    This file is part of LWOS.

    LWOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef __LWOS_H
#define __LWOS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TASK_NO_SETJMP
#include <setjmp.h>
#endif

//! \def TASKWAIT_t 
//! \brief Define for the type used for a semaphore
//! Define for the type used for the semaphore. Defaults to unsigned char.
#ifndef TASKWAIT_t
#define TASKWAIT_t unsigned char
#endif


//! State of a task (stopped, ready, or waiting on an event)
typedef enum 
  {
    TASK_STOPPED=-1,
    TASK_READY=0,
    TASK_WAIT=1
  } TaskState;

//! This is set by (usually) one user task
//! to represent time in some way (e.g., # of seconds or milliseconds)

// Note: this needs to be signed so that overflow works right
extern int task_tick;


//! Each row of the \a task_table contains data about the task
typedef struct _task
{
//! Task's run state
  TaskState state;  
//! Task's function
  int (*taskfunc)(struct _task *tcb);  
//! Pointer to task local storage (see \a task_alloc)
  void *taskdata;  
//! If waiting for a semaphore, point to it
  TASKWAIT_t *wait;  
//! If waiting for a tick count, here it is
  int wake;    
#ifndef TASK_NO_SETJMP
//! If \a TASK_NO_SETJMP is not defined use setjmp for embedded yield
  jmp_buf yieldbuf;
#endif  
} Task;

//! Number of entries in the task table
extern unsigned task_max;

//! Current running task (used by many macros; also passed
//! as an argument to the task
extern Task *task_current;

//! Current running task (as an index into \a task_table)
extern unsigned task_num;

//! The table of tasks. 
extern Task task_table[];

//! Macro to create task table and other global vars
#define TASK_TABLE \
  Task task_table[]= {

//! Macro to define a row in the task table
//! \param rdy - Task ready state (usually TASK_READY)
//! \param func - The task function
#ifndef TASK_NO_SETJMP
#define TASK_DEF(rdy,func) { rdy, func, NULL, NULL, 0, 0, 0 },
#else
#define TASK_DEF(rdy,func) { rdy, func, NULL, NULL, 0, 0},
#endif

//! Macro to end the task table and define some more globals
#define TASK_TABLE_END };	  \
  unsigned task_max=sizeof(task_table)/sizeof(task_table[0]);	


#ifndef TASK_NO_SETJMP
//! Yield to a specific task (or next lowest priority that is ready)
#define task_yield_to(n) longjmp(task_current->yieldbuf,(n)+1)

//! Yield normally
#define task_yield() task_yield_to(0)

//! Yield to next highest priority task that is ready
#define task_yield_next() task_yield_to(task_num+1)
#else
#define task_yield() return 1
#define task_yield_to(n) return ((n)+1
#define task_yield_next() return (task_num+2)
#endif

//! Allocate or fetch task local 
//! \param dstruct - name of data type for storage
#define task_storage(dstruct) (dstruct *)(!task_current->taskdata?	 \
	       task_current->taskdata=calloc(1,sizeof(dstruct)): \
	       task_current->taskdata)
  
//! Wait on a flag to be set to zero
//! \param sem - Name of wait variable
//! \param n - Value to set in wait variable
#define task_waitn(sem,n) { sem=1, task_current->wait=&sem, \
      task_current->state=TASK_WAIT; task_yield();   }
  
//! Wait on a flag to be set to zero
//! \param sem - Name of wait variable
#define task_wait(sem) task_waitn(sem,1)

   
//! Release a wait variable
//! \param sem - Name of wait variable
#define task_release(sem) sem=0  

//! Decrement a wait variable
//! \param sem - Name of wait variable
#define task_decr(sem) --sem

//! Sleep for given tick count
//! \param ticks - Number of ticks to wait
#define task_sleep(ticks) { task_current->wake=ticks,		\
      task_current->state=TASK_WAIT;	 task_yield();	 }


//! Add n to the task_tick count
//! User code must call this directly and not update task_tick directly
//! If n==0 it is treated as 
void task_add_tick(int n);

//! Start LWOS -- typically called from default main
void task_init(void);

#ifdef __cplusplus
}
#endif

#endif
