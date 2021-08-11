#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "exception.hh"
#include "sigToSDF.hh"
#include "signals.hh"
#include "sigtype.hh"
#include "sigtyperules.hh"
#include "xtended.hh"

using namespace std;

/**
 * Draw a list of signals as a synchronous dataflow graph using
 * SDF3-compatible XML format
 */
void sigToSDF(Tree L, ofstream& fout)
{
    set<Tree> alreadyDrawn;

    map<string, Actor> actorList;
    map<string, Channel> chList;
    int chCount = 0;
    int outCount = 0;
    vector<string> delayActors;
    vector<string> recActors;
    vector<string> binopActors;
    const string graphName = gGlobal->gMasterName; // name of .dsp file

    while (isList(L)) {
        recLog(hd(L), alreadyDrawn, actorList, chList, chCount,
               delayActors, recActors, binopActors);
        // add output node (and related ports/channels) to relevant lists
        string outName("OUTPUT_" + to_string(outCount));
        actorList.insert(pair<string, Actor>(outName,
                                             Actor(outName, outName)));
        stringstream srcActor;
        srcActor << hd(L);
        string chName("channel_" + to_string(chCount) + chAttr(getCertifiedSigType(hd(L))));
        string srcPortName("in_" + chName);
        string dstPortName("out_" + chName);
        actorList.at(srcActor.str()).addPort(Port(srcPortName,
                                                  portType::out,
                                                  1));
        actorList.at(outName).addPort(Port(dstPortName,
                                           portType::in,
                                           1));
        chList.insert(pair<string, Channel>(chName,
                                            Channel(chName,
                                                    srcActor.str(), srcPortName,
                                                    outName, dstPortName,
                                                    1, 0)));
        chCount++;
        outCount++;
        L = tl(L);
    }

    // Write graph information to XML
    fout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         << "<sdf3 type=\"sdf\" version=\"1.0\"\n"
         << "    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
         << "    xsi:noNamespaceSchemaLocation=\"http://www.es.ele.tue.nl/sdf3/xsd/sdf3-csdf.xsd\">"
         << endl;
    fout << "<applicationGraph name='" << graphName << "'>" << endl;
    fout << "    <sdf name='" << graphName << "' type='" << graphName << "'>" << endl;
    // Bypass REC actors for SDF
    for (auto& r : recActors) {
      vector<string> inputActorNames = actorList.at(r).getInputSignalNames();
      bypassRec(r, inputActorNames, chList, actorList);
      // remove bypassed channels
      for (auto& i : inputActorNames) {
        string channelToRemove = channelNameFromActors(i, r, chList);
        actorList.at(i).removePort(chList.at(channelToRemove).getSrcPort());
        chList.erase(chList.find(channelToRemove));
        updateBinopArguments(r, i, binopActors, actorList, chList);
      }
      // remove recursive actor
      actorList.erase(actorList.find(r));
    }
    // Modify delay actors representation for SDF
    for (auto& d : delayActors) {
        // remove bypassed channel
        string ch1 = channelNameFromActors(actorList.at(d).getDelayInputSigName(),
                                           d, chList);
        bypassDelay(d, actorList.at(d).getDelayInputSigName(), chList, actorList);
        actorList.at(actorList.at(d).getDelayInputSigName()).removePort(chList.at(ch1).getSrcPort());
        chList.erase(chList.find(ch1));
        // remove delay actor and argument channel
        string argActorName = actorList.at(d).getArg().first;
        string rmChannel = channelNameFromActors(argActorName, d, chList);
        cout << "\tRemoving channel: " << rmChannel << endl;
        cout << "\t\tRemoving port: " << chList.at(rmChannel).getSrcPort() << endl;
        actorList.at(argActorName).removePort(chList.at(rmChannel).getSrcPort());
        chList.erase(chList.find(rmChannel));
        cout << "\t\tNumber of ports left for " << argActorName << ": "
             << actorList.at(argActorName).getPorts().size() << endl;
        updateBinopArguments(d, actorList.at(d).getDelayInputSigName(),
                             binopActors, actorList, chList);
        actorList.erase(actorList.find(d));
        // Remove argument actor if it's found to be purely for delay
        if (actorList.at(argActorName).getPorts().size() == 0) {
            actorList.erase(actorList.find(argActorName));
        }
    }
    // update names of binop actors to reflect order of input arguments
    for (auto& b : binopActors) {
      string newName = actorList.at(b).getName();
      cout << "Updated order of args for " << b << ":" << endl;
      for (auto& arg : actorList.at(b).getInputSignalNames()) {
        newName += "_" + arg;
        cout << "\t " << arg << endl;
      }
      actorList.at(b).setName(newName);
    }
    // Write graph information (actor/channel names, ports)
    for (auto& a : actorList) {
        // add self loops
        string srcPortName("in_R" + a.first);
        string dstPortName("out_R" + a.first);
        a.second.addPort(Port(srcPortName,
                              portType::out,
                              1));
        a.second.addPort(Port(dstPortName,
                              portType::in,
                              1));
        string chName("channel_" + a.first);
        chList.insert(pair<string, Channel>(chName,
                                            Channel(chName,
                                                    a.first, srcPortName,
                                                    a.first, dstPortName,
                                                    1, 1)));
        a.second.writeToXML(fout);
    }
    for (auto& c : chList) {
        c.second.writeToXML(fout);
    }
    fout << "    </sdf>\n" << endl;
    fout << "    <sdfProperties>" << endl;
    // Write actor properties
    for (auto& a : actorList) {
        a.second.writePropertiesToXML(fout);
    }
    fout << "    </sdfProperties>" << endl;
    fout << "</applicationGraph>" << endl;
    fout << "</sdf3>" << endl;
}

