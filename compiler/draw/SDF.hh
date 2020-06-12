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
    portType getType();
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
    string getName();
    string getType();
    void writeToXML(ofstream& fout);
    void printInfo(); // for debugging
    
private:
    string name; // unique identifier for actor
    string type; // describes what the actor does
    vector<Port> ports;
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
    string getName();
    string getSrcActor();
    string getDstActor();
    string getSrcPort();
    string getDstPort();
    int getSize();
    int getInitialTokens();
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
