
#include "testing.hh"
#include "gbl.hh"
#include "gbl_symbols.hh"

#include <algorithm>
#include <random>

using namespace gbl;
using namespace gbl::internal;
using namespace std;

BOOST_AUTO_TEST_SUITE(FlatViewTest)

BOOST_AUTO_TEST_CASE(testFlatView) {
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
        BOOST_CHECK_EQUAL (flatview.getNumFlatInstanciations(mods[i]), 1lu << i);
    }
    FlatModule topMod = flatview.getTop();
    BOOST_CHECK (topMod.isTop());
    BOOST_CHECK_EQUAL(topMod.getIndex(), 0lu);
    
    FlatSize i = 0;
    for (FlatInstance inst : topMod.instances()) {
        FlatModule downMod = inst.getDownModule();
        BOOST_CHECK (inst.getParentModule() == topMod);
        BOOST_CHECK (!downMod.isTop());
        BOOST_CHECK (downMod.getUpInstance() == inst);
        BOOST_CHECK_EQUAL (inst.getIndex(), i+1);
        FlatSize j = 0;
        for (FlatInstance downInst : downMod.instances()) {
            BOOST_CHECK (downInst.getDownModule().getUpInstance() == downInst);
            BOOST_CHECK (downInst.getParentModule() == downMod);
            BOOST_CHECK_EQUAL (downInst.getIndex(), 2*j+i+3);
            ++j;
        }
        ++i;
    }
}

BOOST_AUTO_TEST_SUITE_END()

