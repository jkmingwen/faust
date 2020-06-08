#ifndef SIGTOSDF_HH
#define SIGTOSDF_HH

#include <fstream>
#include <iostream>
#include <string>
#include "signals.hh"

using namespace std;

/**
 * Draw a list of signals L as a synchronous dataflow graph using
 * SDF3-compatible XML format
 */
void sigToSDF(Tree L, ofstream& fout);

#endif  // SIGTOSDF_HH
