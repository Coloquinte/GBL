
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
    const int numWires = 10;
    const int numPorts = 5;
    std::vector<Module> mods;
    mods.push_back(Module::createHier());
    for (int i=0; i<designDepth; ++i) {
        Module last = Module::createHier();
        Instance d1 = mods.back().createInstance(last);
        mods.back().createInstance(last);
        Instance d2 = mods.back().createInstance(last);
        mods.back().createInstance(last);
        d1.destroy();
        d2.destroy();
        mods.push_back(last);

        for (int j=0; j<numWires; ++j) {
            last.createWire();
        }
        for (int j=0; j<numPorts; ++j) {
            last.createPort();
        }
    }
    FlatView flatview(mods.front());
    for (int i=0; i<designDepth; ++i) {
        if (flatview.getNumFlatInstanciations(mods[i]) != 1lu << i) {
            abort();
        }
    }
    return 0;
}

