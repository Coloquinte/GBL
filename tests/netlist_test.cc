
#include "testing.hh"
#include "gbl.hh"
#include "gbl_symbols.hh"

#include <iostream>
#include <algorithm>
#include <random>

using namespace gbl;
using namespace std;

BOOST_AUTO_TEST_SUITE(NetlistTest)

BOOST_AUTO_TEST_CASE(testBasicConstruction) {
    Module hierMod = Module::createHier();
    Module leafMod = Module::createLeaf();

    Instance inst = hierMod.createInstance(leafMod);
    Wire w = hierMod.createWire();

    ModulePort mpt1 = leafMod.createPort();
    ModulePort mpt2 = leafMod.createPort();

    InstancePort ipt1 = mpt1.getUpPort(inst);
    InstancePort ipt2 = mpt2.getUpPort(inst);
    BOOST_CHECK (!ipt1.isConnected());
    BOOST_CHECK (!ipt2.isConnected());

    ipt1.connect(w);
    BOOST_CHECK (ipt1.isConnected());
    BOOST_CHECK (!ipt2.isConnected());

    ipt2.connect(w);
    BOOST_CHECK (ipt1.isConnected());
    BOOST_CHECK (ipt2.isConnected());

    ipt1.disconnect();
    BOOST_CHECK (!ipt1.isConnected());
    BOOST_CHECK (ipt2.isConnected());

    mpt1.destroy();
    inst.destroy();
    w.destroy();
}

BOOST_AUTO_TEST_CASE(testIteration) {
    const int numPorts = 100;
    const int numWires = 400;
    const int numInsts = 300;
    const int numTestIDs = 20;

    random_device r;
    default_random_engine rengine(r());

    Module fstMod = Module::createHier();
    Module sndMod = Module::createHier();
    Module leafMod = Module::createLeaf();

    // Create ports, instances and wires
    vector<ModulePort> leafPorts;
    for (int i=0; i<numPorts; ++i) {
        leafPorts.push_back(leafMod.createPort());
    }
    BOOST_CHECK_EQUAL (leafPorts.size(), leafMod.ports().size());
    vector<Instance> instances;
    for (int i=0; i<numInsts; ++i) {
        instances.push_back(fstMod.createInstance(leafMod));
    }
    vector<Wire> wires;
    for (int i=0; i<numWires; ++i) {
        wires.push_back(fstMod.createWire());
    }

    // Test that the instances and wires match
    for (Instance inst : fstMod.instances()) {
        BOOST_CHECK (find(instances.begin(), instances.end(), inst) != instances.end());
        BOOST_CHECK_EQUAL (leafPorts.size(), inst.ports().size());
    }
    BOOST_CHECK_EQUAL  (fstMod.instances().size(), instances.size());
    for (Wire wire : fstMod.wires()) {
        BOOST_CHECK (find(wires.begin(), wires.end(), wire) != wires.end());
    }
    BOOST_CHECK_EQUAL (fstMod.wires().size(), wires.size());

    // Connect stuff randomly
    for (Instance inst : fstMod.instances()) {
        std::uniform_int_distribution<int> rwireInd(0, numWires-1);
        for (Port p : inst.ports()) {
            int ri = rwireInd(rengine);
            p.connect(wires[ri]);
        }
    }

    for (Instance inst : fstMod.instances()) {
        BOOST_CHECK (!inst.eraseProperty(Symbol::VCC));
        BOOST_CHECK (!inst.hasProperty(Symbol::VCC));
        BOOST_CHECK ( inst.addProperty(Symbol::VCC));
        BOOST_CHECK (!inst.addProperty(Symbol::VCC));
        BOOST_CHECK ( inst.hasProperty(Symbol::VCC));
        BOOST_CHECK ( inst.eraseProperty(Symbol::VCC));
        BOOST_CHECK (!inst.hasProperty(Symbol::VCC));

        BOOST_CHECK_EQUAL (inst.names().size(), 0);
        BOOST_CHECK_EQUAL (inst.properties().size(), 0);
        for (ID i=0; i<numTestIDs; ++i) {
            inst.addName(i);
            inst.addProperty(i);
        }
        BOOST_CHECK_EQUAL (inst.names().size(), numTestIDs);
        BOOST_CHECK_EQUAL (inst.properties().size(), numTestIDs);
    }

    for (Port port : leafMod.ports()) {
        BOOST_CHECK (!port.eraseProperty(Symbol::VCC));
        BOOST_CHECK (!port.hasProperty(Symbol::VCC));
        BOOST_CHECK ( port.addProperty(Symbol::VCC));
        BOOST_CHECK (!port.addProperty(Symbol::VCC));
        BOOST_CHECK ( port.hasProperty(Symbol::VCC));
        BOOST_CHECK ( port.eraseProperty(Symbol::VCC));
        BOOST_CHECK (!port.hasProperty(Symbol::VCC));

        BOOST_CHECK_EQUAL (port.names().size(), 0);
        BOOST_CHECK_EQUAL (port.properties().size(), 0);
        for (ID i=0; i<numTestIDs; ++i) {
            port.addName(i);
            port.addProperty(i);
        }
        BOOST_CHECK_EQUAL (port.names().size(), numTestIDs);
        BOOST_CHECK_EQUAL (port.properties().size(), numTestIDs);
    }

    for (Port port : instances[0].ports()) {
        BOOST_CHECK (!port.eraseProperty(Symbol::VCC));
        BOOST_CHECK (!port.hasProperty(Symbol::VCC));
        BOOST_CHECK ( port.addProperty(Symbol::VCC));
        BOOST_CHECK (!port.addProperty(Symbol::VCC));
        BOOST_CHECK ( port.hasProperty(Symbol::VCC));
        BOOST_CHECK ( port.eraseProperty(Symbol::VCC));
        BOOST_CHECK (!port.hasProperty(Symbol::VCC));

        BOOST_CHECK_EQUAL (port.names().size(), 0);
        BOOST_CHECK_EQUAL (port.properties().size(), 0);
        for (ID i=0; i<numTestIDs; ++i) {
            port.addName(i);
            port.addProperty(i);
        }
        BOOST_CHECK_EQUAL (port.names().size(), numTestIDs);
        BOOST_CHECK_EQUAL (port.properties().size(), numTestIDs);
    }
}

BOOST_AUTO_TEST_SUITE_END()