/**
 * Recursively traverse signal and log actors, channels, and ports
 */
static void recLog(Tree sig, set<Tree>& drawn, map<string, Actor>& actorList,
                   map<string, Channel>& chList, int& chCount,
                   vector<string>& delayActors, vector<string>& recActors,
                   vector<string>& binopActors)
{
    // cerr << ++gGlobal->TABBER << "ENTER REC DRAW OF " << sig << "$" << *sig << endl;
    vector<Tree> subsig;
    int          n;

    if (drawn.count(sig) == 0) {
        drawn.insert(sig);
        if (isList(sig)) {
            do {
                recLog(hd(sig), drawn, actorList, chList, chCount,
                       delayActors, recActors, binopActors);
                sig = tl(sig);
            } while (isList(sig));
        } else {
            // Add actor to list of actors
            stringstream actorName; // workaround to get unique actor names from signal
            actorName << sig;
            actorList.insert(pair<string, Actor>(actorName.str(),
                                                 Actor(actorName.str(), sigLabel(sig))));
            Tree arg1, arg2;
            int arg2_val;
            if (isSigFixDelay(sig, arg1, arg2)) {
                stringstream arg1_name;
                stringstream arg2_name;
                arg1_name << arg1;
                arg2_name << arg2;
                delayActors.push_back(actorName.str());
                if (isSigInt(arg2, &arg2_val)) { // assigns int value to arg2_val
                }
                actorList.at(actorName.str()).setDelayInputSigName(arg1_name.str());
                actorList.at(actorName.str()).setArg(arg2_name.str(), arg2_val);
            } else if (isSigBinOp(sig, &arg2_val, arg1, arg2)) {
              stringstream arg1Name;
              stringstream arg2Name;
              arg1Name << arg1;
              arg2Name << arg2;
              cout << actorName.str() << " order of args:\n"
                   << "\t1. " << arg1Name.str() << "\n"
                   << "\t2. " << arg2Name.str() << endl;
              // track order of arguments for binary operators
              binopActors.push_back(actorName.str());
              actorList.at(actorName.str()).addInputSignalName(arg1Name.str());
              actorList.at(actorName.str()).addInputSignalName(arg2Name.str());
            }

            // draw the subsignals
            n = getSubSignals(sig, subsig);
            if (n > 0) {
                if (n == 1 && isList(subsig[0])) {
                    Tree id, body;
                    faustassert(isRec(sig, id, body));
                    if (isRec(sig, id, body)) {
                      recActors.push_back(actorName.str());
                      for (auto& b : body->branches()) {
                        if (b->node() == "cons") { // NOTE when one input of REC WN is delay, the other is a node "cons" --- need to check branches of cons to retrieve name of input actor
                          for (auto& next : b->branches()) {
                            // NOTE found that each input signal of a REC WN
                            // operator is accompanied by a node with the following
                            // properties: arity = 0, serial = 35, node = nil
                            // cout << "\tnode: " << next->node() << endl;
                            // don't add "nil" nodes
                            if (next->arity() != 0 && next->serial() != 35 && next->node() != "nil") {
                              stringstream sigName;
                              sigName << next;
                              // cout << "\t\tAdding " << next->node() << "(" << sigName.str() << ") as input signal of " << actorName.str() << endl;
                              actorList.at(actorName.str()).addInputSignalName(sigName.str());
                            } else if (next->node() == "cons") { // TODO implement this recursively to support nested REC WN
                              cout << "triple-nested recursion loops not currently supported: error in output" << endl;
                            }
                          }
                        } else {
                          // only add input signals that aren't 'nil' signals
                          if (b->arity() != 0 && b->serial() != 35 && b->node() != "nil") {
                            stringstream sigName;
                            sigName << b;
                            // cout << "Adding " << sigName.str() << " as input signal of " << actorName.str() << endl;
                            // cout << "\tnode: " << b->node() << endl;
                            actorList.at(actorName.str()).addInputSignalName(sigName.str());
                          }
                        }
                      }
                    } else {
                    }
                    // special recursion case, recreate a vector of subsignals instead of the
                    // list provided by getSubSignal
                    Tree L = subsig[0];
                    subsig.clear();
                    n = 0;
                    do {
                        subsig.push_back(hd(L));
                        L = tl(L);
                        n += 1;
                    } while (isList(L));
                }

                for (int i = 0; i < n; i++) {
                    recLog(subsig[i], drawn, actorList, chList, chCount,
                           delayActors, recActors, binopActors);
                    // log channels and corresponding ports for the connected actors
                    string chName("channel_" + to_string(chCount) + chAttr(getCertifiedSigType(subsig[i])));
                    stringstream srcActor;
                    stringstream dstActor;
                    srcActor << subsig[i];
                    dstActor << sig;
                    string srcPortName("in_" + chName);
                    string dstPortName("out_" + chName);
                    actorList.at(srcActor.str()).addPort(Port(srcPortName,
                                                              portType::out,
                                                              1));
                    actorList.at(dstActor.str()).addPort(Port(dstPortName,
                                                              portType::in,
                                                              1));
                    chList.insert(pair<string, Channel>(chName,
                                                        Channel(chName,
                                                                srcActor.str(), srcPortName,
                                                                dstActor.str(), dstPortName,
                                                                1, 0)));
                    chCount++;
                }
            }
        }
    }
    // cerr << --gGlobal->TABBER << "EXIT REC DRAW OF " << sig << endl;
}

