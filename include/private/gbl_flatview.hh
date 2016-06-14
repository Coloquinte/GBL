// Copyright (C) 2016 Gabriel Gouvine - All Rights Reserved

#ifndef GBL_FLATVIEW_HH
#define GBL_FLATVIEW_HH

#include "private/gbl_forward_declarations.hh"
#include "gbl.hh"

#include <vector>
#include <unordered_map>
#include <algorithm>

namespace gbl {

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

    FlatModule getTop() const;

private:
    struct DownInfo {
        // Offset between child and parent flat indexing (local indexing, not the global contiguous one)
        FlatSize _offset;

        DownInfo(FlatSize offs) : _offset(offs) {}
    };
    struct UpInfo {
        // Offset between child and parent flat indexing (local indexing, not the global contiguous one)
        FlatSize _offset;
        Instance _parentInstance;

        UpInfo(Instance inst, FlatSize offs) : _offset(offs), _parentInstance(inst) {}
    };
    struct ParentInfos {
        // Interval for each parent instance 
        std::vector<FlatSize> _instEndIndexs;
        std::vector<UpInfo>   _upInfos;
    };
    struct ChildInfos {
        // Offset for each instance in the module (possibly InvalidFlatIndex for holes)
        std::vector<DownInfo> _downInfos;
    };
    friend FlatInstance;
    friend FlatModule;

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
    // From Module to FlatModule
    std::unordered_map<internal::ModuleImpl*, Size> _mod2Index;

    // Hierarchy bookkeeping
    // Up instances
    std::vector<ParentInfos> _parents;
    // Down instances
    std::vector<ChildInfos>  _children;

    // Contiguous indexing for modules (_modEndIndexs[i] to _modEndIndexs[i+1])
    std::vector<FlatSize> _modEndIndexs;

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

inline FlatModule FlatView::getTop() const {
    return FlatModule(FlatNode(FlatEltRef(_topMod.ref(), 0, *this)));
}

inline Wire FlatWire::getObject() {
    return Wire(_ref._ref._ptr, _ref._ref._ind);
}
inline Node FlatNode::getObject() {
    return Node(_ref._ref._ptr, _ref._ref._ind);
}
inline Instance FlatInstance::getObject() {
    return Instance(FlatNode::getObject());
}
inline Module FlatModule::getObject() {
    return Module(FlatNode::getObject().ref()._ptr);
}
inline Port FlatPort::getObject() {
    return Port(_ref._ref._ptr, _ref._ref._instInd, _ref._ref._portInd);
}
inline InstancePort FlatInstancePort::getObject() {
    return InstancePort(FlatPort::getObject());
}
inline ModulePort FlatModulePort::getObject() {
    return ModulePort(FlatPort::getObject());
}

inline bool FlatModule::isTop() {
    return operator==(_ref._view.getTop());
}

inline FlatEltRef::FlatEltRef(EltRef ref, FlatSize index, const FlatView& view) 
: _view(view)
, _ref(ref)
, _index(index)
{
}

inline FlatPortRef::FlatPortRef(PortRef ref, FlatSize index, const FlatView& view) 
: _view(view)
, _ref(ref)
, _index(index)
{
}

inline FlatNode::FlatNode(const FlatEltRef& ref) : _ref(ref) {}
inline FlatPort::FlatPort(const FlatPortRef& ref) : _ref(ref) {}
inline FlatModule::FlatModule(const FlatNode& node) : FlatNode(node) {}
inline FlatInstance::FlatInstance(const FlatNode& node) : FlatNode(node) {}
inline FlatModulePort::FlatModulePort(const FlatPort& port) : FlatPort(port) {}
inline FlatInstancePort::FlatInstancePort(const FlatPort& port) : FlatPort(port) {}

inline FlatInstance FlatModule::getUpInstance() {
    FlatView::ParentInfos info = _ref._view._parents[_ref._view.getModIndex(getObject())];
    Size index = bisectIndex(info._instEndIndexs, _ref._index);
    FlatView::UpInfo up = info._upInfos[index];
    assert(up._offset <= _ref._index);
    FlatEltRef ref(up._parentInstance.ref(), _ref._index - up._offset, _ref._view);
    return FlatInstance(FlatNode(ref));
}

inline FlatModule FlatInstance::getDownModule() {
    FlatView::DownInfo down = _ref._view._children[_ref._view.getModIndex(getObject().getParentModule())]._downInfos[_ref._ref._ind];
    FlatEltRef ref(getObject().getDownModule().ref(), _ref._index + down._offset, _ref._view);
    return FlatModule(FlatNode(ref));
}

} // End namespace gbl

#endif

