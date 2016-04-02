
#include "gbl.hh"

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
    cout << "Started test for iterators" << endl;

    const int numPorts = 100;
    const int numWires = 400;
    const int numInsts = 300;
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

    cout << "Finished test for iterators" << endl;
}

int main() {
    testBasicConstruction();
    testIteration();
    return 0;
}