/**
 * Return string of signal type
 */
static string chAttr(Type t)
{
    string s;

    // nature
    switch (t->nature()) {
    case kInt:
        s += "_int";
        break;
    case kReal:
        s += "_real";
        break;
    default:
        s+= "_nomatch";
        break;
    }

    // vectorability
    if (t->vectorability() == kVect && t->variability() == kSamp) {
        s += "_vect";
    }

    return s;
}

/**
 * translate signal binary operations into strings
 */
static const char* binopname[] = {"add", "diff", "prod", "div", "mod", "l_shift", "r_shift", "greaterthan", "lessthan", "geq", "leq", "equal", "notequal", "AND", "OR", "XOR"};

/**
 * return the label of a signal as a string
 */
static string sigLabel(Tree sig)
{
    int    i;
    double r;
    Tree   x, y, z, c, type, name, file, ff, largs, id, le, sel, var, label;

    xtended* p = (xtended*)getUserData(sig);

    stringstream fout;

    if (p) {
        fout << p->name();
    } else if (isSigInt(sig, &i)) {
        fout << i;
    } else if (isSigReal(sig, &r)) {
        fout << r;
    } else if (isSigWaveform(sig)) {
        fout << "waveform";
    }

    else if (isSigInput(sig, &i)) {
        fout << "INPUT_" << i;
    }
    // else if ( isSigOutput(sig, &i, x) )             { fout << "OUTPUT_" << i; }

    else if (isSigDelay1(sig, x)) {
        fout << "mem";
    } else if (isSigFixDelay(sig, x, y)) {
        fout << "delay";
    } else if (isSigPrefix(sig, x, y)) {
        fout << "prefix";
    } else if (isSigIota(sig, x)) {
        fout << "iota";
    } else if (isSigBinOp(sig, &i, x, y)) {
        fout << binopname[i];
    } else if (isSigFFun(sig, ff, largs)) {
        fout << "ffunction:" << *ff;
    } else if (isSigFConst(sig, type, name, file)) {
        fout << *name;
    } else if (isSigFVar(sig, type, name, file)) {
        fout << *name;
    }

    else if (isSigTable(sig, id, x, y)) {
        fout << "table:" << id;
    } else if (isSigWRTbl(sig, id, x, y, z)) {
        fout << "write:" << id;
    } else if (isSigRDTbl(sig, x, y)) {
        fout << "read";
    }

    else if (isSigSelect2(sig, sel, x, y)) {
        fout << "select2";
    } else if (isSigSelect3(sig, sel, x, y, z)) {
        fout << "select3";
    }

    else if (isSigGen(sig, x)) {
        fout << "generator";
    }

    else if (isProj(sig, &i, x)) {
        fout << "Proj";
    } else if (isRec(sig, var, le)) {
        fout << "REC " << *var;
    }

    else if (isSigIntCast(sig, x)) {
        fout << "int";
    } else if (isSigFloatCast(sig, x)) {
        fout << "float";
    }
#if 0
    else if ( isSigButton(sig, label) )                         { fout << "button \"" << *label << '"'; }
    else if ( isSigCheckbox(sig, label) )                       { fout << "checkbox \"" << *label << '"'; }
    else if ( isSigVSlider(sig, label,c,x,y,z) )	{ fout << "vslider \"" << *label << '"';  }
    else if ( isSigHSlider(sig, label,c,x,y,z) )	{ fout << "hslider \"" << *label << '"';  }
    else if ( isSigNumEntry(sig, label,c,x,y,z) )	{ fout << "nentry \"" << *label << '"';  }

    else if ( isSigVBargraph(sig, label,x,y,z) )	{ fout << "vbargraph \"" << *label << '"';      }
    else if ( isSigHBargraph(sig, label,x,y,z) )	{ fout << "hbargraph \"" << *label << '"';      }
#else
    else if (isSigButton(sig, label)) {
        fout << "button";
    } else if (isSigCheckbox(sig, label)) {
        fout << "checkbox";
    } else if (isSigVSlider(sig, label, c, x, y, z)) {
        fout << "vslider";
    } else if (isSigHSlider(sig, label, c, x, y, z)) {
      fout << "hslider" << "_" << *c;
    } else if (isSigNumEntry(sig, label, c, x, y, z)) {
        fout << "nentry";
    }

    else if (isSigVBargraph(sig, label, x, y, z)) {
        fout << "vbargraph";
    } else if (isSigHBargraph(sig, label, x, y, z)) {
        fout << "hbargraph";
    }
#endif
    else if (isSigAttach(sig, x, y)) {
        fout << "attach";
    }

    else {
        stringstream error;
        error << "ERROR : unrecognized signal : " << *sig << endl;
        throw faustexception(error.str());
    }

    return fout.str();
}

