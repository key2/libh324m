/*
	AN EFFICIENT GOLAY CODEC FOR
	MIL-STD-188-141A AND FED-STD-1045
	ERIC E. JOHNSON
	NMSU-ECE-91-001 FEBRUARY 1991
	New Mexico State University
	Dept. 3-O
	Las Cruces, NM 88003-0001
*/

#include "encode_table.h" /* P matrix "int enc[4096] = { ... }" */
#define myencode(x) (((long)x<<12)|enc[x])
#include "errwt.h" /* error weight matrix "int wt[4096] = { ... }" */
#include "errpat.h"/* error pattern matrix "int e[4096] = { ... }" */
#include <stdio.h>
#define ERROR_DETECTED -1
/******************** GOLAY DECODER MODULE *********************/

int errwt(long w)
{
	register int i;
	i = (myencode(w>>12) ^ w) & 07777;
	return (wt[i]);
}


long Gdecode(long w, long power) /* decode one 24-bit Golay word */
{
	register int i;
	if (errwt(w) > power) return (ERROR_DETECTED);
	i = (myencode(w>>12) ^ w) & 07777;
	return (w>>12 ^ e[i]);
}


long Gencode(long w)
{
  return myencode(w);
}

