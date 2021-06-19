#include "distributions.h"
#include <iostream>

int main() {

  std::cout << "Starting" << std::endl;
  std::string s = "900";
  std::seed_seq seed(s.begin(), s.end());
  std::mt19937 e(seed);
  int reps = 10;
  int n = 100;
  int dim = 100;
  int *draws = new int[dim];
  for (int i = 0; i < n; ++i) {
    draws[i] = 10000000;
  }
  float *alphas = new float[dim];
  for (int i = 0; i < n; ++i) {
    alphas[i] = 1000.;
  }

  int **results = rDirichletMultinomial(reps, draws, alphas, dim, &e);

  std::cout << "Results:" << std::endl;
  for (int i = 0; i < reps; ++i) {
    std::cout << i << ": " << std::endl;
    for (int j = 0; j < dim; ++j) {
      std::cout << results[i][j] << " ";
    }
    std::cout << std::endl;
  }

  return 1;
}
