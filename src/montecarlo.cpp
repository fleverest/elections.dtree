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

  int *posteriorProbs;
  std::ifstream in;
  std::ofstream out;
  std::string line;
  int c;
  int nElections = 1;
  int nBatch = 1;
  int nBallots = 0;
  int counted, count;
  BallotCount *bc;
  election e = {};
  int **bPerms, **outdata;
  int steps = 9;
  std::string seed = "defaultseed";
  bool treeType = TREE_TYPE_DIRICHLET_TREE;
  float scale = 1.;

  while ((c = getopt(argc, argv, "i:b:e:k:o:c:s:t:")) != -1) {
    switch (c) {
    case 'e':
      try {
        if (optarg)
          nElections = atoi(optarg);
        break;
      } catch (int n) {
        std::cout << "-e (number of elections per batch) parameter must be an "
                     "integer >= 1."
                  << std::endl;
        return 1;
      }
    case 'b':
      try {
        if (optarg)
          nBatch = atoi(optarg);
        break;
      } catch (int n) {
        std::cout << "-b (number of batches) parameter must be an integer >= 1."
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
    case 't':
      if (strcmp("dirichlet", optarg) == 0) {
        break;
      } else if (strcmp("dirichlettree", optarg) == 0) {
        treeType = TREE_TYPE_VANILLA_DIRICHLET;
        break;
      } else {
        std::cout << "-t (TreeType) parameter must be one of `dirichlet` or "
                     "`dirichlettree`."
                  << std::endl;
        return 1;
      }
    case 'c':
      if (optarg)
        seed = optarg;
      break;
    case 's':
      try {
        if (optarg)
          steps = atoi(optarg);
        break;
      } catch (int n) {
        std::cout << "-s (number of steps) parameter must be an integer >= 1."
                  << std::endl;
        return 1;
      }
      break;
    case 'i':
      if (optarg)
        in.open(optarg, std::ifstream::in);
      break;
    case 'o':
      if (optarg)
        out.open(optarg, std::ofstream::out);
      break;
    }
  }

  if (!in.is_open()) {
    std::cout << "-i parameter must be a valid file location." << std::endl;
  } else if (!out.is_open()) {
    std::cout << "-o parameter must be a valid file location." << std::endl;
  }

  // Get number of candidates from header
  std::getline(in, line);
  c = getNCandidates(line);

  DirichletTreeIRV dtree(c, scale, treeType, seed);

  // Assemble an array of ballot permutations and shuffle.
  std::getline(in, line);
  while (in.good()) {
    bc = getBC(line, c);
    nBallots += bc->count;
    e.push_back(*bc);
    delete bc;
    std::getline(in, line);
  }
  bPerms = new int *[nBallots];
  int i = 0;
  for (BallotCount bc : e) {
    for (int j = i; j < i + bc.count; ++j) {
      bPerms[j] = bc.ballotPermutation;
    }
    i += bc.count;
  }
  std::shuffle(bPerms, bPerms + nBallots, *dtree.getEnginePtr());

  // Prepare output file.
  out << "counted,";
  for (int i = 0; i < c; ++i) {
    out << "P(" << i + 1 << " wins)";
    if (i < c - 1)
      out << ",";
  }
  out << "\n";

  // Proceed to audit (in batches).
  outdata = new int *[steps];
  for (int i = 0; i < steps; ++i) {
    outdata[i] = new int[c + 1];
  }
  for (int b = 0; b < nBatch; ++b) {
    std::cout << "Batch " << b << std::endl;
    dtree.reset();
    count = nBallots / (steps + 1);
    counted = 0;
    for (int s = 0; s < steps; ++s) {
      for (int i = counted; i < counted + count; ++i) {
        bc = new BallotCount;
        bc->count = 1;
        bc->ballotPermutation = bPerms[i];
        dtree.update(*bc);
        delete bc;
      }
      counted += count;
      // Get posterior winning probabilities for each candidate.
      posteriorProbs = dtree.samplePosterior(nElections, nBallots - counted);
      outdata[s][0] = counted;
      for (int i = 0; i < c; ++i) {
        outdata[s][i + 1] += posteriorProbs[i];
      }
      delete[] posteriorProbs;
    }
  }

  std::cout << "Done" << std::endl;
  for (int i = 0; i < steps; ++i) {
    for (int j = 0; j < c + 1; ++j) {
      out << outdata[i][j];
      if (j < c)
        out << ',';
    }
    delete[] outdata[i];
    out << "\n";
  }
  delete[] outdata;
  for (BallotCount bc : e) {
    delete[] bc.ballotPermutation;
  }
  delete[] bPerms;

  return 0;
}
