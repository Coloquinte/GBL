// Copyright (C) 2016 Gabriel Gouvine - All Rights Reserved

#ifndef GBL_ITERATORS_HH
#define GBL_ITERATORS_HH

#include <iterator>

/************************************************************************
 * Implementation of iterators and containers
 ************************************************************************/

namespace gbl {

// This stuff is InputIterator-only: most notably, reference isn't necessarily value_type&
template <class InputIterator, class Predicate>
class FilterIterator {
    private:
    InputIterator _cur;
    InputIterator _end;
    Predicate _pred;

    public:
    typedef typename std::iterator_traits<InputIterator>::value_type value_type;
    typedef typename std::iterator_traits<InputIterator>::reference reference;
    typedef typename std::iterator_traits<InputIterator>::pointer pointer;
    typedef typename std::iterator_traits<InputIterator>::difference_type difference_type;
    typedef std::input_iterator_tag iterator_category;

    FilterIterator& operator++() {
        if (_cur != _end) {
            do {
                ++_cur;
            } while (_cur != _end && !_pred(*_cur));
        }
        return *this;
    }

    bool operator!=(const FilterIterator& it) const { return _cur != it._cur; }
    bool operator==(const FilterIterator& it) const { return _cur == it._cur; }

    typename InputIterator::reference operator*() {
        return *_cur;
    }
    FilterIterator(InputIterator cur=InputIterator(), InputIterator end=InputIterator(), Predicate pred=Predicate())
        : _cur(cur), _end(end), _pred(pred) {
        while(_cur != _end && !_pred(*_cur)) {
            ++_cur;
        }
    }
};

template <class InputIterator, class UnaryFunction>
class TransformIterator {
    private:
    InputIterator _it;
    UnaryFunction _func;

    public:
    typedef decltype(_func(*_it)) value_type;
    typedef value_type reference;
    typedef value_type* pointer;
    typedef typename std::iterator_traits<InputIterator>::difference_type difference_type;
    typedef std::input_iterator_tag iterator_category;

    TransformIterator& operator++() { ++_it; return *this; }
    bool operator!=(const TransformIterator& it) const { return _it != it._it; }
    bool operator==(const TransformIterator& it) const { return _it == it._it; }
    auto operator*() -> value_type {
        return _func(*_it);
    }
    TransformIterator(InputIterator it=InputIterator(), UnaryFunction func=UnaryFunction()) : _it(it), _func(func) {}
};

template<class InputIterator>
class Container {
    public:
    Container() {}
    Container(InputIterator b, InputIterator e) : _begin(b), _end(e) {}

    const InputIterator& begin () const { return _begin; }
    const InputIterator& end   () const { return _end; }

    typename std::iterator_traits<InputIterator>::difference_type size() const { return std::distance(_begin, _end); }

    private:
    InputIterator _begin;
    InputIterator _end;
};

namespace internal {
// Iterators for sequential traversal and filtering
class EltRefInputIterator : public EltRef {
    public:
    typedef EltRef value_type;
    typedef EltRef reference;
    typedef EltRef* pointer;
    typedef Size difference_type;
    typedef std::input_iterator_tag iterator_category;

    EltRefInputIterator& operator++() { ++_ind; return *this; }
    value_type operator*() { return *this; }
    EltRefInputIterator() {}
    EltRefInputIterator(const EltRef& n) : EltRef(n) {}
};

class PortRefInputIterator : public PortRef {
    public:
    typedef PortRef value_type;
    typedef PortRef reference;
    typedef PortRef* pointer;
    typedef Size difference_type;
    typedef std::input_iterator_tag iterator_category;

    PortRefInputIterator& operator++() { ++_portInd; return *this; }
    value_type operator*() { return *this; }
    PortRefInputIterator() {}
    PortRefInputIterator(const PortRef& p) : PortRef(p) {}
};

struct WireFilter { bool operator()(EltRef wire){ return wire.isValidWireRef(); } };
struct NodeFilter { bool operator()(EltRef node){ return node.isValidNodeRef(); } };
struct InstanceFilter { bool operator()(Node node){ return node.isInstance(); } };
struct NodePortFilter { bool operator()(PortRef port){ return port.isValidNodePortRef(); } };
struct WirePortFilter { bool operator()(PortRef port){ return port.isValidWirePortRef(); } };

struct WireTransform {
    Wire operator()(EltRef ref){ return Wire(ref._ptr, ref._ind); }
};
struct NodeTransform {
    Node operator()(EltRef ref){ return Node(ref._ptr, ref._ind); }
};
struct InstanceTransform {
    Instance operator()(Node node){ return Instance(node); }
};
struct ModPortTransform {
    ModulePort   operator()(Port port){ return ModulePort(port); }
};
struct InsPortTransform {
    InstancePort operator()(Port port){ return InstancePort(port); }
};
struct NodePortRefTransform {
    Port operator()(PortRef ref){ return Port(ref._ptr, ref._instInd, ref._portInd); }
};
struct WirePortRefTransform {
    Port operator()(PortRef ref){
        const Xref wref = ref._ptr->_wires[ref._instInd]._refs[ref._portInd];
        return Port(ref._ptr, wref._obj_id, wref._ind);
    }
};

class FlatTransform {
    public:
    FlatModule       operator()(Module module)      { return FlatModule       (FlatEltRef(module.ref()  , _index, _view)); }
    FlatNode         operator()(Node node)          { return FlatNode         (FlatEltRef(node.ref()    , _index, _view)); }
    FlatInstance     operator()(Instance instance)  { return FlatInstance     (FlatEltRef(instance.ref(), _index, _view)); }
    FlatWire         operator()(Wire wire)          { return FlatWire         (FlatEltRef(wire.ref()    , _index, _view)); }
    FlatPort         operator()(Port port)          { return FlatPort         (FlatPortRef(port.ref()   , _index, _view)); }
    FlatInstancePort operator()(InstancePort port)  { return FlatInstancePort (FlatPortRef(port.ref()   , _index, _view)); }
    FlatModulePort   operator()(ModulePort port)    { return FlatModulePort   (FlatPortRef(port.ref()   , _index, _view)); }

