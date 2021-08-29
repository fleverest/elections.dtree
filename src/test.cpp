#include "ballot.cpp"
#include "dirichlet-tree.cpp"
#include "distributions.cpp"
#include <chrono>
#include <iostream>
#include <thread>

int main(int argc, char **argv) {
  DirichletTreeIRV dtree(20, 10.0, TREE_TYPE_DIRICHLET_TREE, "seed");

  std::chrono::seconds dura(10);

  std::cout << "Starting sampling" << std::endl;

  election *e = dtree.sample(1, 10);

  std::cout << "Updating prior with sampled ballots" << std::endl;

  for (auto b : e[0]) {
    dtree.update(b);
  }

  std::cout << "Sampling posterior" << std::endl;

  int *post = dtree.samplePosterior(10, 10, false);

  delete[] post;
  delete[] e;

  std::cout << "Resetting tree" << std::endl;

  dtree.reset();

  _Exit(0);

  return 0;
}
