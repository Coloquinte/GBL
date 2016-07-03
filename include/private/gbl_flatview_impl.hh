// Copyright (C) 2016 Gabriel Gouvine - All Rights Reserved

#ifndef GBL_FLATVIEW_IMPL_HH
#define GBL_FLATVIEW_IMPL_HH

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

    FlatSize getNumFlatModules() const;
    FlatSize getNumFlatWires() const;
    FlatSize getNumFlatPorts() const;

    FlatSize getNumFlatInstanciations(Node node) const;
    FlatSize getNumFlatInstanciations(Wire wire) const;

    FlatModule getTop() const;

    FlatModule       getFlatModuleByIndex       (FlatSize index) const;
    FlatInstance     getFlatInstanceByIndex     (FlatSize index) const;
    FlatWire         getFlatWireByIndex         (FlatSize index) const;
    FlatModulePort   getFlatModulePortByIndex   (FlatSize index) const;
    FlatInstancePort getFlatInstancePortByIndex (FlatSize index) const;

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
    friend FlatNode;
    friend FlatWire;
    friend FlatPort;

private:
    Size getModIndex(FlatSize flatIndex) const;
    Size getWireModIndex(FlatSize flatIndex) const;
    Size getPortModIndex(FlatSize flatIndex) const;

    Size getModIndex(Module module) const;

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

namespace internal {
class FlatTransform {
    public:
    FlatNode         operator()(Node node)          { return FlatNode         (node, _ref); }
    FlatWire         operator()(Wire wire)          { return FlatWire         (wire, _ref); }
    FlatPort         operator()(Port port)          { return FlatPort         (port, _ref); }
    FlatModule       operator()(Module module)      { return FlatModule       (FlatNode(module, _ref)); }
    FlatInstance     operator()(Instance instance)  { return FlatInstance     (FlatNode(instance, _ref)); }
    FlatInstancePort operator()(InstancePort port)  { return FlatInstancePort (FlatPort(port, _ref)); }
    FlatModulePort   operator()(ModulePort port)    { return FlatModulePort   (FlatPort(port, _ref)); }

    FlatTransform(const FlatRef& ref) : _ref(ref) {}

    private:
    FlatRef _ref;
};
}

inline FlatWire::Ports
FlatWire::ports() {
    return getTransformContainer(getObject().ports(), internal::FlatTransform(_ref));
}

inline FlatNode::Ports
FlatNode::ports() {
    return getTransformContainer(getObject().ports(), internal::FlatTransform(_ref));
}

inline FlatModule::Ports
FlatModule::ports() {
    return getTransformContainer(getObject().ports(), internal::FlatTransform(_ref));
}

inline FlatInstance::Ports
FlatInstance::ports() {
    return getTransformContainer(getObject().ports(), internal::FlatTransform(_ref));
}

inline FlatModule::Wires
FlatModule::wires() {
    return getTransformContainer(getObject().wires(), internal::FlatTransform(_ref));
}

inline FlatModule::Instances
FlatModule::instances() {
    return getTransformContainer(getObject().instances(), internal::FlatTransform(_ref));
}

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
    return FlatModule(FlatNode(_topMod, FlatRef(0, *this)));
}

