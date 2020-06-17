#include <fstream>
#include <iostream>
#include <string>
#include "signals.hh"
#include "SDF.hh"

Port::Port(string name, portType type, int rate)
  :name{name}, type{type}, execRate{rate} {}

void Port::setName(string name)
{
    this->name = name;
}

void Port::setType(portType type)
{
    this->type = type;
}

void Port::setRate(int rate)
{
    this->execRate = rate;
}

string Port::getName()
{
    return this->name;
}

string Port::getType()
{
    if (this->type == portType::in)
        return "in";
    else if (this->type == portType::out)
        return "out";
    else
        return "ERROR: invalid port type detected";
}

int Port::getRate()
{
    return this->execRate;
}

Actor::Actor(string name, string type) :name{name}, type{type}
{
    // this->ports = nullptr;
}

void Actor::setName(string name)
{
    this->name = name;
}

void Actor::setType(string type)
{
    this->type = type;
}

void Actor::setInputSigName(string inputSig)
{
    this->inputSigName = inputSig;
}

void Actor::setArg(string argActorName, int value)
{
    this->args.first = argActorName;
    this->args.second = value;
}

void Actor::addPort(Port newPort)
{
    this->ports.push_back(newPort);
}

string Actor::getName()
{
    return this->name;
}

string Actor::getType()
{
    return this->type;
}

string Actor::getInputSigName()
{
    return this->inputSigName;
}

pair<string, int> Actor::getArg()
{
    return this->args;
}

void Actor::writeToXML(ofstream& fout)
{
    fout << "        <actor name='" << this->getName()
         << "' type='" << this->getType() << "'>" << endl;
    for (auto p : this->ports) {
        fout << "            <port type='" << p.getType()
             << "' name='" << p.getName()
             << "' rate='" << p.getRate()
             << "'/>" << endl;
    }
    fout << "        </actor>" << endl;
}

void Actor::writePropertiesToXML(ofstream& fout)
{
    fout << "        <actorProperties actor='" << this->getName() << "'>" << endl;
    fout << "            <processor type='cluster_0' default='true'>" << endl;
    fout << "                <executionTime time='1' />" << endl;
    fout << "            </processor>" << endl;
    fout << "        </actorProperties>" << endl;
}

void Actor::printInfo()
{
    std::cout << "Actor name, type: " << this->getName() << ", " << this->getType() << std::endl;
    for (auto i : this->ports) {
        std::cout << "\tPort: " << i.getName() << std::endl;
    }
}

Channel::Channel(string name, string srcActor, string srcPort,
                 string dstActor, string dstPort,
                 int size, int initialTokens)
    : name{name},
      srcActor{srcActor},
      dstActor{dstActor},
      srcPort{srcPort},
      dstPort{dstPort},
      size{size},
      initialTokens{initialTokens}
{
}

Channel::Channel(string name, string srcActor, string srcPort,
                 string dstActor, string dstPort)
    : name{name},
      srcActor{srcActor},
      dstActor{dstActor},
      srcPort{srcPort},
      dstPort{dstPort},
      size{1},
      initialTokens{0}
{
}

string Channel::getName()
{
    return this->name;
}

string Channel::getSrcActor()
{
    return this->srcActor;
}

string Channel::getDstActor()
{
    return this->dstActor;
}

string Channel::getSrcPort()
{
    return this->srcPort;
}

string Channel::getDstPort()
{
    return this->dstPort;
}

int Channel::getSize()
{
    return this->size;
}

int Channel::getInitialTokens()
{
    return this->initialTokens;
}

void Channel::setInitialTokens(int nTokens)
{
    this->initialTokens = nTokens;
}

void Channel::writeToXML(ofstream& fout)
{
    fout << "        <channel name='" << this->getName()
         << "' srcActor='" << this->getSrcActor()
         << "' srcPort='" << this->getSrcPort()
         << "' dstActor='" << this->getDstActor()
         << "' dstPort='" << this->getDstPort()
         << "' size='" << this->getSize()
         << "' initialTokens='" << this->getInitialTokens()
         << "'/>" << endl;
}

void Channel::printInfo()
{
    cout << "Channel name: " << this->getName() << endl;
    cout << "\tSource Port: " << this->getSrcPort() << endl;
    cout << "\tDest Port: " << this->getDstPort() << endl;
    cout << "\tSrcActor: " << this->getSrcActor() << endl;
    cout << "\tDstActor: " << this->getDstActor() << endl;
    
}
