// Copyright (C) 2016 Gabriel Gouvine - All Rights Reserved

#include "gbl_flatview.hh"

#include <unordered_set>

namespace gbl {

namespace { // Helpers
void visitModule(Module module, std::vector<internal::ModuleImpl*>& moduleOrder, std::unordered_set<internal::ModuleImpl*>& visited) {
    // Will ignore loops awkwardly for now; we could just error out if loops are present
    visited.insert(module.ref()._ptr);
    for (Instance instance : module.instances()) {
        Module downModule = instance.getDownModule();
        if (visited.count(downModule.ref()._ptr) == 0) {
            visitModule(downModule, moduleOrder, visited);
        }
    }
    moduleOrder.push_back(module.ref()._ptr);
}
} // End anonymous namespace

FlatView::FlatView(Module topModule)
: _topMod(topModule)
{
    using namespace std;
    // Deterministic topological traversal from top
    std::unordered_set<internal::ModuleImpl*> visitedSet;
    visitModule(_topMod, _mods, visitedSet);
    std::reverse(_mods.begin(), _mods.end());

    _parents.resize(_mods.size());
    _children.resize(_mods.size());
    _wires.resize(_mods.size());
    _ports.resize(_mods.size());
    _wireHierToInternal.resize(_mods.size());
    _portHierToInternal.resize(_mods.size());

    // Create structure for module to index translation
    for (Size i=0; i<_mods.size(); ++i) {
        _mod2Index[_mods[i]] = i;
    }

    // Size of the flat indexing range for each module
    std::vector<FlatSize> flatSizes(_mods.size(), 0);
    // The top module has exactly one instanciation
    flatSizes[0] = 1;
    // First flat index of the top module is 0
    _modEndIndexs.push_back(0);

    for (Size i=0; i<_mods.size(); ++i) {
        Module module(_mods[i]);
        FlatSize fsize = flatSizes[i];
        assert(fsize > 0);
        for (Instance instance : module.instances()) {
            Size downModIndex = _mod2Index[instance.getDownModule().ref()._ptr];
            assert(downModIndex > i);
            flatSizes[downModIndex] += fsize;
        }
        _modEndIndexs.push_back(fsize + _modEndIndexs.back());
    }

    // Init each parent/child couple
    for (Size i=0; i<_mods.size(); ++i) {
        _parents[i]._instEndIndexs.push_back(0);
    }
    for (Size modIndex=0; modIndex<_mods.size(); ++modIndex) {
        Module module(_mods[modIndex]);
        _children[modIndex]._downInfos.emplace_back(0); // For the module, which is the node of _ind 0
        for (Instance instance : module.instances()) {
            Size instIndex = instance.ref()._ind;
            Size downModIndex = _mod2Index[instance.getDownModule().ref()._ptr];

            std::vector<FlatSize>& instIndexs = _parents[downModIndex]._instEndIndexs;
            FlatSize offset = instIndexs.back();
            instIndexs.push_back(instIndexs.back() + flatSizes[modIndex]);

            _parents[downModIndex]._upInfos.emplace_back(instance, offset);
            // Handle holes in the instance list
            while(_children[modIndex]._downInfos.size() < instIndex) {
                _children[modIndex]._downInfos.emplace_back(InvalidFlatIndex);
            }
            _children[modIndex]._downInfos.emplace_back(offset);
        }
    }

    _wireEndIndexs.push_back(0);
    _portEndIndexs.push_back(0);
    for (Size i=0; i<_mods.size(); ++i) {
        Module module(_mods[i]);
        for (Wire wire : module.wires()) {
            _wires[i].push_back(wire.ref()._ind);
            while (_wireHierToInternal[i].size() < wire.ref()._ind) {
                _wireHierToInternal[i].push_back(InvalidIndex);
            }
            _wireHierToInternal[i].push_back(_wires[i].size()-1);
            assert(_wireHierToInternal[i].size() == wire.ref()._ind + 1);
        }
        _wireEndIndexs.push_back(_wireEndIndexs.back() + _wires[i].size() * getNumFlatInstanciations(i));
        for (Port port : module.ports()) {
            _ports[i].push_back(port.ref()._portInd);
            while (_portHierToInternal[i].size() < port.ref()._portInd) {
                _portHierToInternal[i].push_back(InvalidIndex);
            }
            _portHierToInternal[i].push_back(_ports[i].size()-1);
            assert(_portHierToInternal[i].size() == port.ref()._portInd + 1);
        }
        _portEndIndexs.push_back(_portEndIndexs.back() + _ports[i].size() * getNumFlatInstanciations(i));
    }

    selfcheck();
}

void FlatView::selfcheck() const {
    assert(!_mods.empty() && _mods.front() == _topMod.ref()._ptr);
    assert(_modEndIndexs.size() == _mods.size() + 1);
    assert(_wires.size() == _mods.size());
    assert(_ports.size() == _mods.size());
    assert(_wireHierToInternal.size() == _mods.size());
    assert(_portHierToInternal.size() == _mods.size());
    assert(_wireEndIndexs.size() == _mods.size() + 1);
    assert(_portEndIndexs.size() == _mods.size() + 1);

    // Checks for all but the top Module
    for (Size i=1; i<_mods.size(); ++i) {
        assert(_parents[i]._instEndIndexs.front() == 0);
        assert(_parents[i]._instEndIndexs.back() == _modEndIndexs[i+1] - _modEndIndexs[i]);
        assert(_parents[i]._instEndIndexs.size() == _parents[i]._upInfos.size() + 1);

        for (Size j=0; j<_parents[i]._upInfos.size(); ++j) {
            UpInfo upInfo = _parents[i]._upInfos[j];
            Instance inst = upInfo._parentInstance;
            Size parentInd = getModIndex(inst.getParentModule());
            assert(parentInd < i);
        }
    }

    // Children are enumerated in order of their index in the gbl module
    for (Size i=0; i<_mods.size(); ++i) {
        // Test strictly ordered collections
        assert( std::is_sorted(_wires[i].begin(), _wires[i].end()) );
        assert( std::is_sorted(_ports[i].begin(), _ports[i].end()) );
        assert( std::adjacent_find(_wires[i].begin(), _wires[i].end()) == _wires[i].end());
        assert( std::adjacent_find(_ports[i].begin(), _ports[i].end()) == _ports[i].end());

        // Test that we have a correct translation from and to hier indexes
        for (Size j=0; j<_wireHierToInternal[i].size(); ++j) {
            Size flatInd = _wireHierToInternal[i][j];
            assert(flatInd == InvalidIndex || _wires[i].at(flatInd) == j);
        }
        for (Size j=0; j<_wires[i].size(); ++j) {
            assert(_wireHierToInternal[i].at(_wires[i][j]) == j);
        }
        for (Size j=0; j<_portHierToInternal[i].size(); ++j) {
            Size flatInd = _portHierToInternal[i][j];
            assert(flatInd == InvalidIndex || _ports[i].at(flatInd) == j);
        }
        for (Size j=0; j<_ports[i].size(); ++j) {
            assert(_portHierToInternal[i].at(_ports[i][j]) == j);
        }
    }
}

} // End namespace gbl

