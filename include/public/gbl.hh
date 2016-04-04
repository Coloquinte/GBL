// Header for GBL's C++ interface
// Entirely a proxy for the C interface

#ifndef GBL_DB_HH
#define GBL_DB_HH

#include "gbl_forward_declarations.hh"

namespace gbl {

/************************************************************************
 * Classes to be used internally to reference the storage
 ************************************************************************/
class Wire : protected EltRef {
  public:
  typedef internal::WirePortIterator PortIterator;
  typedef Container<PortIterator>    Ports;

  public:
  void disconnectAll();
  void destroy();
  Module getParentModule();

  // Access
  Ports ports();

  bool hasName(ID id);
  bool hasProperty(ID id);
  bool addName(ID id);
  bool addProperty(ID id);
  bool eraseName(ID id);
  bool eraseProperty(ID id);

  Wire() {}
  Wire(internal::ModuleImpl *ptr, Size ind);

  bool operator==(const Wire&) const;
  bool operator!=(const Wire&) const;
  bool isValid();

  friend Port;
};

class Node : protected EltRef {
  public:
  typedef internal::NodePortIterator PortIterator;
  typedef Container<PortIterator>    Ports;

  public:
  bool isModule();
  bool isInstance();
  Module getParentModule();

  void disconnectAll();

  // Access
  Ports ports();

  bool hasName(ID id);
  bool hasProperty(ID id);
  bool addName(ID id);
  bool addProperty(ID id);
  bool eraseName(ID id);
  bool eraseProperty(ID id);

  Node() {}
  Node(internal::ModuleImpl *ptr, Size ind);

  bool operator==(const Node&) const;
  bool operator!=(const Node&) const;
  bool isValid();

  friend ModulePort;
  friend InstancePort;
};

class Module : public Node {
  public:
  typedef internal::WireIterator       WireIterator;
  typedef internal::NodeIterator       NodeIterator;
  typedef internal::InstanceIterator   InstanceIterator;
  typedef internal::ModulePortIterator PortIterator;

  typedef Container<WireIterator>     Wires;
  typedef Container<NodeIterator>     Nodes;
  typedef Container<InstanceIterator> Instances;
  typedef Container<PortIterator>     Ports;

  public:
  static Module createHier();
  static Module createLeaf();

  bool isLeaf();
  bool isHier();
  ModulePort createPort();

  // Only for non-leaf modules
  Wire     createWire();
  Instance createInstance(Module instanciated);

  // Access
  Ports ports();
  Wires wires();
  Nodes nodes();
  Instances instances();

  Module() {}
  Module(internal::ModuleImpl* ptr);
  Module(const Module & module);
  Module& operator=(const Module & module);
  ~Module();

  bool operator==(const Module&) const;
  bool operator!=(const Module&) const;
};


class Instance : public Node {
  public:
  typedef internal::InstancePortIterator PortIterator;
  typedef Container<PortIterator> Ports;

  public:
  Module getDownModule();

  void destroy();

  // Access
  Ports ports();

  void replaceModule(Module mod);

  Instance() {}
  explicit Instance(const Node& n);
};

class Port : protected PortRef {
  public:
  bool isConnected();
  void disconnect();
  void connect(Wire wire);

  bool hasName(ID id);
  bool hasProperty(ID id);
  bool addName(ID id);
  bool addProperty(ID id);
  bool eraseName(ID id);
  bool eraseProperty(ID id);

  Module getParentModule();
  Node getNode();
  bool isInstancePort();
  bool isModulePort();

  Port() {}
  Port(internal::ModuleImpl *ptr, Size instInd, Size portInd);

  bool operator==(const Port&) const;
  bool operator!=(const Port&) const;
  bool isValid();
};

class InstancePort : public Port {
  public:
  ModulePort getDownPort();
  Instance getInstance();

  InstancePort() {}
  explicit InstancePort(const Port& port);
};

class ModulePort : public Port {
  public:
  void destroy();

  InstancePort getUpPort(Instance inst);

  void resetDir();
  void addDirIn();
  void addDirOut();

  ModulePort() {}
  explicit ModulePort(const Port& port);
};

} // End namespace gbl

#include "gbl_impl.hh"
#include "gbl_iterators_impl.hh"

#endif

