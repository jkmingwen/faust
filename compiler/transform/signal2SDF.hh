#include <cstdlib>
#include "property.hh"
#include "sigtyperules.hh"
#include "tree.hh"
#include "treeTraversal.hh"
#include "xtended.hh"
#include "SDF.hh"

//-------------------------Signal2SDF-------------------------------
// Compile signal expresssions into SDF representations in XML
//------------------------------------------------------------------

using namespace std;

class Signal2SDF : public TreeTraversal {
protected:
  bool fTraceFlag{false};  // trace transformations when true
  bool fVisitGen{false};
  int  fIndent{0};         // current indentation during trace
  string fMessage;         // trace message
  set<Tree> fVisited;      // avoid visiting a tree twice

  map<string, Actor> actorList;
  map<string, Channel> chList;
  int chCount = 0;
  int outCount = 0;
  vector<string> delayActors;
  vector<string> recActors;
  vector<string> binopActors;

  void visit(Tree t) override;

public:
  Signal2SDF() = default;
  void self(Tree t);
  void sigToSDF(Tree t, ofstream& fout);
  string chAttr(Type t);
  void mergeChannels(string ch1, string ch2);
  void bypassRec(string recActorName, vector<string> inputSignalNames);
  string channelNameFromPort(Port port);
  string channelNameFromActors(string srcActor, string dstActor);
  void updateBinopArguments(string oldArg, string newArg);
  void addChannel(Tree sig);
  void logActor(Tree sig, string type);
  void logDelayActor(Tree sig, Tree x, Tree y, string type);
  void logRecActor(Tree sig, Tree le, string type);
  void logBinopActor(Tree sig, Tree x, Tree y, string type);
  void logUIActor(Tree sig, Tree init);
  void logPowActor(Tree sig, Tree x, Tree y, string type);
  bool isSigPow(Tree sig, int* i, Tree &x, Tree &y);
};
