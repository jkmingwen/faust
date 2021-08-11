#ifndef SIGTOSDF_HH
#define SIGTOSDF_HH

#include <fstream>
#include <iostream>
#include <string>
#include "signals.hh"
#include "sigtype.hh"
#include "SDF.hh"

using namespace std;

/**
 * Draw a list of signals L as a synchronous dataflow graph using
 * SDF3-compatible XML format
 */
void sigToSDF(Tree L, ofstream& fout);
static void recLog(Tree sig, set<Tree>& drawn, map<string, Actor>& actors,
                   map<string, Channel>& channels, int& chCount,
                   vector<string>& delayList, vector<string>& recList,
                   vector<string>& binopList); // store delay ('@'), recursive ('REC WN'), and binary operator actors in their own vectors
static string chAttr(Type t);
static string sigLabel(Tree sig);
void mergeChannels(string ch1, string ch2, map<string, Channel>& chList);
void bypassDelay(string delayActorName, string inputActorName,
                 map<string, Channel>& chList, map<string, Actor>& actorList);
void bypassRec(string recActorName, vector<string> inputSignalNames,
               map<string, Channel>& chList, map<string, Actor>& actorList);
string channelNameFromPort(Port port, map<string, Channel>& chList);
string channelNameFromActors(string srcActor, string dstActor,
                             map<string, Channel>& chList);
void updateBinopArguments(string oldArg, string newArg, vector<string>& binopList,
                          map<string, Actor>& actorList, map<string, Channel>& chList);
#endif  // SIGTOSDF_HH
