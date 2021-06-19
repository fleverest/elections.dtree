#include "ballot.h"
#include "dirichlet-tree.h"
#include <cstdlib>
#include <iostream>
#include <vector>

int main(int argc, char **argv) {

  std::string seed = argv[1];
  float scale = atof(argv[3]);
  int nCandidates = atoi(argv[4]);
  int nBallots = atoi(argv[5]);

  bool treeType = TREE_TYPE_DIRICHLET_TREE;

  election *out;

  DirichletTreeIRV *dtree =
      new DirichletTreeIRV(nCandidates, scale, treeType, seed);

  out = dtree->sample(1, nBallots);
}
