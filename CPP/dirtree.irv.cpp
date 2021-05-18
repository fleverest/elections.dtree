#include <list>
#include <string>
#include <fstream>

using namespace std;

const string PTYPE_UNIFORM = "uniform";
const string PTYPE_DIRICHLET = "dirichlet";

struct node {
    float alpha;
    string name;
    int ballots;
    list<node>* children;
};

class IRV_Dirichlet_Tree {
public:
    // Constructor
    IRV_Dirichlet_Tree (list<string> candidates, string ptype, float scale);
    

private:
    // attributes
    node root;
    float scale;
    string ptype;
    list<string> candidates;
    
    void build () {
        root.alpha=scale;
        root.ballots=0;
        root.name="";
        buildSubTree(root, candidates);
    }
    void buildSubTree(node n, list<string> remaining) {
        n.alpha = 0;
        n.ballots = 0;
        if (remaining==2) {
            n.children = NULL;
            return scale;
        }
        n.children = {};
        for (int i=0; i<remaining; i++) {
            node child;
            child.name = n.name + "." + to_string(remaining[i]);
            list<string> child_remaining = list(remaining);
            child_remaining.erase(i);
            n.alpha += buildSubTree(child, child_remaining);
            delete child_remaining;
            n.children.push_back(
                child
            );
        }
        if (strcmp(PTYPE_DIRICHLET, ptype)) { // if ptype is not dirichlet
            n.alpha = scale; // reset alpha to initial scale
        }
    }
}
