#include "v4l2dev.h"
#include <stdio.h>

extern "C" int function(sampleObject *s){
  strcpy(s->Name,"Raina");
  strcpy(s->Role,"Cricketer");
  return 1;
}
