#include "ballot.h"
#include "dirichlet-tree.h"
#include <iostream>
#include <vector>

void hc(election s, int nCandidates) {
  int total = 0;
  std::cout << "Distinct ballots: " << s.size() << std::endl;
  std::cout << "Highest count: ";
  int maxi = -1;
  int max = 0;
  for (unsigned long int i = 0; i < s.size(); ++i) {
    total += s[i].count;
    if (s[i].count > max) {
      max = s[i].count;
      maxi = i;
    }
  }
  if (maxi == -1) {
    std::cout << "NA";
  } else {
    for (int i = 0; i < nCandidates; ++i) {
      std::cout << s[maxi].ballotPermutation[i] << " ";
    }
    std::cout << "with count " << max;
  }
  std::cout << std::endl;
  std::cout << "Total count " << total;
  std::cout << std::endl;
}

int main() {

  std::cout << "Starting" << std::endl;
  std::string seed = "08971";
  int nElections = 2;
  election *out;
  int nBallots1 = 100000000;
  int nBallots2 = 100000000;
  int nCandidates = 10;
  float scale = 1.;
  bool treeType = TREE_TYPE_DIRICHLET_TREE;

  // std::cout << "Initializing Tree" << std::endl;
  DirichletTreeIRV *dtree =
      new DirichletTreeIRV(nCandidates, scale, treeType, seed);

  // std::cout << "Sampling" << std::endl;

  out = dtree->sample(nElections, nBallots1);

  // std::cout << "---- Initial ballot stats: ----" << std::endl;
  hc(out[0], nCandidates);

  // std::cout << "-------------------------------" << std::endl;
  hc(out[1], nCandidates);

  // std::cout << "hits: " << hits << ", samples: " << timesCalled << std::endl;

  //  std::cout << "Updating posterior with first batch" << std::endl;
  //  for (BallotCount bc : out[0]) {
  //    dtree->update(bc);
  //  }
  //
  //  std::cout << "Sampling second batch" << std::endl;
  //  out = dtree->sample(nElections, nBallots2);
  //
  //  std::cout << "----- Second ballot stats -----" << std::endl;
  //  hc(out[0], nCandidates);
  //  std::cout << "-------------------------------" << std::endl;
  //  hc(out[1], nCandidates);
  //
  return 1;
}
