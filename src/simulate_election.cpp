#include "ballot.h"
#include "dirichlet-tree.h"
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <vector>

int main(int argc, char **argv) {

  std::string seed = "default seed";
  float scale = 1.;
  int nCandidates = 0;
  int nBallots = 0;
  bool treeType = TREE_TYPE_DIRICHLET_TREE;

  int c;

  while ((c = getopt(argc, argv, "s:t:k:n:m:")) != -1) {
    switch (c) {
    case 's':
      if (optarg)
        seed = optarg;
      break;
    case 't':
      if (strcmp("dirichlet", optarg) == 0) {
        treeType = TREE_TYPE_VANILLA_DIRICHLET;
        break;
      } else if (strcmp("dirichlettree", optarg) == 0) {
        treeType = TREE_TYPE_DIRICHLET_TREE;
        break;
      } else {
        std::cout << "-t (TreeType) parameter must be one of `dirichlet` or "
                     "`dirichlettree`."
                  << std::endl;
        return 1;
      }
    case 'k':
      try {
        if (optarg)
          scale = atof(optarg);
        break;
      } catch (int n) {
        std::cout << "-k (Scale) parameter must be a floating point number."
                  << std::endl;
        return 1;
      }
    case 'n':
      try {
        if (optarg)
          nCandidates = atoi(optarg);
        if (nCandidates < 2)
          throw 0;
        break;
      } catch (int n) {
        std::cout << "-n (Number of Candidates) parameter must be an integer "
                     ">= 2."
                  << std::endl;
        return 1;
      }
    case 'm':
      try {
        if (optarg)
          nBallots = atoi(optarg);
      } catch (int n) {
        std::cout
            << "-m (Number of Ballots) parameter must me a positive integer."
            << std::endl;
      }
    }
  }

  if (nCandidates < 2) {
    std::cout << "-n (Number of Candidates) parameter must be an integer "
                 ">= 2."
              << std::endl;
    return 1;
  }
  if (nBallots <= 1) {
    std::cout << "-m (Number of Ballots) parameter must me a positive integer."
              << std::endl;
  }

  election *out;

  // Create a fresh Dirichlet Tree with IRV ballot leaves
  DirichletTreeIRV *dtree =
      new DirichletTreeIRV(nCandidates, scale, treeType, seed);

  // Sample a random election.
  out = dtree->sample(1, nBallots);

  // Output to stdout
  std::cout << electionToStr(out[0], nCandidates);

  return 0;
}
