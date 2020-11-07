#ifndef __BEEP_H
#define __BEEP_H
void BEEP_Init(void);
#define BEEP PFout(8)=1
#define UNBEEP PFout(8)=0
#endif
