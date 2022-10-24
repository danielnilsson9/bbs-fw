#ifndef _INTELLISENSE_H_
#define _INTELLISENSE_H_

// NOTE:
// The defines below are here to keep IntelliSense 
// in Visual Studio happy and not throw incorrect errors.

#if !defined (SDCC) && !defined (__SDCC)

#define INTERRUPT(name, vector)					void name()
#define INTERRUPT_USING(name, vector,regnum)	void name()

#define __interrupt(vector)						

#define enableInterrupts()
#define disableInterrupts()

#define NOP()

#define _Bool uint8_t

#endif

#endif
