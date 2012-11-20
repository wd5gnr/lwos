
// Example Binary Clock using LWOS for Arudino
// Al Williams -- DJDJ
#include <LiquidCrystal.h>
#include "lwos.h"
#include "switchin.h"

// You can consider trying O and * to mimic LEDs
#define ONLED '1'
#define OFFLED '0'


/* definitions */
LiquidCrystal lcd(8,9,4,5,6,7);  // the LCD
int backlight=10;  // backlight (use PWM to dim)

unsigned hr;  // hours 
unsigned mm;  // minuts
unsigned ss;  // seconds
int blvalue=120;   // backlight PWM value
char modechar='*';  // 2nd line character


// Display binary 
// at LCD (x,y)
// Start with mask bit and work to the right
// So is mask is 8, you get 4 bits (8,4,2,1)
// if mask is 2, you get 2 bits (2,1)
void dispbin(int x, int y, int val, int mask)
{
  lcd.setCursor(x,y);
  while (mask)
  {
    lcd.print((val&mask)?ONLED:OFFLED);
    mask>>=1;
  }
}


// The Arduino is really C++
// lwos is, by default, C
// You could recompile lwos as C++
// but instead, I decided to force the
// tasks to C
extern "C" {
  
// This task wakes up periodically and 
// checks to see if more than 100mS has
// elapsed, if so task_tick is incremented
// I assume this will run at least fast enough
// to not miss. Also, the underlying 
// counter will roll over every 50 days
// That could be a problem
// if *t0+100 is, say, 0xFFFFFFFF
// and you don't react until t1 is 
// (for example) 4.
  int timing(Task *tcb)
  {
    unsigned long *t0=task_storage(unsigned long);
    unsigned long t1=millis();
    if (t1>(*t0+100))
    {
      task_add_tick((t1-*t0)/100);
      *t0=t1;
    }
    task_yield_next(); // release to next task
  }

// update every second and after 60 seconds
// update the minute
// The task wakes up every second (10 ticks)
  int secminutes(Task *tcb)
  {
    ss++;
    if (ss>59) {
      mm++;
      ss=0;
    }
    task_sleep(10);
  }

// This task wakes up every 10 seconds (which is overkill)
// and updates the idea of the hour.
  int hour(Task *tcb)
  {
    if (mm>59) 
    {
      mm=0;
      hr++;
      if (hr>23) hr=0;
    }
    task_sleep(100);
  }

// This function handles the display both for the do_disp task
// and for time setting
void do_disp(void)
{
    dispbin(0,0,hr/10,2);
    dispbin(3,0,hr%10,8);
    dispbin(8,0,mm/10,4);
    dispbin(12,0,mm%10,8);
}

// This task uses disp bin to show the current time
// it also handles the animation on the 2nd line of 
// the display
  int disp(Task *tcb)
  {
    int i;
    int *pos=task_storage(int);
    do_disp();
    lcd.setCursor(3,1);
    for (i=0;i<*pos;i++) lcd.print(" ");
    lcd.print(modechar);
    for (i=*pos;i<10;i++) lcd.print(" ");
    if (++*pos==10) *pos=0;
    task_sleep(10);
  }


// The UI needs to remember what mode it is in
// so it has this in local storage. I made it 
// a structure becuase I might add some more
// data here later
  struct uidata 
  {
    int mode;  // set mode
  };

// The ui task processes keyboard input. 
// The select button sets the defualt mode (mode 0)
// The left button sets mode 1 and the right mode 2
// The operation of the up/down buttons depends
// on the mode
// Mode 0 - Adjust backlight
// Mode 1 - Adjust hours
// Mode 2 - Adjust minutes

 int ui(Task *tcb)
  {
    struct uidata *data=task_storage(struct uidata);
    Serial.println(task_tick);
    int val=getswitch();  // read input
    int offset=0;    // assume no up and down
    switch (val)
    {
    case LEFT:
      data->mode=1;  // hour set
      modechar='H';
      break;
    case RIGHT:
      data->mode=2; // min set
      modechar='M';
      break;
    case SELECT:
      data->mode=0;  // backlight set  
      modechar='*';
      break;
    case UP:   // if up and down, set offset to 1 or -1
      offset=1;
      break;
    case DOWN:
      offset=-1;
      break;
    }
    
    // at this point if offset is set, do the change
    // based on mode
    if (offset!=0)
    {
      switch (data->mode)
      {
        case 0:
        // backlight up/down 10 at a time from 0 to 55
            blvalue+=offset*10;
            if (blvalue<0) blvalue=0;
            if (blvalue>255) blvalue=255;
            analogWrite(backlight,blvalue);  // write backlight
          break;
        case 1:
        // change hour
            hr+=offset;
            // note hr unsigned so any wrap will be >23
            if (hr>23) hr=0;
            do_disp();
          break;
        case 2:
        // change minutes
            mm+=offset;
            // note mm  unsigned so any wrap will be >59
            if (mm>59) mm=0;
            do_disp();
          break;
      }
    }
  task_sleep(4);
  }
 
  // The task table (priority order)
  TASK_TABLE
  TASK_DEF(TASK_READY,secminutes)
  TASK_DEF(TASK_READY,hour)
  TASK_DEF(TASK_READY,timing)
  TASK_DEF(TASK_READY,disp)
  TASK_DEF(TASK_READY,ui)
  TASK_TABLE_END   

}  // end of extern "C"





void setup() {
 lcd.begin(16,2);  // 16 character x 2 rows
  pinMode(backlight,OUTPUT);   // turn on backlight
  analogWrite(backlight,blvalue);  // and set about 1/2 way
  task_tick=32500;
  Serial.begin(57600);
  task_init();  // start lwos
}

// Since lwos does all the scheduling, there is no need for anything in loop
void loop() {
}


