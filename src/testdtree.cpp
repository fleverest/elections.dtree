#include "ballot.h"
#include "dirichlet-tree.h"
#include <cstdlib>
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

int main(int argc, char **argv) {

  std::string seed = argv[1];
  int nElections = atof(argv[2]);
  float scale = atof(argv[3]);
  int nCandidates = atoi(argv[4]);
  int nBallots = atoi(argv[5]);

  election *out;

  DirichletTreeIRV *dtree =
      new DirichletTreeIRV(nCandidates, scale, TREE_TYPE_DIRICHLET_TREE, seed);

  out = dtree->sample(nElections, nBallots);

  for (int i = 0; i < nElections; ++i) {
    hc(out[i], nCandidates);
  }

  std::cout << "Updating posterior with first batch" << std::endl;
  for (BallotCount bc : out[0]) {
    dtree->update(bc);
  }

  std::cout << "Sampling second batch" << std::endl;
  out = dtree->sample(nElections, nBallots);

  for (int i = 0; i < nElections; ++i) {
    hc(out[i], nCandidates);
  }

  return 0;
}
