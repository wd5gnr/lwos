/*  &copy; Copyright 2012 by Al Williams (al.williams@awce.com) 
    Distributed under the terms of the GNU Lesser General Public License */

/*! 
 * \file lwos.c
 * \author Al Williams
 * \date 30 Oct 2012
 * \brief Main library source for LWOS
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
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

/*! \mainpage Lightweight Operating System
 * &copy; Copyright 2012 by Al Williams (al.williams@awce.com)
 * Distributed under the terms of the GNU Lesser General Public License.

 * \section license License
    LWOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 * \section about About
 * This is a very simple and lightweight cooperative multitasking kernel.
 * Features include:
 * - Prioritization
 * - Task local storage
 * - Ability for a task to directly yield to another task
 * - Semaphore-like waiting
 * - Ability to sleep for a certain amount of time (with some user-supplied support)
 * Using LWOS is extremely simple. The file \a sample.c shows a simple
 * example. Minimally, you have to build a task table using the \a TASKTABLE
 * macro and some task functions. If you want to use time-based sleeping
 * you also need to provide a task to keep the scheduler's tick count (see
 * \a sleeping).
 * \section priority Task Priority
 * Entries in the task table are ordered by priority with the first
 * entry having the highest priority. If the first task is always
 * ready, for example, it will always run unless it does a directed
 * yield.
 * \section func Writing Task Functions
 * A task function is a simple C function that takes a pointer
 * to \a Task and returns an integer.
 * The scheduler will run the function until it returns. A return of 1
 * allows the schedule to continue execution with the highest
 * priority task that is ready (which may be the same function).
 * A non-zero return will yield to a specific task (1-based), or the first
 * task that is ready at that priority level or lower. Any
 * wait or yield will cause the function to reenter 
 * at the start on the next execution. You may yield from
 * a non-top-level function as long as TASK_NO_SETJMP is not defined.
 * \section sleeping Implementing Sleep
 * Because LWOS is portable, it depends on a user task to update
 * the \a task_tick variable. If you do not plan to use \a task_sleep
 * then you do not have to update \a task_tick. More to follow
 */

#include "lwos.h"


/*! \def LWOS_NOMAIN 
  \brief Define LWOS_NOMAIN if you want to supply your own main  and you are compiling lwos.c into your project
  Define this to prevent LWOS from including its own main if you are direct
  linking to your code.
 */
#ifndef LWOS_NOMAIN

//! The LWOS-defined main just calls \a task_init
int main(int argc, char *argv[])
{
  task_init();
  return 0;
}
#endif


//! This function is usually called by main and forms the basic OS logic
void task_init(void)
{
  // Our current spot in the task table
  unsigned int taskindex=0;
  int rv;
  while (1)
    {
      // don't go past the end
      if (taskindex>=task_max) taskindex=0; 
      // if this task is waiting...
      if (task_table[taskindex].state==TASK_WAIT)
	{
	  // see if its "semaphore" is zero and if so kick start it
	  if (task_table[taskindex].wait && !*(task_table[taskindex].wait))
	    {
	    task_table[taskindex].state=TASK_READY;
	    }
	  
	  // See if its timer has expired, if so kick start it
	  if (task_table[taskindex].wake && 
	      task_table[taskindex].wake<=task_tick)
	    {
	      task_table[taskindex].state=TASK_READY;
	      task_table[taskindex].wake=0;
	    }
	}
      // If this task is not ready then go to the next one
      if (task_table[taskindex].state!=TASK_READY) 
	{
	  taskindex++;
	  continue;
	}
      // Run this task
      // When it returns, a zero (yield) takes me to the top of the list
      // A non-zero jumps me to a specific task
      task_num=taskindex;
      task_current=task_table+taskindex;
#ifndef TASK_NO_SETJMP
      if (!(rv=setjmp(task_current->yieldbuf))) 
	{
#endif
		rv=task_current->taskfunc(task_current);
#ifndef TASK_NO_SETJMP
	}
#endif
      taskindex=rv==0?rv:rv-1;
    }
}
