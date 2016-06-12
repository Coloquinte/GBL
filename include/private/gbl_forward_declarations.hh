// Copyright (C) 2016 Gabriel Gouvine - All Rights Reserved

#ifndef GBL_FORWARD_DECLARATIONS_HH
#define GBL_FORWARD_DECLARATIONS_HH

#include <cstdint>

namespace gbl {

typedef std::uint32_t ID;
typedef std::uint32_t Size;
typedef std::uint64_t FlatSize;

const Size InvalidIndex = -1;
const FlatSize InvalidFlatIndex = -1;


// Main classes

class Module;
class Node;
class Instance;
class Wire;
class Port;
class InstancePort;
class ModulePort;

class FlatModule;
class FlatNode;
class FlatInstance;
class FlatWire;
class FlatPort;
class FlatInstancePort;
class FlatModulePort;

class FlatView;

// Implementation

namespace internal {
class ModuleImpl;
class WireImpl;
class NodeImpl;
}

// Iterators

template <class InputIterator, class Predicate>     class FilterIterator;
template <class InputIterator, class UnaryFunction> class TransformIterator;
template <class InputIterator> class Container;

namespace internal {
class EltRefInputIterator;
class PortRefInputIterator;

class WireFilter;
class NodeFilter;
class InstanceFilter;
class NodePortFilter;
class WirePortFilter;

class WireTransform;
class NodeTransform;
class InstanceTransform;
class ModPortTransform;
class InsPortTransform;
class NodePortRefTransform;
class WirePortRefTransform;

typedef TransformIterator<FilterIterator<PortRefInputIterator, WirePortFilter>, WirePortRefTransform> WirePortIterator;
typedef TransformIterator<FilterIterator<PortRefInputIterator, NodePortFilter>, NodePortRefTransform> NodePortIterator;
typedef TransformIterator<NodePortIterator, ModPortTransform> ModulePortIterator;
typedef TransformIterator<NodePortIterator, InsPortTransform> InstancePortIterator;

typedef TransformIterator<FilterIterator<EltRefInputIterator, WireFilter>, WireTransform> WireIterator;
typedef TransformIterator<FilterIterator<EltRefInputIterator, NodeFilter>, NodeTransform> NodeIterator;
typedef TransformIterator<FilterIterator<NodeIterator, InstanceFilter>, InstanceTransform> InstanceIterator;

class FlatTransform;

typedef TransformIterator<WireIterator,         FlatTransform> FlatWireIterator;
typedef TransformIterator<NodeIterator,         FlatTransform> FlatNodeIterator;
typedef TransformIterator<InstanceIterator,     FlatTransform> FlatInstanceIterator;
typedef TransformIterator<WirePortIterator,     FlatTransform> FlatWirePortIterator;
typedef TransformIterator<NodePortIterator,     FlatTransform> FlatNodePortIterator;
typedef TransformIterator<InstancePortIterator, FlatTransform> FlatInstancePortIterator;
typedef TransformIterator<ModulePortIterator,   FlatTransform> FlatModulePortIterator;

typedef const ID* NameIterator;
typedef const ID* PropertyIterator;
}

struct EltRef {
  bool operator==(const EltRef&) const;
  bool operator!=(const EltRef&) const;

  bool isValidNodeRef();
  bool isValidWireRef();

  EltRef();
  EltRef(internal::ModuleImpl *ptr, Size ind);

  internal::ModuleImpl *_ptr;
  Size _ind;
};

struct PortRef {
  bool operator==(const PortRef&) const;
  bool operator!=(const PortRef&) const;

  bool isValidNodePortRef();
  bool isValidWirePortRef();

  PortRef();
  PortRef(internal::ModuleImpl *ptr, Size instInd, Size portInd);

  internal::ModuleImpl *_ptr;
  Size _instInd;
  Size _portInd;
};

struct FlatEltRef {
  bool operator==(const FlatEltRef&) const;
  bool operator!=(const FlatEltRef&) const;

  const FlatView& _view;
  EltRef _ref;
  FlatSize _index;

  FlatEltRef(EltRef ref, FlatSize index, const FlatView& view);
};

struct FlatPortRef {
  bool operator==(const FlatPortRef&) const;
  bool operator!=(const FlatPortRef&) const;

  const FlatView& _view;
  PortRef _ref;
  FlatSize _index;

  FlatPortRef(PortRef ref, FlatSize index, const FlatView& view);
};

typedef Container<internal::NameIterator>     Names;
typedef Container<internal::PropertyIterator> Properties;

} // End namespace gbl


#endif

