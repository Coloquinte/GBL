// Copyright (C) 2016 Gabriel Gouvine - All Rights Reserved

#ifndef GBL_FLATVIEW_HH
#define GBL_FLATVIEW_HH

#include "private/gbl_forward_declarations.hh"
#include "gbl.hh"

#include <vector>
#include <unordered_map>
#include <algorithm>

namespace gbl {
namespace internal {

/*
 * Internal datastructure for flat netlist view
 */
class FlatView {
public:
    FlatView(Module topModule);

    void selfcheck() const;

    FlatSize beginIndex(Node node) const;
    FlatSize endIndex(Node node) const;
    FlatSize beginIndex(Wire wire) const;
    FlatSize endIndex(Wire wire) const;

    FlatSize getNumFlatInstanciations(Node node) const;
    FlatSize getNumFlatInstanciations(Wire wire) const;

private:
    // TODO: parent info is only instance + flat index offset; child info is only flat index offset
    struct HierInfo {
        // Offset between child and parent flat indexing ranges
        FlatSize _offset;
        // Index of the module (child or parent) in the FlatView
        Size _moduleIndex;
        // Index of the corresponding instance in the parent gbl module
        Size _instanceIndex;
        // Index of the HierInfo in the associated child/parent
        Size _associatedIndex;
        HierInfo(Size modInd=-1, Size instInd=-1, Size associatedIndex=-1, FlatSize offs=-1)
            : _offset(offs)
            , _moduleIndex(modInd)
            , _instanceIndex(instInd)
            , _associatedIndex(associatedIndex)
            {}
    };
    struct ParentInfos {
        std::vector<FlatSize> _instEndIndexs;
        std::vector<HierInfo> _upInfos;
    };
    struct ChildInfos {
        std::vector<HierInfo> _downInfos;
    };

private:
    Size getModIndex(FlatSize flatIndex) const;
    Size getWireModIndex(FlatSize flatIndex) const;
    Size getPortModIndex(FlatSize flatIndex) const;

    Size getModIndex(Module module) const;
    Size getRepresentantModIndex(Node node) const;

    FlatSize getNumFlatInstanciations(Size modIndex) const;

private:
    Module _topMod;

    // Basic module <--> index range bookkeeping
    std::vector<internal::ModuleImpl*>   _mods;
    // Module to flat index range (_modEndIndexs[i] to _modEndIndexs[i+1] for module i)
    std::vector<FlatSize> _modEndIndexs;

    // From Module to FlatModule
    std::unordered_map<internal::ModuleImpl*, Size> _mod2Index;

    // Hierarchy bookkeeping
    // Up instances
    std::vector<ParentInfos> _parents;
    // Down instances
    std::vector<ChildInfos>  _children;
    std::vector<std::vector<Size> > _instHierToInternal;

    // Contiguous indexing for wires
    std::vector<FlatSize> _wireEndIndexs;
    std::vector<std::vector<Size> > _wires;
    std::vector<std::vector<Size> > _wireHierToInternal;

    // Contiguous indexing for ports
    std::vector<FlatSize> _portEndIndexs;
    std::vector<std::vector<Size> > _ports;
    std::vector<std::vector<Size> > _portHierToInternal;
};

inline Size bisectIndex(const std::vector<FlatSize>& vec, FlatSize flatIndex) {
    Size ind = std::distance(
        vec.begin(),
        std::upper_bound(vec.begin(), vec.end(), flatIndex)
    );
    assert(ind > 0 && ind < vec.size());
    return ind - 1;
}

inline Size FlatView::getModIndex(Module module) const {
    return _mod2Index.at(module.ref()._ptr);
}

inline Size FlatView::getModIndex(FlatSize flatIndex) const {
    return bisectIndex(_modEndIndexs, flatIndex);
}
inline Size FlatView::getWireModIndex(FlatSize flatIndex) const {
    return bisectIndex(_wireEndIndexs, flatIndex);
}
inline Size FlatView::getPortModIndex(FlatSize flatIndex) const {
    return bisectIndex(_portEndIndexs, flatIndex);
}

inline FlatSize FlatView::getNumFlatInstanciations(Size modIndex) const {
    return _modEndIndexs[modIndex+1] - _modEndIndexs[modIndex];
}
inline FlatSize FlatView::getNumFlatInstanciations(Node node) const {
    return getNumFlatInstanciations(getModIndex(node.getParentModule()));
}
inline FlatSize FlatView::getNumFlatInstanciations(Wire wire) const {
    return getNumFlatInstanciations(getModIndex(wire.getParentModule()));
}

} // End namespace internal
} // End namespace gbl

#endif

