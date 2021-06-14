#ifndef BALLOT_H
#define BALLOT_H

#include <string>
#include <sstream>
#include <vector>

struct BallotCount
{
    int* ballot; // A ballot is represented by a list of indices which
                 // navigate down the tree to the corresponding node.
    int count;   // The number of occurances of that ballot in the set.
};

typedef std::vector<BallotCount> election;

// Formats a BallotCount as a string.
std::string bcToStr(BallotCount bc, int nCandidates){
    std::ostringstream out;
    for( int i = 0; i <= nCandidates - 2; ++i ){
        out << bc.ballot[i] << ",";
    }

    out << bc.count;
    return out.str();
}

// Formats an election as a stringe
std::string electionToStr(election e, int nCandidates){
    std::ostringstream out;

    for( BallotCount bc: e ){
        out << bcToStr(bc, nCandidates) << std::endl;
    }

    return out.str();
}

#endif

