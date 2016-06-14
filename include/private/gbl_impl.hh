// Copyright (C) 2016 Gabriel Gouvine - All Rights Reserved

#ifndef GBL_IMPL_HH
#define GBL_IMPL_HH

#include "data_impl.hh"

#include <vector>
#include <cassert>
#include <atomic>

namespace gbl {
namespace internal {

/************************************************************************
 * Helper classes
 *    * Cross-references to represent connections
 *    * Pools for allocation without iterator invalidation
 ************************************************************************/

// To mark unused refs and end of freelist
const Size EmptyInd = -1;
// To mark unconnected ports
const Size DisconnectedInd = -2;
// To mark used pool entries
const Size UsedInd = -2;

struct Xref {
  Xref(Size obj, Size ind) : _obj_id(obj), _ind(ind) {}
  bool isValid     () const { return _obj_id != EmptyInd; }
  bool isConnected () const { return _ind != EmptyInd; }

  static Xref Invalid      () { return Xref(EmptyInd, EmptyInd); }
  static Xref Disconnected () { return Xref(DisconnectedInd, EmptyInd); }

  Size _obj_id;
  Size _ind;
};

class XrefList {
  public:
  XrefList() : _freeList(EmptyInd) {}

  Size push() {
    Size ret;
    if(_freeList != EmptyInd) {
      ret = _freeList;
      _freeList = _refs[ret]._ind;
      _refs[ret] = Xref::Disconnected();
    }
    else {
      ret = _refs.size();
      _refs.emplace_back(Xref::Disconnected());
    }
    return ret;
  }

  void erase(Size ind) {
    assert(ind < _refs.size());
    _refs[ind]._obj_id = EmptyInd;
    _refs[ind]._ind = _freeList;
    _freeList = ind;
  }

  const Xref& operator[](Size ind) const {
    return _refs[ind];
  }
  Xref& operator[](Size ind) {
    return _refs[ind];
  }

  Size size() const { return _refs.size(); }

  private:
  std::vector<Xref> _refs;
  Size              _freeList;
};


template <typename T>
class Pool {
  private:
  // Element + free list + free information
  // _nextFree is UsedInd if allocated, EmptyInd or the index of the next free element if allocated
  struct Elt {
    T    _val;
    Size _nextFree;
  };

  public:
  Pool() {
    _freeList = EmptyInd;
  }
  bool isValid(Size ind) {
    return ind < _data.size() && _data[ind]._nextFree == UsedInd;
  }
  Size allocate() {
    if (_freeList != EmptyInd) {
      Size newAlloc = _freeList;
      _freeList = _data[newAlloc]._nextFree;
      return newAlloc;
    }
    else {
      Size newAlloc = _data.size();
      _data.emplace_back();
      _data.back()._nextFree = UsedInd;
      return newAlloc;
    }
  }
  void deallocate(Size ind) {
    assert(_data[ind]._nextFree == UsedInd);
    _data[ind]._val = T();
    _data[ind]._nextFree = _freeList;
    _freeList = ind;
  }
  T& operator[](Size ind) {
    assert(isValid(ind));
    return _data[ind]._val;
  }
  Size size() const { return _data.size(); }

  private:
  std::vector<Elt> _data;
  Size _freeList;
};

/************************************************************************
 * Core classes: storage of wires, modules and instances
 ************************************************************************/

struct BaseImpl {
  DataImpl _data;
};

struct NodeImpl : public BaseImpl {
  // Cross-references for the connections: no freelist allocation since the ports are linked to module ports
  std::vector<Xref>     _refs;
  // Data for the connections
  std::vector<DataImpl> _refData;
  ModuleImpl* _instanciation;

  NodeImpl();
};

struct WireImpl : public BaseImpl {
  // Cross-references for the connections, with an embedded freelist
  XrefList _refs;
};

struct ModuleImpl {
  Pool<NodeImpl> _nodes;
  Pool<WireImpl> _wires;

  std::atomic<std::uint64_t> _refcnt;

