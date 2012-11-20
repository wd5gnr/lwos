#include <Arduino.h>
#include "switchin.h"

static int switchin=0;

// read a switch
int getswitch(void) {
  int val=analogRead(switchin);
  if ( val<100 ) return RIGHT;
  if ( val<200 ) return UP;
  if ( val<400 ) return DOWN;
  if ( val<500 ) return LEFT;
  if ( val<800 ) return SELECT;
  return NONE;
}
