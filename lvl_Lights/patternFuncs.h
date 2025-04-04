#ifndef PATTERNFUNCS_H_INCLUDED
#define PATTERNFUNCS_H_INCLUDED

bool scrollToRight( unsigned int n, unsigned int step, unsigned int numInGroup );// returns state assignment
bool scrollToLeft( unsigned int n, unsigned int step, unsigned int numInGroup );

bool fillFromRight( unsigned int n, unsigned int step, unsigned int numInGroup ){}
bool fillFromLeft( unsigned int n, unsigned int step, unsigned int numInGroup ){}

bool crissCross( unsigned int n, unsigned int step, unsigned int numInGroup ){}
bool alternateBlink( unsigned int n, unsigned int step, unsigned int unusedParam ){}

#endif // PATTERNFUNCS_H_INCLUDED
