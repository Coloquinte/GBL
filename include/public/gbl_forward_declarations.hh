
#ifndef GBL_FORWARD_DECLARATIONS_HH
#define GBL_FORWARD_DECLARATIONS_HH

namespace gbl {

// Main classes

class Module;
class Wire;
class Instance;
class Port;
class InstancePort;
class ModulePort;

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
class NodeInputIterator;
class WireInputIterator;
class PortRefInputIterator;

class WireFilter;
class NodeFilter;
class InstanceFilter;
class NodePortFilter;
class WirePortFilter;

class InstanceTransform;
class ModPortTransform;
class InsPortTransform;
class NodePortRefTransform;
class WirePortRefTransform;

typedef TransformIterator<FilterIterator<PortRefInputIterator, WirePortFilter>, WirePortRefTransform> WirePortIterator;
typedef TransformIterator<FilterIterator<PortRefInputIterator, NodePortFilter>, NodePortRefTransform> NodePortIterator;
typedef TransformIterator<NodePortIterator, ModPortTransform> ModulePortIterator;
typedef TransformIterator<NodePortIterator, InsPortTransform> InstancePortIterator;

typedef FilterIterator<WireInputIterator, WireFilter> WireIterator;
typedef FilterIterator<NodeInputIterator, NodeFilter> NodeIterator;
typedef TransformIterator<FilterIterator<NodeIterator, InstanceFilter>, InstanceTransform> InstanceIterator;
}

} // End namespace gbl


#endif

