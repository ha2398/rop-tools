/**
 * @author: Hugo Sousa (hugosousa@dcc.ufmg.br)
 *
 * vcalls.cpp: Pintool used to determine valid calls in an application's
 * execution.
 */


#include "pin.H"
#include <iostream>
#include <fstream>

/**
 * Prints the correct usage of the pintool.
 */
void printUsage() { } // TODO



int main(int argc, char *argv[])
{
    bool error = false;

    // Starts Pin and checks parameters.
    error = PIN_Init(argc, argv);
    if (error) {
        printUsage();
        return(1);
    }

    PIN_StartProgram();

    return 0;
}