  Size _firstFreePort;
  bool _leaf;

  ModuleImpl(bool leaf);
};

inline
ModuleImpl::ModuleImpl(bool leaf)
: _refcnt(0)
, _firstFreePort(EmptyInd)
, _leaf(leaf)
{
    Size interfaceInd = _nodes.allocate();
    assert(interfaceInd == 0);
    _nodes[0]._instanciation = this;
}

inline
NodeImpl::NodeImpl()
: _instanciation(nullptr)
{
}

} // End namespace gbl::internal

/************************************************************************
 * Implementation of the main API
 ************************************************************************/

inline
Module::Module(internal::ModuleImpl* mod)
: Node(mod, 0)
{
    ++_ref._ptr->_refcnt;
}

inline
Module::Module(const Module& module)
: Node(module)
{
    ++_ref._ptr->_refcnt;
}

inline Module &
Module::operator=(const Module& module) {
    ++module._ref._ptr->_refcnt;
    if (_ref._ptr != nullptr) {
        if(--_ref._ptr->_refcnt == 0) {
            delete _ref._ptr;
        }
    }
    _ref._ptr = module._ref._ptr;
    return *this;
}

inline
Module::~Module() {
    if(--_ref._ptr->_refcnt == 0) {
        delete _ref._ptr;
    }
}

inline Module
Module::createHier() {
    return Module(new internal::ModuleImpl(false));
}

inline Module
Module::createLeaf() {
    return Module(new internal::ModuleImpl(true));
}

inline bool Module::isLeaf() { return  _ref._ptr->_leaf; }
inline bool Module::isHier() { return !_ref._ptr->_leaf; }

inline Wire
Module::createWire() {
    return Wire(_ref._ptr, _ref._ptr->_wires.allocate());
}

inline Instance
Module::createInstance(Module instanciated) {
    Size ind = _ref._ptr->_nodes.allocate();
    _ref._ptr->_nodes[ind]._instanciation = instanciated._ref._ptr;
    return Instance(Node(_ref._ptr, ind));
}

inline ModulePort
Module::createPort() {
    Size newPortInd;
    if (_ref._ptr->_firstFreePort == internal::EmptyInd) {
        newPortInd = _ref._ptr->_nodes[0]._refs.size();
        _ref._ptr->_nodes[0]._refs.push_back(internal::Xref::Disconnected());
    }
    else {
        newPortInd = _ref._ptr->_firstFreePort;
        _ref._ptr->_firstFreePort = _ref._ptr->_nodes[0]._refs[newPortInd]._ind;
        _ref._ptr->_nodes[0]._refs[newPortInd] = internal::Xref::Disconnected();
    }
    return ModulePort(Port(_ref._ptr, 0, newPortInd));
}

inline EltRef::EltRef() : _ptr(nullptr), _ind(-1) {}
inline EltRef::EltRef(internal::ModuleImpl *ptr, Size ind) : _ptr(ptr), _ind(ind) {}
inline Wire::Wire(internal::ModuleImpl *ptr, Size ind) : _ref(ptr, ind) {}
inline Node::Node(internal::ModuleImpl *ptr, Size ind) : _ref(ptr, ind) {}
inline EltRef Node::ref() const { return _ref; }
inline EltRef Wire::ref() const { return _ref; }
inline FlatEltRef FlatNode::ref() const { return _ref; }
inline FlatEltRef FlatWire::ref() const { return _ref; }

inline Instance::Instance(const Node& node) : Node(node) { assert(isInstance()); }

inline PortRef::PortRef() : _ptr(nullptr), _instInd(-1), _portInd(-1) {}
inline PortRef::PortRef(internal::ModuleImpl *ptr, Size instInd, Size portInd) : _ptr(ptr), _instInd(instInd), _portInd(portInd) {}
inline Port::Port(internal::ModuleImpl *ptr, Size instInd, Size portInd) : _ref(ptr, instInd, portInd) {}
inline PortRef Port::ref() const { return _ref; }
inline FlatPortRef FlatPort::ref() const { return _ref; }

inline ModulePort::ModulePort(const Port& port) : Port(port) { assert(isModulePort()); }
inline InstancePort::InstancePort(const Port& port) : Port(port) { assert(isInstancePort()); }

inline bool Node::isModule   () { return _ref._ind == 0; }
inline bool Node::isInstance () { return _ref._ind != 0; }
inline bool Port::isModulePort () { return getNode().isModule(); }
inline bool Port::isInstancePort () { return getNode().isInstance(); }

inline Module Wire::getParentModule() { return Module(_ref._ptr); }
inline Module Node::getParentModule() { return Module(_ref._ptr); }
inline Module Port::getParentModule() { return Module(_ref._ptr); }
inline Node Port::getNode() { return Node(_ref._ptr, _ref._instInd); }
inline Instance InstancePort::getInstance() { return Instance(getNode()); }
inline Module Instance::getDownModule() { return _ref._ptr->_nodes[_ref._ind]._instanciation; }

inline ModulePort
InstancePort::getDownPort() {
    assert(isValid());
    return ModulePort(Port(
        _ref._ptr->_nodes[_ref._instInd]._instanciation,
        0,
        _ref._portInd
    ));
}

inline InstancePort
ModulePort::getUpPort(Instance inst) {
    assert(isValid());
    return InstancePort(Port(
        inst._ref._ptr,
        inst._ref._ind,
        _ref._portInd
    ));
}

inline bool
Port::isConnected() {
    assert(isValid());
    std::vector<internal::Xref> const & refvec = _ref._ptr->_nodes[_ref._instInd]._refs;
    if (refvec.size() <= _ref._portInd) {
        return false;
    }
    else {
        // May be invalid for a disconnected reference port
        return refvec[_ref._portInd].isValid() && refvec[_ref._portInd].isConnected();
    }
}

inline void
Port::connect(Wire wire) {
    assert(isValid());
    assert(!isConnected());
    assert(wire._ref._ptr == _ref._ptr);
    Size wirePortInd = _ref._ptr->_wires[wire._ref._ind]._refs.push();

    // In an instance, the crossref vector may not have this port yet, although the module has it: resize it if necessary
    std::vector<internal::Xref>& refvec = _ref._ptr->_nodes[_ref._instInd]._refs;
    if (refvec.size() <= _ref._portInd) {
        assert(isInstancePort());
        // Allocate to match the module's port width
        refvec.resize(_ref._portInd+1, internal::Xref(internal::EmptyInd, internal::EmptyInd));
    }
    internal::Xref& ref = refvec[_ref._portInd];
    ref._obj_id = wire._ref._ind;
    ref._ind    = wirePortInd;

    internal::Xref& wireRef = _ref._ptr->_wires[wire._ref._ind]._refs[wirePortInd];
    wireRef._obj_id = _ref._instInd;
    wireRef._ind    = _ref._portInd;
}

inline void
Port::disconnect() {
    assert(isValid());
    assert(isConnected());
    internal::Xref& instRef = _ref._ptr->_nodes[_ref._instInd]._refs[_ref._portInd];
    _ref._ptr->_wires[instRef._obj_id]._refs.erase(instRef._ind);
    instRef = internal::Xref::Disconnected();
}

inline void
Node::disconnectAll() {
    assert(isValid());
    std::vector<internal::Xref>& nodeRefs = _ref._ptr->_nodes[_ref._ind]._refs;
    for (Size i=0; i<nodeRefs.size(); ++i) {
        if (nodeRefs[i].isValid() && nodeRefs[i].isConnected()) {
            _ref._ptr->_wires[nodeRefs[i]._obj_id]._refs.erase(nodeRefs[i]._ind);
            nodeRefs[i] = internal::Xref::Disconnected(); // Works for both instances and modules (but for an instance we could just deallocate/invalidate everything)
        }
    }
}

inline void
Wire::disconnectAll() {
    assert(isValid());
    internal::XrefList& wireRefs = _ref._ptr->_wires[_ref._ind]._refs; 
    for (Size i=0; i<wireRefs.size(); ++i) {
        if (wireRefs[i].isConnected() && wireRefs[i].isValid()) {
            _ref._ptr->_nodes[wireRefs[i]._obj_id]._refs[wireRefs[i]._ind] = internal::Xref::Disconnected();
            wireRefs.erase(i);
        }
    }
}


inline void
Instance::destroy() {
    assert(isValid());
    disconnectAll();
    _ref._ptr->_nodes.deallocate(_ref._ind);
}

inline void
Wire::destroy() {
    assert(isValid());
    disconnectAll();
    _ref._ptr->_wires.deallocate(_ref._ind);
}

inline void
ModulePort::destroy() {
    // TODO: have some refcounting mecanism
    _ref._ptr->_nodes[0]._refs[_ref._portInd] = internal::Xref::Invalid();
    _ref._ptr->_nodes[0]._refs[_ref._portInd]._ind = _ref._ptr->_firstFreePort;
    _ref._ptr->_firstFreePort = _ref._portInd;
}

inline bool EltRef::operator==(const EltRef& o) const { return _ptr == o._ptr && _ind == o._ind; }
inline bool EltRef::operator!=(const EltRef& o) const { return _ptr != o._ptr || _ind != o._ind; }
inline bool Node::operator==(const Node& o) const { return _ref == o._ref; }
inline bool Node::operator!=(const Node& o) const { return _ref != o._ref; }
inline bool Wire::operator==(const Wire& o) const { return _ref == o._ref; }
inline bool Wire::operator!=(const Wire& o) const { return _ref != o._ref; }

inline bool PortRef::operator==(const PortRef& o) const { return _ptr == o._ptr && _instInd == o._instInd && _portInd == o._portInd; }
inline bool PortRef::operator!=(const PortRef& o) const { return _ptr != o._ptr || _instInd != o._instInd || _portInd != o._portInd; }
inline bool Port::operator==(const Port& o) const { return _ref == o._ref; }
inline bool Port::operator!=(const Port& o) const { return _ref != o._ref; }

inline bool FlatEltRef::operator==(const FlatEltRef& o) const { return _ref == o._ref && _index == o._index; }
inline bool FlatEltRef::operator!=(const FlatEltRef& o) const { return _ref != o._ref || _index != o._index; }
inline bool FlatNode::operator==(const FlatNode& o) const { return _ref == o._ref; }
inline bool FlatNode::operator!=(const FlatNode& o) const { return _ref != o._ref; }
inline bool FlatWire::operator==(const FlatWire& o) const { return _ref == o._ref; }
inline bool FlatWire::operator!=(const FlatWire& o) const { return _ref != o._ref; }

inline bool FlatPortRef::operator==(const FlatPortRef& o) const { return _ref == o._ref && _index == o._index; }
inline bool FlatPortRef::operator!=(const FlatPortRef& o) const { return _ref != o._ref || _index != o._index; }
inline bool FlatPort::operator==(const FlatPort& o) const { return _ref == o._ref; }
inline bool FlatPort::operator!=(const FlatPort& o) const { return _ref != o._ref; }

inline bool EltRef::isValidWireRef() {
    return _ptr != nullptr
        && _ptr->_wires.isValid(_ind);
}
inline bool EltRef::isValidNodeRef() {
    return _ptr != nullptr
        && _ptr->_nodes.isValid(_ind);
}
inline bool PortRef::isValidNodePortRef() {
    return _ptr != nullptr
        && _ptr->_nodes.isValid(_instInd)
        && _ptr->_nodes[_instInd]._instanciation->_nodes[0]._refs[_portInd].isValid();
}
inline bool PortRef::isValidWirePortRef() {
    return _ptr != nullptr
        && _ptr->_wires.isValid(_instInd)
        && _ptr->_wires[_instInd]._refs[_portInd].isValid();
}

inline bool Wire::isValid() { return _ref.isValidWireRef(); }
inline bool Node::isValid() { return _ref.isValidNodeRef(); }
inline bool Port::isValid() { return _ref.isValidNodePortRef(); }

inline bool Wire::hasName(ID id) {
    assert(isValid());
    return _ref._ptr->_wires[_ref._ind]._data.hasName(id);
}
inline bool Wire::hasProperty(ID id) {
    assert(isValid());
    return _ref._ptr->_wires[_ref._ind]._data.hasProp(id);
}
inline bool Wire::addName(ID id) {
    assert(isValid());
    return _ref._ptr->_wires[_ref._ind]._data.addName(id);
}
inline bool Wire::addProperty(ID id) {
    assert(isValid());
    return _ref._ptr->_wires[_ref._ind]._data.addProp(id);
}
inline bool Wire::eraseName(ID id) {
    assert(isValid());
    return _ref._ptr->_wires[_ref._ind]._data.eraseName(id);
}
inline bool Wire::eraseProperty(ID id) {
    assert(isValid());
    return _ref._ptr->_wires[_ref._ind]._data.eraseProp(id);
}

inline bool Node::hasName(ID id) {
    assert(isValid());
    return _ref._ptr->_nodes[_ref._ind]._data.hasName(id);
}
inline bool Node::hasProperty(ID id) {
    assert(isValid());
    return _ref._ptr->_nodes[_ref._ind]._data.hasProp(id);
}
inline bool Node::addName(ID id) {
    assert(isValid());
    return _ref._ptr->_nodes[_ref._ind]._data.addName(id);
}
inline bool Node::addProperty(ID id) {
    assert(isValid());
    return _ref._ptr->_nodes[_ref._ind]._data.addProp(id);
}
inline bool Node::eraseName(ID id) {
    assert(isValid());
    return _ref._ptr->_nodes[_ref._ind]._data.eraseName(id);
}
inline bool Node::eraseProperty(ID id) {
    assert(isValid());
    return _ref._ptr->_nodes[_ref._ind]._data.eraseProp(id);
}

inline bool Port::hasName(ID id) {
    assert(isValid());
    if (_ref._ptr->_nodes[_ref._instInd]._refData.size() <= _ref._portInd) return false;
    return _ref._ptr->_nodes[_ref._instInd]._refData[_ref._portInd].hasName(id);
}
inline bool Port::hasProperty(ID id) {
    assert(isValid());
    if (_ref._ptr->_nodes[_ref._instInd]._refData.size() <= _ref._portInd) return false;
    return _ref._ptr->_nodes[_ref._instInd]._refData[_ref._portInd].hasProp(id);
}
inline bool Port::addName(ID id) {
    assert(isValid());
    if (_ref._ptr->_nodes[_ref._instInd]._refData.size() <= _ref._portInd) {
        _ref._ptr->_nodes[_ref._instInd]._refData.resize(_ref._portInd+1);
    }
    return _ref._ptr->_nodes[_ref._instInd]._refData[_ref._portInd].addName(id);
}
inline bool Port::addProperty(ID id) {
    assert(isValid());
    if (_ref._ptr->_nodes[_ref._instInd]._refData.size() <= _ref._portInd) {
        _ref._ptr->_nodes[_ref._instInd]._refData.resize(_ref._portInd+1);
    }
    return _ref._ptr->_nodes[_ref._instInd]._refData[_ref._portInd].addProp(id);
}
inline bool Port::eraseName(ID id) {
    assert(isValid());
    if (_ref._ptr->_nodes[_ref._instInd]._refData.size() <= _ref._portInd) return false;
    return _ref._ptr->_nodes[_ref._instInd]._refData[_ref._portInd].eraseName(id);
}
inline bool Port::eraseProperty(ID id) {
    assert(isValid());
    if (_ref._ptr->_nodes[_ref._instInd]._refData.size() <= _ref._portInd) return false;
    return _ref._ptr->_nodes[_ref._instInd]._refData[_ref._portInd].eraseProp(id);
}

} // End namespace gbl

#endif







