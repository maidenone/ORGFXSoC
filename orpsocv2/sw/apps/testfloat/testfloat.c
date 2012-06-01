
/*
===============================================================================

This C source file is part of TestFloat, Release 2a, a package of programs
for testing the correctness of floating-point arithmetic complying to the
IEC/IEEE Standard for Floating-Point.

Written by John R. Hauser.  More information is available through the Web
page `http://HTTP.CS.Berkeley.EDU/~jhauser/arithmetic/TestFloat.html'.

THIS SOFTWARE IS DISTRIBUTED AS IS, FOR FREE.  Although reasonable effort
has been made to avoid it, THIS SOFTWARE MAY CONTAIN FAULTS THAT WILL AT
TIMES RESULT IN INCORRECT BEHAVIOR.  USE OF THIS SOFTWARE IS RESTRICTED TO
PERSONS AND ORGANIZATIONS WHO CAN AND WILL TAKE FULL RESPONSIBILITY FOR ANY
AND ALL LOSSES, COSTS, OR OTHER PROBLEMS ARISING FROM ITS USE.

Derivative works are acceptable, even for commercial purposes, so long as
(1) they include prominent notice that the work is derivative, and (2) they
include prominent notice akin to these four paragraphs for those parts of
this code that are retained.

Modified for use with or1ksim's testsuite.

Contributor Julius Baxter <julius.baxter@orsoc.se>


===============================================================================
*/
								 /*
#include <stdlib.h>
#include <signal.h>
#include <string.h>
								 */
#include "cpu-utils.h" // OR1k support C library
#include "milieu.h"
#include "printf.h"
#include "spr-defs.h"
#include "int.h"
#include "fail.h"
#include "softfloat.h"
#include "testCases.h"
#include "testLoops.h"
#include "systflags.h" // defines fpcsr_flags global
#include "testFunction.h"
								 /*
static void catchSIGINT( int signalCode )
{

    if ( stop ) exit( EXIT_FAILURE );
    stop = TRUE;

}
*/

// Clear flags in floating point control/status reg (FPCSR), and enable
// exception
void inline or1k_enable_fpee(void)
{
  // Read the FPCSR
  unsigned int spr = SPR_FPCSR;
  unsigned int value;
  // Read the SPR
  asm("l.mfspr\t\t%0,%1,0" : "=r" (value) : "r" (spr));

  // Clear flags in FPCSR
  value &= ~(SPR_FPCSR_ALLF);

  // Re-enable floating point exceptions
  value |= SPR_FPCSR_FPEE;
  
  // Write value back to FPCSR
  asm("l.mtspr\t\t%0,%1,0": : "r" (spr), "r" (value));
  
}
  // Interrupt handler for floating point exceptions
  // Just copy out the flags and go back to work...

void float_except_handler(void)
{
  
  // Read the FPCSR
  unsigned int spr = SPR_FPCSR;
  unsigned int value;
  
  // Clear the global we'll use 
  fpcsr_flags = 0;
  // Read the SPR
  asm("l.mfspr\t\t%0,%1,0" : "=r" (value) : "r" (spr));
  
  // Extract the flags from OR1K's FPCSR, put into testfloat's flags format  
  if (value & SPR_FPCSR_IXF)
    fpcsr_flags |= float_flag_inexact;

  if (value & SPR_FPCSR_UNF)
    fpcsr_flags |= float_flag_underflow;

  if (value & SPR_FPCSR_OVF)
    fpcsr_flags |= float_flag_overflow;

  if (value & SPR_FPCSR_DZF)
    fpcsr_flags |= float_flag_divbyzero;

  if (value & SPR_FPCSR_IVF)
    fpcsr_flags |= float_flag_invalid;
  
  //  printf("testfloat: getting flags, FPCSR: 0x%x, softfloatflags: 0x%x\n", 
  //         value & SPR_FPCSR_ALLF, flags);

  // This clears flags and re-enables FPEE bit
  or1k_enable_fpee();

}

// Running this bare metal standalone for OR1K - hard set the configuration
int
main( int argc, char **argv )
{
  //    char *argPtr; // Unused variable
    flag functionArgument;
    uint8 functionCode;
    int8 operands, roundingPrecision, roundingMode;

    // Add exception handler for floating point exception
    add_handler(0xd, float_except_handler);

    // Enable floating point exceptions in FPCSR
    or1k_enable_fpee();

    // If we have UART init it:
#ifdef _UART_H_
    uart_init(DEFAULT_UART);
#endif

    printf("testfloat\n");

    fail_programName = "testfloat";
    //if ( argc <= 1 ) goto writeHelpMessage;
    testCases_setLevel( 1 );
    trueName = "soft";
    testName = "syst";
    errorStop = FALSE;
    forever = FALSE;
    maxErrorCount = 0;
    trueFlagsPtr = &float_exception_flags;
    testFlagsFunctionPtr = syst_float_flags_clear;
    tininessModeName = 0;
    operands = 0;
    roundingPrecision = 0;
    roundingMode = 0;// ROUND_DOWN// - for only round down tests ; 
                     //0 - for do all rounding modes 
    // "all" setting:
    functionArgument = TRUE;
    functionCode = 0; // See testFunction.c for list. 
    // 0 = all possible functions
    // 9 = float32_to_int32
    // 10 = float32_to_int32_round_to_zero
    // 17 = float32_add
    // 18 = float32_sub
    // 19 = float32_mul
    // 20 = float32_div
    // 23 = float32_eq
    // 24 = float32_le
    // 25 = float32_lt

    operands = 0;

    if ( ! functionArgument ) fail( "Function argument required" );
    //    (void) signal( SIGINT, catchSIGINT );
    //    (void) signal( SIGTERM, catchSIGINT );
    if ( functionCode ) {
        if ( forever ) {
            if ( ! roundingPrecision ) roundingPrecision = 80;
            if ( ! roundingMode ) roundingMode = ROUND_NEAREST_EVEN;
        }
        testFunction( functionCode, roundingPrecision, roundingMode );
    }
    else {

      for ( functionCode = 1;
                  functionCode < NUM_FUNCTIONS;
                  ++functionCode
                ) {
	//printf("trying function %d\n",functionCode);
                if ( functionExists[ functionCode ] ) {
                    testFunction(
                        functionCode, roundingPrecision, roundingMode );
                }
            }

    }
    exitWithStatus();

    // Should never reach here
    return 1;

}

