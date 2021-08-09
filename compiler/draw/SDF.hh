#include <fstream>
#include <iostream>
#include <string>
#include "signals.hh"
#include <set>

enum portType {in, out};

class Port {
public:
    Port(string, portType, int);
    void setName(string);
    void setType(portType);
    void setRate(int);
    string getName();
    string getType();
    int getRate();

private:
    string name;
    portType type;
    int execRate;
};

class Actor {
public:
    Actor(string, string);
    void setName(string);
    void setType(string);
    void addPort(Port);
    void removePort(string);
    void setDelayInputSigName(string);
    void setArg(string, int);
    void addInputSignalName(string);
    string getName();
    string getType();
    vector<Port> getPorts();
    string getDelayInputSigName();
    pair<string, int> getArg();
    vector<string> getInputSignalNames();
    void writeToXML(ofstream& fout);
    void writePropertiesToXML(ofstream& fout);
    void printInfo(); // for debugging

private:
    string name; // unique identifier for actor
    string type; // describes what the actor does
    vector<Port> ports;
    string delayInputSigName; // track input signals for delay operator in order to bypass it in SDF representation
    pair<string, int> args;
    vector<string> inputSignals; // track list of input signals for rec operator in order to bypass it in SDF representation
};

class Channel {
public:
    Channel(string name,
            string srcActor, string srcPort,
            string dstActor, string dstPort,
            int size, int initialTokens);
    Channel(string name,
            string srcActor, string srcPort,
            string dstActor, string dstPort);
    void setSrcActor(string);
    void setDstActor(string);
    void setSrcPort(string);
    void setDstPort(string);
    string getName();
    string getSrcActor();
    string getDstActor();
    string getSrcPort();
    string getDstPort();
    int getSize();
    int getInitialTokens();
    void setInitialTokens(int);
    void writeToXML(ofstream& fout);
    void printInfo();
private:
    string name;
    string srcActor;
    string dstActor;
    string srcPort;
    string dstPort;
    int size;
    int initialTokens;
};
