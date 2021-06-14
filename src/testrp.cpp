#include "dirichlet-tree.h"
#include <iostream>
#include <vector>

int main() {

    std::cout << "Starting" << std::endl;
    unsigned seed = 0;
    int nElections = 5;
    int nBallots = 50;
    int nCandidates = 5;
    int biasCount = 1;
    int biasBallot[] = {0,0,0,0};
    float scale = 1.;
    bool treeType = TREE_TYPE_DIRICHLET_TREE;

    std::cout << "Initializing Tree" << std::endl;
    DirichletTreeIRV* dtree = new DirichletTreeIRV(
            nCandidates,
            scale,
            treeType,
            seed
    );

    std::cout << "Updating" << std::endl;
    dtree->update(biasBallot, biasCount);

    std::cout << "Sampling" << std::endl;
    std::string* samples = dtree->sample(nElections, nBallots);

    for( int i = 0; i < nElections; ++i ){
        std::cout << "Election " << i + 1 << ":\n";
        std::cout << samples[i] << std::endl;
    }

    return 1;
}
