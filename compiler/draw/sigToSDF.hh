#ifndef SIGTOSDF_HH
#define SIGTOSDF_HH

#include <fstream>
#include <iostream>
#include <string>
#include "signals.hh"
#include "SDF.hh"

using namespace std;

/**
 * Draw a list of signals L as a synchronous dataflow graph using
 * SDF3-compatible XML format
 */
void sigToSDF(Tree L, ofstream& fout);
static void recdraw(Tree sig, set<Tree>& drawn, ofstream& fout,
                    map<string, Actor>& actors);
static string sigLabel(Tree sig);

#endif  // SIGTOSDF_HH
