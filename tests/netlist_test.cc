
#include "testing.hh"
#include "gbl.hh"
#include "gbl_symbols.hh"
#include "gbl_flatview.hh"

#include <iostream>
#include <algorithm>
#include <random>

using namespace gbl;
using namespace std;

class ModuleGenerator {
    public:
    ModuleGenerator(int dpth = 0, int seed = 0)
    : rengine(seed)
    {
        depth = dpth;
        iters = 2;
        portIters = 2;

        instCount = 10;
        instDestroyProb = 0.2;
        portCount = 10;
        portDestroyProb = 0.2;
        wireCount = 10;
        wireDestroyProb = 0.2;

        portConnectProb = 0.9;
        portDisconnectProb = 0.1;
    }

    Module getModule(int i=0) {
        return _mods[i];
    }

    void initPorts(int creationNumber, float destructionProbability) {
        for (Module mod : _mods) {
            Size initSize = mod.ports().size();
            for (int j=0; j<creationNumber; ++j) {
                mod.createPort();
            }
            Size grownSize = mod.ports().size();
            assert(grownSize == initSize + creationNumber);
            unsigned removalNumber = 0;
            for (ModulePort port : mod.ports()) {
                std::uniform_real_distribution<float> dist(0.0, 1.0);
                if (dist(rengine) < destructionProbability) {
                    port.destroy();
                    ++removalNumber;
                }
            }
            assert(mod.ports().size() == grownSize - removalNumber);
        }
    }

    void createInstances(int creationNumber, float destructionProbability) {
        for (unsigned i=0; i+1<_mods.size(); ++i) {
            Module mod = _mods[i];
            Module downMod = _mods[i+1];
            Size initSize = mod.instances().size();
            for (int j=0; j<creationNumber; ++j) {
                Instance inst = mod.createInstance(downMod);
                assert(inst.getParentModule() == mod);
                assert(inst.getDownModule() == downMod);
            }
            Size grownSize = mod.instances().size();
            assert(grownSize == initSize + creationNumber);
            unsigned removalNumber = 0;
            for (Instance inst : mod.instances()) {
                std::uniform_real_distribution<float> dist(0.0, 1.0);
                if (dist(rengine) < destructionProbability) {
                    inst.destroy();
                    assert(!inst.isValid());
                    ++removalNumber;
                }
            }
            assert(mod.instances().size() == grownSize - removalNumber);
        }
    }

    void createWires(int creationNumber, float destructionProbability) {
        for (Module mod : _mods) {
            Size initSize = mod.wires().size();
            for (int j=0; j<creationNumber; ++j) {
                Wire wire = mod.createWire();
                assert(wire.getParentModule() == mod);
            }
            Size grownSize = mod.wires().size();
            assert(grownSize == initSize + creationNumber);
            unsigned removalNumber = 0;
            for (Wire wire : mod.wires()) {
                std::uniform_real_distribution<float> dist(0.0, 1.0);
                if (dist(rengine) < destructionProbability) {
                    wire.destroy();
                    assert(!wire.isValid());
                    ++removalNumber;
                }
            }
            assert(mod.wires().size() == grownSize - removalNumber);
        }
    }

    void connectPorts(float connectionProbability, float disconnectionProbability) {
        for (Module mod : _mods) {
            const std::vector<Wire> wires(mod.wires().begin(), mod.wires().end());
            
            for (Node node : mod.nodes()) {
                std::uniform_real_distribution<float> dist(0.0, 1.0);
                for (Port port : node.ports()) {
                    assert(node.isValid());
                    assert(port.isValid());
                    assert(port.getNode() == node);
                    if (port.isConnected() && dist(rengine) < disconnectionProbability) {
                        assert(port.getWire().isValid());
                        port.disconnect();
                    }
                    if (!wires.empty() && !port.isConnected() && dist(rengine) < connectionProbability) {
                        std::uniform_int_distribution<int> wireDist(0, wires.size() - 1);
                        Wire wire = wires[wireDist(rengine)];
                        port.connect(wire);
                    }
                }
            }
        }
    }

    void check() {
        for (Module mod : _mods) {
            for (Wire wire : mod.wires()) {
                assert(wire.getParentModule() == mod);
                assert(wire.isValid());
            }
            for (Instance inst : mod.instances()) {
                assert(inst.getParentModule() == mod);
                assert(inst.isValid());
            }
            for (Node node : mod.nodes()) {
                for (Port port : node.ports()) {
                    assert(port.getParentModule() == mod);
                    assert(port.isValid());
                }
            }
        }
    }

    void run() {
        _mods.clear();
        for (int i=0; i<depth+1; ++i) {
            _mods.push_back(Module::createHier());
            assert(_mods.back().isValid());
        }
        _mods.push_back(Module::createLeaf());
        assert(_mods.back().isValid());

        for (int i=0; i<portIters; ++i){
            initPorts(portCount, portDestroyProb);
            check();
        }
        for (int i=0; i<iters; ++i) {
            createInstances(instCount, instDestroyProb);
            check();
            createWires(wireCount, wireDestroyProb);
            check();
            connectPorts(portConnectProb, portDisconnectProb);
            check();
        }
    }

    private:
    std::vector<Module> _mods;
    std::mt19937 rengine;

    public:
    int depth;

    int iters;
    int portIters;

    int instCount;
    float instDestroyProb;
    int portCount;
    float portDestroyProb;
    int wireCount;
    float wireDestroyProb;

    float portConnectProb;
    float portDisconnectProb;
};

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

BOOST_AUTO_TEST_CASE(testRandomConstruction) {
    ModuleGenerator gen(20);
    gen.iters = 10;
    gen.portIters = 10;
    gen.instCount = 100;
    gen.instDestroyProb = 0.9;
    gen.portCount = 100;
    gen.portDestroyProb = 0.9;
    gen.wireCount = 100;
    gen.wireDestroyProb = 0.9;
    gen.portConnectProb = 0.6;
    gen.portDisconnectProb = 0.6;
    gen.run();
}

BOOST_AUTO_TEST_CASE(testRandomFlatView) {
    ModuleGenerator gen(1);
    gen.run();
    Module mod = gen.getModule();
    FlatView view(mod);
    BOOST_CHECK (view.getTop().getObject() == mod);

    for (FlatSize i=0; i<view.getNumFlatModules(); ++i) {
        FlatModule mod = view.getFlatModuleByIndex(i);
        BOOST_CHECK (mod.isTop() == (i==0lu));
        BOOST_CHECK (mod.getObject().isValid());
        BOOST_CHECK_EQUAL (mod.getIndex(), i);
        if (!mod.isTop()) {
            BOOST_CHECK_EQUAL (mod.getUpInstance().getIndex(), i);
        }
    }
    for (FlatSize i=0; i<view.getNumFlatWires(); ++i) {
        FlatWire wire = view.getFlatWireByIndex(i);
        BOOST_CHECK (wire.getObject().isValid());
        BOOST_CHECK_EQUAL (wire.getIndex(), i);
    }
    for (FlatSize i=0; i<view.getNumFlatPorts(); ++i) {
        FlatModulePort port = view.getFlatModulePortByIndex(i);
        BOOST_CHECK (port.getObject().isValid());
        BOOST_CHECK_EQUAL (port.getIndex(), i);
        if (!port.isTopPort()) {
            BOOST_CHECK_EQUAL (port.getUpPort().getIndex(), i);
            BOOST_CHECK (port.getUpPort().getDownPort() == port);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

