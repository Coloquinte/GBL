
#include "gbl.hh"
#include "gbl_symbols.hh"

#include <iostream>
#include <algorithm>
#include <random>

using namespace gbl;
using namespace std;

void testBasicConstruction() {
    cout << "Started test for module construction" << endl;
    Module hierMod = Module::createHier();
    Module leafMod = Module::createLeaf();

    Instance inst = hierMod.createInstance(leafMod);
    Wire w = hierMod.createWire();

    ModulePort mpt1 = leafMod.createPort();
    ModulePort mpt2 = leafMod.createPort();
    InstancePort ipt1 = mpt1.getUpPort(inst);
    InstancePort ipt2 = mpt2.getUpPort(inst);
    if (ipt1.isConnected() || ipt2.isConnected()) {
        cerr << "Connected port at construction time" << endl;
        abort();
    }
    ipt1.connect(w);
    ipt2.connect(w);
    if (!ipt1.isConnected() || !ipt2.isConnected()) {
        cerr << "Failed port connection" << endl;
        abort();
    }
    ipt1.disconnect();
    if (ipt1.isConnected()) {
        cerr << "Failed port disconnection" << endl;
        abort();
    }
    mpt1.destroy();
    inst.destroy();
    w.destroy();
    cout << "Finished test for module construction" << endl;
}

void testIteration() {
    cout << "Started test for iterators and data" << endl;

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
    if (leafPorts.size() != leafMod.ports().size()) abort();
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
        if (find(instances.begin(), instances.end(), inst) == instances.end()) {
            abort();
        }
        if (leafPorts.size() != inst.ports().size()) abort();
    }
    if (fstMod.instances().size() != instances.size()) { abort(); }
    for (Wire wire : fstMod.wires()) {
        if (find(wires.begin(), wires.end(), wire) == wires.end()) {
            abort();
        }
    }
    if (fstMod.wires().size() != wires.size()) { abort(); }

    // Connect stuff randomly
    for (Instance inst : fstMod.instances()) {
        std::uniform_int_distribution<int> rwireInd(0, numWires-1);
        for (Port p : inst.ports()) {
            int ri = rwireInd(rengine);
            p.connect(wires[ri]);
        }
    }

    for (Instance inst : fstMod.instances()) {
        if ( inst.eraseProperty(Symbol::VCC)) abort();
        if ( inst.hasProperty(Symbol::VCC)) abort();
        if (!inst.addProperty(Symbol::VCC)) abort();
        if ( inst.addProperty(Symbol::VCC)) abort();
        if (!inst.hasProperty(Symbol::VCC)) abort();
        if (!inst.eraseProperty(Symbol::VCC)) abort();
        if ( inst.hasProperty(Symbol::VCC)) abort();

        if (inst.names().size() != 0) abort();
        if (inst.properties().size() != 0) abort();
        for (ID i=0; i<numTestIDs; ++i) {
            inst.addName(i);
            inst.addProperty(i);
        }
        if (inst.names().size() != numTestIDs) abort();
        if (inst.properties().size() != numTestIDs) abort();
    }

    for (Port port : leafMod.ports()) {
        if ( port.eraseProperty(Symbol::VCC)) abort();
        if ( port.hasProperty(Symbol::VCC)) abort();
        if (!port.addProperty(Symbol::VCC)) abort();
        if ( port.addProperty(Symbol::VCC)) abort();
        if (!port.hasProperty(Symbol::VCC)) abort();
        if (!port.eraseProperty(Symbol::VCC)) abort();
        if ( port.hasProperty(Symbol::VCC)) abort();

        if (port.names().size() != 0) abort();
        if (port.properties().size() != 0) abort();
        for (ID i=0; i<numTestIDs; ++i) {
            port.addName(i);
            port.addProperty(i);
        }
        if (port.names().size() != numTestIDs) abort();
        if (port.properties().size() != numTestIDs) abort();
    }

    for (Port port : instances[0].ports()) {
        if ( port.eraseProperty(Symbol::VCC)) abort();
        if ( port.hasProperty(Symbol::VCC)) abort();
        if (!port.addProperty(Symbol::VCC)) abort();
        if ( port.addProperty(Symbol::VCC)) abort();
        if (!port.hasProperty(Symbol::VCC)) abort();
        if (!port.eraseProperty(Symbol::VCC)) abort();
        if ( port.hasProperty(Symbol::VCC)) abort();

        if (port.names().size() != 0) abort();
        if (port.properties().size() != 0) abort();
        for (ID i=0; i<numTestIDs; ++i) {
            port.addName(i);
            port.addProperty(i);
        }
        if (port.names().size() != numTestIDs) abort();
        if (port.properties().size() != numTestIDs) abort();
    }

    cout << "Finished test for iterators and data" << endl;
}

int main() {
    testBasicConstruction();
    testIteration();
    return 0;
}

