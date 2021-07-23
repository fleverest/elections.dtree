#include "ballot.h"
#include "dirichlet-tree.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string.h>
#include <string>
#include <unistd.h>

int getNCandidates(std::string s) {
  int out = 0;

  for (char c : s)
    if (c == ',')
      ++out;

  return out;
}

BallotCount *getBC(std::string s, int nCandidates) {
  BallotCount *bc = new BallotCount;
  int *ballotPermutation = new int[nCandidates];
  int i = 0;
  std::ostringstream next;

  for (char c : s) {
    if (c == ',') {
      ballotPermutation[i] = stoi(next.str());
      ++i;
      next.str("");
    } else {
      next << c;
    }
  }

  bc->count = stoi(next.str());
  bc->ballotPermutation = ballotPermutation;

  return bc;
}

int main(int argc, char **argv) {

  std::ifstream in;
  std::string line;
  BallotCount *bc;
  int c;
  int **bPerms;
  int nBallots = 0;
  election e = *new election{};

  while ((c = getopt(argc, argv, "i:")) != -1) {
    switch (c) {
    case 'i':
      if (optarg)
        in.open(optarg, std::ifstream::in);
      break;
    }
  }

  if (!in.is_open()) {
    std::cout << "-i parameter must be a valid file location." << std::endl;
    return 0;
  }

  // Get number of candidates from header
  std::getline(in, line);
  c = getNCandidates(line);

  // Assemble an array of ballot permutations and shuffle.
  std::getline(in, line);
  while (in.good()) {
    bc = getBC(line, c);
    nBallots += bc->count;
    e.push_back(*bc);
    delete bc;
    std::getline(in, line);
  }

  std::cout << "Candidate " << evaluateElection(e, c) << " wins." << std::endl;

  return 1;
}