// combine two channels in channel list
void mergeChannels(string ch1, string ch2, map<string, Channel>& chList)
{
    chList.at(ch1).setDstActor(chList.at(ch2).getDstActor());
    chList.at(ch1).setDstPort(chList.at(ch2).getDstPort());
    // retain ch1 in list
    chList.erase(chList.find(ch2));
}

// modify a channel to bypass the given delay actor
void bypassDelay(string delayActorName, string inputActorName,
                 map<string, Channel>& chList, map<string, Actor>& actorList)
{
    int delayArg = actorList.at(delayActorName).getArg().second;
    // connect the input actor to the destination of the delay signal
    for (auto& p : actorList.at(delayActorName).getPorts()) {
        if (p.getType() == "out") {
            string channelToMod = channelNameFromPort(p, chList);
            actorList.at(inputActorName).addPort(p);
            chList.at(channelToMod).setSrcActor(inputActorName);
            chList.at(channelToMod).setInitialTokens(delayArg);
        }
    }
}

// modify a channel to bypass the given REC actor
void bypassRec(string recActorName, vector<string> inputSignalNames,
               map<string, Channel>& chList, map<string, Actor>& actorList) {
  vector<Port> outputPorts;
  for (auto& p : actorList.at(recActorName).getPorts()) {
    if (p.getType() == "out") {
      outputPorts.push_back(p);
    }
  }
  assert(outputPorts.size() == inputSignalNames.size()); // rec signals must have matching input and output signal numbers
  // randomly assign inputs to outputs TODO figure out actual mapping of this
  for (size_t i = 0; i < outputPorts.size(); i++) {
    string channelToMod = channelNameFromPort(outputPorts[i], chList);
    actorList.at(inputSignalNames[i]).addPort(outputPorts[i]);
    chList.at(channelToMod).setSrcActor(inputSignalNames[i]); // connect output channel of REC to one of its input actors
  }
}

// identify channel name based on an input or output port
string channelNameFromPort(Port port, map<string, Channel>& chList)
{
    for (auto& c : chList) {
        if (port.getType() == "in") {
            if (port.getName() == c.second.getDstPort()) {
                return c.second.getName();
            }
        } else if (port.getType() == "out") {
            if (port.getName() == c.second.getSrcPort()) {
                return c.second.getName();
            }
        }
    }
    return "ERROR: no matching channel";
}

// identify name of channel between two actors
string channelNameFromActors(string srcActor, string dstActor, map<string, Channel>& chList)
{
    for (auto& c : chList) {
        if (c.second.getSrcActor() == srcActor && c.second.getDstActor() == dstActor) {
            return c.second.getName();
        }
    }
    return "ERROR no matching channel";
}

// update argument actor names of binary operators if they have changed
void updateBinopArguments(string oldArg, string newArg, vector<string>& binopList,
                          map<string, Actor>& actorList, map<string, Channel>& chList) {
  for (auto& op : binopList) {
    vector<string> argNames = (actorList.at(op)).getInputSignalNames();
    for (auto& arg : argNames) {
      if (oldArg == arg) {
        (actorList.at(op)).replaceInputSignalName(oldArg, newArg);
      }
    }
  }
}
