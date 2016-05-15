// Copyright (C) 2016 Gabriel Gouvine - All Rights Reserved

#ifndef GBL_FLATVIEW_HH
#define GBL_FLATVIEW_HH

#include "private/gbl_forward_declarations.hh"
#include "gbl.hh"

#include <vector>
#include <unordered_map>
#include <algorithm>

namespace gbl {

// Reference for a flat node or wire
class FlatRef {
    public:
    FlatRef(const FlatView& view, Size modInd, Size ind, Size gblInd, FlatSize flatInd)
        : _view(view)
        , _flatInd(flatInd)
        , _modInd(modInd)
        , _ind(ind)
        , _gblIndex(gblInd)
        {}

    private:
    const FlatView& _view;
    // Index in the flat view: the main identifier
    FlatSize _flatInd;
    // Index of the parent module in the FlatView
    Size _modInd;
    // Index in the parent (or -1 for a module)
    Size _ind;
    // Index of the object (wire or node) in the usual gbl view
    Size _gblIndex;
};

/*
 *
 */
class FlatView {
public:
    FlatView(Module topModule);

    void selfcheck() const;

    FlatRef getModFlatRef(FlatSize flatIndex) const;
    FlatRef getInstFlatRef(FlatSize flatIndex) const;
    FlatRef getWireFlatRef(FlatSize flatIndex) const;

    FlatSize beginIndex(Node node) const;
    FlatSize endIndex(Node node) const;
    FlatSize beginIndex(Wire wire) const;
    FlatSize endIndex(Wire wire) const;

    FlatSize getNumFlatInstanciations(Node node) const;
    FlatSize getNumFlatInstanciations(Wire wire) const;

private:
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

    // Contiguous indexing for transhierarchical wires
    // TODO

    friend FlatRef;
};

inline Size bisectIndex(const std::vector<FlatSize>& vec, FlatSize flatIndex) {
    Size ind = std::distance(
        vec.begin(),
        std::upper_bound(vec.begin(), vec.end(), flatIndex)
    );
    assert(ind > 0 && ind < vec.size());
    return ind - 1;
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

inline FlatRef FlatView::getModFlatRef(FlatSize flatIndex) const {
    Size modInd = getModIndex(flatIndex);
    return FlatRef(*this, modInd, -1, 0, flatIndex);
}
inline FlatRef FlatView::getInstFlatRef(FlatSize flatIndex) const {
    // Index of the instanciated module
    Size modInd = getModIndex(flatIndex);
    // Index of the parent and the instance in the parent
    Size locParentInd = bisectIndex(_parents[modInd]._instEndIndexs, flatIndex);
    Size parentInd   = _parents[modInd]._upInfos[locParentInd]._moduleIndex;
    Size indInParent = _parents[modInd]._upInfos[locParentInd]._associatedIndex;
    Size instInd     = _parents[modInd]._upInfos[locParentInd]._instanceIndex;
    return FlatRef(*this, parentInd, indInParent, instInd, flatIndex);
}
inline FlatRef FlatView::getWireFlatRef(FlatSize flatIndex) const {
    Size modInd = getWireModIndex(flatIndex);
    Size indInParent = (flatIndex - _wireEndIndexs[modInd]) / _wires[modInd].size();
    Size wireInd = _wires[modInd][indInParent];
    return FlatRef(*this, modInd, indInParent, wireInd, flatIndex);
}

inline Size FlatView::getModIndex(Module module) const {
    return _mod2Index.at(module.ref()._ptr);
}
inline Size FlatView::getRepresentantModIndex(Node node) const {
    return _mod2Index.at(node.ref()._ptr->_nodes[node.ref()._ind]._instanciation);
}

inline FlatSize FlatView::beginIndex(Node node) const {
    return _modEndIndexs[getRepresentantModIndex(node)];
}
inline FlatSize FlatView::endIndex(Node node) const {
    return _modEndIndexs[getRepresentantModIndex(node) + 1];
}
inline FlatSize FlatView::beginIndex(Wire wire) const {
    Size modInd = getModIndex(wire.getParentModule());
    FlatSize wireIndex = _wireHierToInternal[modInd][wire.ref()._ind];
    return _wireEndIndexs[modInd] + getNumFlatInstanciations(modInd) * wireIndex;
}
inline FlatSize FlatView::endIndex(Wire wire) const {
    Size modInd = getModIndex(wire.getParentModule());
    FlatSize wireIndex = _wireHierToInternal[modInd][wire.ref()._ind];
    return _wireEndIndexs[modInd] + getNumFlatInstanciations(modInd) * (wireIndex+1);
}

inline FlatSize FlatView::getNumFlatInstanciations(Size modIndex) const {
    return _modEndIndexs[modIndex+1] - _modEndIndexs[modIndex];
}
inline FlatSize FlatView::getNumFlatInstanciations(Node node) const {
    return getNumFlatInstanciations(getRepresentantModIndex(node));
}
inline FlatSize FlatView::getNumFlatInstanciations(Wire wire) const {
    return getNumFlatInstanciations(getModIndex(wire.getParentModule()));
}

} // End namespace gbl

#endif

