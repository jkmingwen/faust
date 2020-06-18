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
    while (isList(L)) {
        recLog(hd(L), alreadyDrawn, actorList, chList, chCount, delayActors);
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
    // for (auto& a : actorList) {
    //     a.second.printInfo();
    // }
    // for (auto& c : chList) {
    //     c.second.printInfo();
    // }

    // Write graph information to XML
    fout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         << "<sdf3 type=\"sdf\" version=\"1.0\"\n"
         << "    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
         << "    xsi:noNamespaceSchemaLocation=\"http://www.es.ele.tue.nl/sdf3/xsd/sdf3-csdf.xsd\">"
         << endl;
    fout << "<applicationGraph name='test'>" << endl;
    fout << "    <sdf name='test' type='test'>" << endl;
    // Write graph information (actor/channel names, ports)
    for (auto& d : delayActors) {
        string ch1 = channelNameFromActors(actorList.at(d).getInputSigName(),
                                           d, chList);
        // delay actors only have 1 output
        string ch2;
        for (auto p : actorList.at(d).getPorts()) {
            if (p.getType() == "out") {
                ch2 = channelNameFromPort(p, chList);
            }
        }
        mergeChannels(ch1, ch2, chList);
        chList.at(ch1).setInitialTokens(actorList.at(d).getArg().second);
    }
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
                   vector<string>& delayActors)
{
    // cerr << ++gGlobal->TABBER << "ENTER REC DRAW OF " << sig << "$" << *sig << endl;
    vector<Tree> subsig;
    int          n;

    if (drawn.count(sig) == 0) {
        drawn.insert(sig);
        if (isList(sig)) {
            do {
                recLog(hd(sig), drawn, actorList, chList, chCount, delayActors);
                sig = tl(sig);
            } while (isList(sig));
        } else {
            // Add actor to list of actors
            stringstream actorName; // workaround to get unique actor names from signal
            actorName << sig;
            actorList.insert(pair<string, Actor>(actorName.str(),
                                                 Actor(actorName.str(), sigLabel(sig))));
            // TODO if sigLabel(sig) == "delay", then save name and don't add to actorList
            Tree arg1, arg2;
            int arg2_val;
            if (isSigFixDelay(sig, arg1, arg2)) {
                stringstream arg1_name;
                stringstream arg2_name;
                arg1_name << arg1;
                arg2_name << arg2;
                delayActors.push_back(actorName.str());
                if (isSigInt(arg2, &arg2_val)) { // assign int value
                }
                actorList.at(actorName.str()).setInputSigName(arg1_name.str());
                actorList.at(actorName.str()).setArg(arg2_name.str(), arg2_val);
            }

            // draw the subsignals
            n = getSubSignals(sig, subsig);
            if (n > 0) {
                if (n == 1 && isList(subsig[0])) {
                    Tree id, body;
                    faustassert(isRec(sig, id, body));
                    if (!isRec(sig, id, body)) {
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
                    recLog(subsig[i], drawn, actorList, chList, chCount, delayActors);
                    // log channels and corresponding ports for the connected actors
                    string chName("channel_" + to_string(chCount) + chAttr(getCertifiedSigType(subsig[i])));
                    stringstream srcActor;
                    stringstream dstActor;
                    srcActor << subsig[i];
                    dstActor << sig;
                    // TODO if sigLabel(sig) == "delay" then add srcPort to srcActor but not dstPort/dstActor
                    // TODO if sigLabel(subsig[i]) == "delay" then insert channel
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
                                                                1, 0))); // don't really know the right settings for this yet
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
        fout << "Proj" << i;
    } else if (isRec(sig, var, le)) {
        fout << "REC " << *var;
    }

    else if (isSigIntCast(sig, x)) {
        fout << "int";
    } else if (isSigFloatCast(sig, x)) {
        fout << "float";
    }
#if 0
    else if ( isSigButton(sig, label) ) 			{ fout << "button \"" << *label << '"'; }
    else if ( isSigCheckbox(sig, label) ) 			{ fout << "checkbox \"" << *label << '"'; }
    else if ( isSigVSlider(sig, label,c,x,y,z) )	{ fout << "vslider \"" << *label << '"';  }
    else if ( isSigHSlider(sig, label,c,x,y,z) )	{ fout << "hslider \"" << *label << '"';  }
    else if ( isSigNumEntry(sig, label,c,x,y,z) )	{ fout << "nentry \"" << *label << '"';  }
    
    else if ( isSigVBargraph(sig, label,x,y,z) )	{ fout << "vbargraph \"" << *label << '"'; 	}
    else if ( isSigHBargraph(sig, label,x,y,z) )	{ fout << "hbargraph \"" << *label << '"'; 	}
#else
    else if (isSigButton(sig, label)) {
        fout << "button";
    } else if (isSigCheckbox(sig, label)) {
        fout << "checkbox";
    } else if (isSigVSlider(sig, label, c, x, y, z)) {
        fout << "vslider";
    } else if (isSigHSlider(sig, label, c, x, y, z)) {
        fout << "hslider";
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