inline Wire FlatWire::getObject() { return _object; }
inline Node FlatNode::getObject() { return _object; }
inline Port FlatPort::getObject() { return _object; }
inline Instance FlatInstance::getObject() {
    return Instance(FlatNode::getObject());
}
inline Module FlatModule::getObject() {
    return Module(FlatNode::getObject());
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

inline FlatRef::FlatRef(FlatSize index, const FlatView& view) 
: _view(view)
, _index(index)
{
}
inline bool FlatRef::operator==(const FlatRef& o) const { return _index == o._index; }
inline bool FlatRef::operator!=(const FlatRef& o) const { return _index != o._index; }

inline FlatNode::FlatNode(const Node& object, const FlatRef& ref) : _object(object), _ref(ref) {}
inline FlatWire::FlatWire(const Wire& object, const FlatRef& ref) : _object(object), _ref(ref) {}
inline FlatPort::FlatPort(const Port& object, const FlatRef& ref) : _object(object), _ref(ref) {}

inline FlatModule::FlatModule(const Module& object, const FlatRef& ref) : FlatNode(object, ref) {}
inline FlatInstance::FlatInstance(const Instance& object, const FlatRef& ref) : FlatNode(object, ref) {}
inline FlatInstancePort::FlatInstancePort(const InstancePort& object, const FlatRef& ref) : FlatPort(object, ref) {}
inline FlatModulePort::FlatModulePort(const ModulePort& object, const FlatRef& ref) : FlatPort(object, ref) {}

inline FlatModule::FlatModule(const FlatNode& node) : FlatNode(node) {}
inline FlatInstance::FlatInstance(const FlatNode& node) : FlatNode(node) {}
inline FlatModulePort::FlatModulePort(const FlatPort& port) : FlatPort(port) {}
inline FlatInstancePort::FlatInstancePort(const FlatPort& port) : FlatPort(port) {}

inline FlatModule FlatNode::getParentModule() { return FlatModule(getObject().getParentModule(), _ref); }
inline FlatModule FlatWire::getParentModule() { return FlatModule(getObject().getParentModule(), _ref); }
inline FlatModule FlatPort::getParentModule() { return FlatModule(getObject().getParentModule(), _ref); }

inline bool FlatNode::isInstance() { return getObject().isInstance(); }
inline bool FlatNode::isModule() { return getObject().isModule(); }
inline bool FlatPort::isInstancePort() { return getObject().isInstancePort(); }
inline bool FlatPort::isModulePort() { return getObject().isModulePort(); }

inline bool FlatPort::isConnected() { return getObject().isConnected(); }
inline FlatWire FlatPort::getWire() { return FlatWire(getObject().getWire(), _ref); }
inline FlatNode FlatPort::getNode() { return FlatNode(getObject().getNode(), _ref); }
inline FlatInstance FlatInstancePort::getInstance() { return FlatInstance(getNode()); }

inline FlatInstance FlatModule::getUpInstance() {
    FlatView::ParentInfos info = _ref._view._parents[_ref._view.getModIndex(getObject())];
    Size index = bisectIndex(info._instEndIndexs, _ref._index);
    FlatView::UpInfo up = info._upInfos[index];
    assert(up._offset <= _ref._index);
    return FlatInstance(up._parentInstance, FlatRef(_ref._index - up._offset, _ref._view));
}

inline FlatModule FlatInstance::getDownModule() {
    FlatView::DownInfo down = _ref._view._children[_ref._view.getModIndex(getObject().getParentModule())]._downInfos[_object.ref()._ind];
    return FlatModule(getObject().getDownModule(), FlatRef(_ref._index + down._offset, _ref._view));
}

inline bool FlatModulePort::isTopPort() {
    return FlatPort::getParentModule().isTop();
}

inline FlatInstancePort FlatModulePort::getUpPort() {
    assert(!isTopPort());
    FlatInstance upInst = getParentModule().getUpInstance();
    return FlatInstancePort(
        getObject().getUpPort(upInst.getObject()),
        upInst._ref
    );
}

inline FlatModulePort FlatInstancePort::getDownPort() {
    FlatModule downMod = getInstance().getDownModule();
    return FlatModulePort(
        getObject().getDownPort(),
        downMod._ref
    );
}

inline FlatSize FlatNode::getIndex() {
    FlatModule repr = isInstance() ? FlatInstance(*this).getDownModule() : FlatModule(*this);
    return repr._ref._index + _ref._view._modEndIndexs[_ref._view.getModIndex(repr.getObject())];
}
inline FlatSize FlatWire::getIndex() {
    Size modInd = _ref._view.getModIndex(getObject().getParentModule());
    FlatSize numInst = _ref._view.getNumFlatInstanciations(modInd);
    assert(_ref._index < numInst);
    return _ref._view._wireEndIndexs[modInd] + numInst * _ref._view._wireHierToInternal[modInd][getObject().ref()._ind] + _ref._index;
}
inline FlatSize FlatPort::getIndex() {
    const FlatView& v = _ref._view;
    FlatModulePort repr = isInstancePort() ? FlatInstancePort(*this).getDownPort() : FlatModulePort(*this);
    Size modInd = v.getModIndex(repr.getObject().getParentModule());
    FlatSize numInst = v.getNumFlatInstanciations(modInd);
    assert(repr._ref._index < numInst);
    return v._portEndIndexs[modInd] + numInst * v._portHierToInternal[modInd][repr.getObject().ref()._portInd] + repr._ref._index;
}

inline FlatModule FlatView::getFlatModuleByIndex(FlatSize index) const {
    Size modInd = getModIndex(index);
    return FlatModule(Module(_mods[modInd]), FlatRef(index - _modEndIndexs[modInd], *this));
}
inline FlatInstance FlatView::getFlatInstanceByIndex(FlatSize index) const {
    return getFlatModuleByIndex(index).getUpInstance();
}

inline FlatWire FlatView::getFlatWireByIndex(FlatSize index) const {
    Size modInd = getWireModIndex(index);
    FlatSize num = getNumFlatInstanciations(modInd);
    FlatSize localIndex = index - _wireEndIndexs[modInd];
    return FlatWire(
        Wire(_mods[modInd], _wires[modInd][localIndex / num])
      , FlatRef(localIndex % num, *this)
    );
}
inline FlatModulePort FlatView::getFlatModulePortByIndex(FlatSize index) const {
    Size modInd = getPortModIndex(index);
    FlatSize num = getNumFlatInstanciations(modInd);
    FlatSize localIndex = index - _portEndIndexs[modInd];
    return FlatModulePort(
        ModulePort(Port(_mods[modInd], 0u, _ports[modInd][localIndex / num]))
      , FlatRef(localIndex % num, *this)
    );
}
inline FlatInstancePort FlatView::getFlatInstancePortByIndex(FlatSize index) const {
    return getFlatModulePortByIndex(index).getUpPort();
}

inline FlatSize FlatView::getNumFlatModules() const {
    return _modEndIndexs.back() - _modEndIndexs.front();
}
inline FlatSize FlatView::getNumFlatWires() const {
    return _wireEndIndexs.back() - _wireEndIndexs.front();
}
inline FlatSize FlatView::getNumFlatPorts() const {
    return _portEndIndexs.back() - _portEndIndexs.front();
}

inline bool FlatNode::hasName(ID id) { return getObject().hasName(id); }
inline bool FlatWire::hasName(ID id) { return getObject().hasName(id); }
inline bool FlatPort::hasName(ID id) { return getObject().hasName(id); }
inline bool FlatNode::hasProperty(ID id) { return getObject().hasProperty(id); }
inline bool FlatWire::hasProperty(ID id) { return getObject().hasProperty(id); }
inline bool FlatPort::hasProperty(ID id) { return getObject().hasProperty(id); }
inline Names FlatNode::names() { return getObject().names(); }
inline Names FlatWire::names() { return getObject().names(); }
inline Names FlatPort::names() { return getObject().names(); }
inline Properties FlatNode::properties() { return getObject().properties(); }
inline Properties FlatWire::properties() { return getObject().properties(); }
inline Properties FlatPort::properties() { return getObject().properties(); }

inline bool FlatNode::operator==(const FlatNode& o) const { return _object == o._object && _ref == o._ref; }
inline bool FlatNode::operator!=(const FlatNode& o) const { return !operator==(o); }
inline bool FlatWire::operator==(const FlatWire& o) const { return _object == o._object && _ref == o._ref; }
inline bool FlatWire::operator!=(const FlatWire& o) const { return !operator==(o); }
inline bool FlatPort::operator==(const FlatPort& o) const { return _object == o._object && _ref == o._ref; }
inline bool FlatPort::operator!=(const FlatPort& o) const { return !operator==(o); }


} // End namespace gbl

#endif

