#include "patternFuncs.h"

bool scrollToRight( unsigned int n, unsigned int step, unsigned int numInGroup )// returns state assignment
{
    return ( n >= step && n < step + numInGroup );
}

bool scrollToLeft( unsigned int n, unsigned int stepIter, unsigned int numInGroup )
{
 //   return ( n <= numLts - 1 - step ) && ( n + numInGroup > numLts - 1 - step );// uh oh
    return false;
}
