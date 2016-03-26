
#include "gbl.hh"

#include <iostream>

using namespace gbl;
using namespace std;

void testBasicCreation() {
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

int main() {
    testBasicCreation();
    return 0;
}

