
#include "private/gbl_flatview.hh"
#include "gbl_symbols.hh"

#include <iostream>
#include <algorithm>
#include <random>

using namespace gbl;
using namespace std;

int main() {
    // Create a huge design with 2^60 flat instances to check that the structure doesn't consume a linear amount of memory
    const int designDepth = 59;
    std::vector<Module> mods;
    mods.push_back(Module::createHier());
    for (int i=0; i<designDepth; ++i) {
        Module last = Module::createHier();
        mods.back().createInstance(last);
        mods.back().createInstance(last);
        mods.push_back(last);
    }
    FlatView flatview(mods.front());
    return 0;
}
