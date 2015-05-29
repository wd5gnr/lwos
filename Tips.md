A few notes about design patterns to use with lwos.

# Tick Counter #
You can use the tick counter for anything, not just time. For example, you can implement round robin by having the last task in the table do nothing but bump the tick counter by one. All other tasks do their thing and then sleep for 1 tick. Then everyone will run round robin. You can get the same effect, of course, by just yielding to the next task every time.

# Resume task at midpoint #
You can use local storage and a switch to "resume" a task in the  middle. Suppose you'd like to write:

```
while (1) {
  a();
  yield();
  b();
  yield();
  }
```

This won't work because yield never returns. The reschedule goes to the top of the task function. However, if you could do something like this:

```
typedef struct
   {
   int phase;
    // local variables here
    } Xlocal;

int taskX(Task *tcb)
  {
  XLocal xlocal=task_storage(XLocal);
  // note that xlocal is zeroed out on allocation
    switch (xlocal->phase)
      {
case 0:   a();
          xlocal->phase=1;
          yield();

case 1:   b();
          xlocal->phase=0;
          yield();
      }
   }
```

Note you could put break statements in each case, but since yield never returns, it doesn't hurt to omit them.