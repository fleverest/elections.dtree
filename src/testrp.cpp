#include "dirichlet-tree.h"
#include <iostream>
#include <vector>

int main() {

  std::cout << "Starting" << std::endl;
  unsigned seed = 5;
  int nElections = 5;
  int nBallots = 10000;
  int nCandidates = 10;
  BallotCount bc = *(new BallotCount);
  bc.ballotIndices = new int[9]{0, 0, 0, 0, 0, 0, 0, 0, 0};
  bc.ballotPermutation = new int[9]{1, 2, 3, 4, 5, 6, 7, 8, 9};
  bc.count = 1;
  float scale = 1.;
  bool treeType = TREE_TYPE_VANILLA_DIRICHLET;

  std::cout << "Initializing Tree" << std::endl;
  DirichletTreeIRV *dtree =
      new DirichletTreeIRV(nCandidates, scale, treeType, seed);

  std::cout << "Updating" << std::endl;
  dtree->update(bc);

  std::cout << "Sampling" << std::endl;
  std::string *samples = dtree->sample(nElections, nBallots);

  for (int i = 0; i < nElections; ++i) {
    std::cout << "Election " << i + 1 << ":\n";
    std::cout << samples[i] << std::endl;
  }

  return 1;
}