    FlatTransform(FlatSize index, const FlatView& view) : _index(index), _view(view) {}

    private:
    FlatSize _index;
    const FlatView& _view;
};

} // End namespace gbl::internal

template<class InputIterator, class UnaryFunction>
Container<TransformIterator<InputIterator, UnaryFunction> > getTransformContainer(InputIterator b, InputIterator e, UnaryFunction func) {
    typedef TransformIterator<InputIterator, UnaryFunction> Iterator;
    return Container<Iterator>(
        Iterator(b, func),
        Iterator(e, func)
    );
}

template<class InputIterator, class UnaryFunction>
Container<TransformIterator<InputIterator, UnaryFunction> > getTransformContainer(Container<InputIterator> full, UnaryFunction func) {
    return getTransformContainer(full.begin(), full.end(), func);
}

template<class InputIterator, class Predicate>
Container<FilterIterator<InputIterator, Predicate> > getFilterContainer(InputIterator b, InputIterator e, Predicate pred) {
    typedef FilterIterator<InputIterator, Predicate> Iterator;
    return Container<Iterator>(
        Iterator(b, e, pred),
        Iterator(e, e, pred)
    );
}

template<class InputIterator, class Predicate>
Container<FilterIterator<InputIterator, Predicate> > getFilterContainer(Container<InputIterator> full, Predicate pred) {
    return getFilterContainer(full.begin(), full.end(), pred);
}

inline Module::Wires
Module::wires() {
    return getTransformContainer(
        getFilterContainer(
            internal::EltRefInputIterator(EltRef(_ref._ptr, 0)),
            internal::EltRefInputIterator(EltRef(_ref._ptr, _ref._ptr->_wires.size())),
            internal::WireFilter()
        ), internal::WireTransform()
    );
}

inline Module::Nodes
Module::nodes() {
    return getTransformContainer(
        getFilterContainer(
            internal::EltRefInputIterator(EltRef(_ref._ptr, 0)),
            internal::EltRefInputIterator(EltRef(_ref._ptr, _ref._ptr->_nodes.size())),
            internal::NodeFilter()
        ), internal::NodeTransform()
    );
}

inline Module::Instances
Module::instances() {
    return getTransformContainer(
        getFilterContainer(nodes(), internal::InstanceFilter()),
        internal::InstanceTransform()
    );
}

inline Node::Ports
Node::ports() {
    return getTransformContainer(getFilterContainer(
        internal::PortRefInputIterator(PortRef(_ref._ptr, _ref._ind, 0)),
        internal::PortRefInputIterator(PortRef(_ref._ptr, _ref._ind, _ref._ptr->_nodes[_ref._ind]._instanciation->_nodes[0]._refs.size())),
        internal::NodePortFilter()
    ), internal::NodePortRefTransform());
}

inline Wire::Ports
Wire::ports() {
    return getTransformContainer(getFilterContainer(
        internal::PortRefInputIterator(PortRef(_ref._ptr, _ref._ind, 0)),
        internal::PortRefInputIterator(PortRef(_ref._ptr, _ref._ind, _ref._ptr->_wires[_ref._ind]._refs.size())),
        internal::WirePortFilter()
    ), internal::WirePortRefTransform());
}

inline Instance::Ports
Instance::ports() {
    return getTransformContainer(Node::ports(), internal::InsPortTransform());
}

inline Module::Ports
Module::ports() {
    return getTransformContainer(Node::ports(), internal::ModPortTransform());
}

inline Names Node::names() {
    const internal::DataImpl& data = _ref._ptr->_nodes[_ref._ind]._data;
    return Names(data.beginNames(), data.endNames());
}
inline Properties Node::properties() {
    const internal::DataImpl& data = _ref._ptr->_nodes[_ref._ind]._data;
    return Properties(data.beginProps(), data.endProps());
}
inline Names Wire::names() {
    const internal::DataImpl& data = _ref._ptr->_wires[_ref._ind]._data;
    return Names(data.beginNames(), data.endNames());
}
inline Properties Wire::properties() {
    const internal::DataImpl& data = _ref._ptr->_wires[_ref._ind]._data;
    return Properties(data.beginProps(), data.endProps());
}
inline Names Port::names() {
    const std::vector<internal::DataImpl>& refData = _ref._ptr->_nodes[_ref._instInd]._refData;
    if (refData.size() <= _ref._portInd) return Names(nullptr, nullptr);
    else return Names(refData[_ref._portInd].beginNames(), refData[_ref._portInd].endNames());
}
inline Properties Port::properties() {
    const std::vector<internal::DataImpl>& refData = _ref._ptr->_nodes[_ref._instInd]._refData;
    if (refData.size() <= _ref._portInd) return Properties(nullptr, nullptr);
    else return Names(refData[_ref._portInd].beginProps(), refData[_ref._portInd].endProps());
}

} // End namespace gbl

#endif



