
#include "private/gbl_flatview.hh"

#include <unordered_set>

namespace gbl {

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

FlatView::FlatView(Module topModule)
: _topMod(topModule)
{
    using namespace std;
    // Deterministic topological traversal from top
    std::unordered_set<internal::ModuleImpl*> visitedSet;
    visitModule(_topMod, _mods, visitedSet);
    std::reverse(_mods.begin(), _mods.end());
    assert(!_mods.empty() && _mods.front() == _topMod.ref()._ptr);

    _parents.resize(_mods.size());
    _children.resize(_mods.size());

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
    assert(_modEndIndexs.size() == _mods.size() + 1);

    // Init each parent/child couple
    for (Size i=0; i<_mods.size(); ++i) {
        _parents[i]._instEndIndexs.push_back(_modEndIndexs[i]);
    }

    for (Size i=0; i<_mods.size(); ++i) {
        Module module(_mods[i]);
        for (Instance instance : module.instances()) {
            Size downModIndex = _mod2Index[instance.getDownModule().ref()._ptr];
            Size instIndex = instance.ref()._ind;
            std::vector<FlatSize>& instIndexs = _parents[downModIndex]._instEndIndexs;
            FlatSize offset = instIndexs.back() - _modEndIndexs[i];
            _parents[downModIndex]._upInfos.emplace_back(i, instIndex, offset);
            _children[i]._downInfos.emplace_back(downModIndex, instIndex, offset);
            instIndexs.push_back(instIndexs.back() + flatSizes[i]);
        }
    }

    // Checks for all but the top Module
    for (Size i=1; i<_mods.size(); ++i) {
        assert(_parents[i]._instEndIndexs.front() == _modEndIndexs[i]);
        assert(_parents[i]._instEndIndexs.back() == _modEndIndexs[i+1]);
    }
    
}
} // End namespace gbl

