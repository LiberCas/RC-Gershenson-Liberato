// Main file of the serial port project.
// NOTE: This file must not be changed.

#include <stdio.h>

#include "applicationLayer.h"

#define BAUDRATE 9600
#define N_TRIES 3
#define TIMEOUT 4

// Arguments:
//   $1: /dev/ttySxx
//   $2: tx | rx
//   $3: filename
int main(int argc, char *argv[])
{
    mopen();
    return 0;
}
