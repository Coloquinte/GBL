
#ifndef GBL_ITERATORS_HH
#define GBL_ITERATORS_HH

#include <iterator>

/************************************************************************
 * Implementation of iterators and containers
 ************************************************************************/

namespace gbl {

// This stuff is InputIterator-only: most notably, reference_type isn't necessarily value_type&
template <class InputIterator, class Predicate>
class FilterIterator : std::input_iterator_tag {
    private:
    InputIterator _cur;
    InputIterator _end;
    Predicate _pred;

    public:
    typedef typename InputIterator::value_type value_type;
    typedef typename InputIterator::reference_type reference_type;

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

    typename InputIterator::reference_type operator*() {
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
class TransformIterator : std::input_iterator_tag {
    private:
    InputIterator _it;
    UnaryFunction _func;

    public:
    typedef decltype(_func(*_it)) value_type;
    typedef value_type reference_type;

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

    private:
    InputIterator _begin;
    InputIterator _end;
};

namespace internal {
// Iterators for sequential traversal and filtering
class NodeInputIterator : public Node, std::input_iterator_tag {
    public:
    typedef Node value_type;
    typedef Node reference_type;
    NodeInputIterator& operator++() { ++_ind; return *this; }
    value_type operator*() { return *this; }
    NodeInputIterator() {}
    NodeInputIterator(const Node& n) : Node(n) {}
};

class WireInputIterator : public Wire, std::input_iterator_tag {
    public:
    typedef Wire value_type;
    typedef Wire reference_type;
    WireInputIterator& operator++() { ++_ind; return *this; }
    value_type operator*() { return *this; }
    WireInputIterator() {}
    WireInputIterator(const Wire& w) : Wire(w) {}
};

class PortRefInputIterator : public PortRef, std::input_iterator_tag {
    public:
    typedef PortRef value_type;
    typedef PortRef reference_type;
    PortRefInputIterator& operator++() { ++_portInd; return *this; }
    value_type operator*() { return *this; }
    PortRefInputIterator() {}
    PortRefInputIterator(const PortRef& p) : PortRef(p) {}
};

struct WireFilter { bool operator()(Wire wire){ return wire.isValid(); } };
struct NodeFilter { bool operator()(Node node){ return node.isValid(); } };
struct InstanceFilter { bool operator()(Node node){ return node.isInstance(); } };
struct NodePortFilter { bool operator()(PortRef port){ return port.isValidNodePortRef(); } };
struct WirePortFilter { bool operator()(PortRef port){ return port.isValidWirePortRef(); } };

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

Module::Wires Module::wires() {
    return getFilterContainer(
        internal::WireInputIterator(Wire(_ptr, 0)),
        internal::WireInputIterator(Wire(_ptr, _ptr->_wires.size())),
        internal::WireFilter()
    );
}

Module::Nodes Module::nodes() {
    return getFilterContainer(
        internal::NodeInputIterator(Node(_ptr, 0)),
        internal::NodeInputIterator(Node(_ptr, _ptr->_nodes.size())),
        internal::NodeFilter()
    );
}

Module::Instances Module::instances() {
    return getTransformContainer(
        getFilterContainer(nodes(), internal::InstanceFilter()),
        internal::InstanceTransform()
    );
}

Node::Ports Node::ports() {
    return getTransformContainer(getFilterContainer(
        internal::PortRefInputIterator(PortRef(_ptr, _ind, 0)),
        internal::PortRefInputIterator(PortRef(_ptr, _ind, _ptr->_nodes[_ind]._instanciation->_nodes[0]._refs.size())),
        internal::NodePortFilter()
    ), internal::NodePortRefTransform());
}

Wire::Ports Wire::ports() {
    return getTransformContainer(getFilterContainer(
        internal::PortRefInputIterator(PortRef(_ptr, _ind, 0)),
        internal::PortRefInputIterator(PortRef(_ptr, _ind, _ptr->_wires[_ind]._refs.size())),
        internal::WirePortFilter()
    ), internal::WirePortRefTransform());
}

Instance::Ports Instance::ports() {
    return getTransformContainer(Node::ports(), internal::InsPortTransform());
}

} // End namespace gbl

#endif



