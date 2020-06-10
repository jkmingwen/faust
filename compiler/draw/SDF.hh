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
    void printInfo(); // for debugging
    
private:
    string name; // unique identifier for actor
    string type; // describes what the actor does
    vector<Port> ports;
};
