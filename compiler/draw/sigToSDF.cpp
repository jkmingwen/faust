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
    fout << "<?xml version=\"1.0\" encoding=\"UTF08\"?>\n"
         << "<sdf3 type=\"sdf\" version=\"1.0\"\n"
         << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
         << "xsi:noNamespaceSchemaLocation=\"http://www.es.ele.tue.nl/sdf3/xsd/sdf3-csdf.xsd\">"
         << endl;
    fout << "<applicationGraph name='test'>" << endl;
    fout << "<sdf name='test' type='test'>" << endl;
    // TODO actors port types (in/out), rates
    // TODO channels source and destination actor, initial tokens
    fout << "</sdf>" << endl;
    fout << "<sdfProperties>" << endl;
    // TODO actor properties (processor, execution time)
    fout << "</sdfProperties>" << endl;
    fout << "</applicationGraph>" << endl;
    fout << "</sdf3>" << endl;
}
