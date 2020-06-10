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

portType Port::getType()
{
    return this->type;
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

void Actor::printInfo()
{
    std::cout << "Actor name, type: " << this->getName() << ", " << this->getType() << std::endl;
    for (auto i : this->ports) {
        std::cout << i.getName() << std::endl;
    }
}
