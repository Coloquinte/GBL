// Copyright (C) 2016 Gabriel Gouvine - All Rights Reserved

#ifndef GBL_DB_HH
#define GBL_DB_HH

#include "private/gbl_forward_declarations.hh"

namespace gbl {

/************************************************************************
 * Proxy classes to access the database
 ************************************************************************/

class Wire : protected EltRef {
  public:
  typedef internal::WirePortIterator  PortIterator;
  typedef Container<PortIterator>     Ports;

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

  Names names();
  Properties properties();

  Wire() {}
  Wire(internal::ModuleImpl *ptr, Size ind);

  bool operator==(const Wire&) const;
  bool operator!=(const Wire&) const;
  bool isValid();

  // Internal use
  EltRef ref() const;
  friend Port;
};

class Node : protected EltRef {
  public:
  typedef internal::NodePortIterator  PortIterator;
  typedef Container<PortIterator>     Ports;

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

  Names names();
  Properties properties();

  Node() {}
  Node(internal::ModuleImpl *ptr, Size ind);

  bool operator==(const Node&) const;
  bool operator!=(const Node&) const;
  bool isValid();

  // Internal use
  EltRef ref() const;
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
  typedef Container<PortIterator>        Ports;

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

  Names names();
  Properties properties();

  Module getParentModule();
  Node getNode();
  bool isInstancePort();
  bool isModulePort();

  Port() {}
  Port(internal::ModuleImpl *ptr, Size instInd, Size portInd);

  bool operator==(const Port&) const;
  bool operator!=(const Port&) const;
  bool isValid();

  // Internal use
  PortRef ref() const;
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

/************************************************************************
 * Flat view of the database
 ************************************************************************/

class FlatWire : protected FlatEltRef {
  public:
  typedef internal::FlatWirePortIterator  PortIterator;
  typedef Container<PortIterator>         Ports;

  public:
  FlatModule getParentModule();

  // Access
  Ports ports();

  bool hasName(ID id);
  bool hasProperty(ID id);

  Names names();
  Properties properties();

  explicit FlatWire(const FlatEltRef&);

  bool operator==(const FlatWire&) const;
  bool operator!=(const FlatWire&) const;
  bool isValid();

  // Internal use
  FlatEltRef ref() const;
  friend FlatPort;
};

class FlatNode : protected FlatEltRef {
  public:
  typedef internal::FlatNodePortIterator  PortIterator;
  typedef Container<PortIterator>         Ports;

  public:
  bool isModule();
  bool isInstance();
  FlatModule getParentModule();

  // Access
  Ports ports();

  bool hasName(ID id);
  bool hasProperty(ID id);

  Names names();
  Properties properties();

  explicit FlatNode(const FlatEltRef&);

  bool operator==(const FlatNode&) const;
  bool operator!=(const FlatNode&) const;
  bool isValid();

  // Internal use
  FlatEltRef ref() const;
  friend FlatModulePort;
  friend FlatInstancePort;
};

class FlatModule : public FlatNode {
  public:
  typedef internal::FlatWireIterator       WireIterator;
  typedef internal::FlatNodeIterator       NodeIterator;
  typedef internal::FlatInstanceIterator   InstanceIterator;
  typedef internal::FlatModulePortIterator PortIterator;

  typedef Container<WireIterator>     Wires;
  typedef Container<NodeIterator>     Nodes;
  typedef Container<InstanceIterator> Instances;
  typedef Container<PortIterator>     Ports;

  public:
  FlatInstance getUpInstance();

  bool isLeaf();
  bool isHier();

  // Access
  Ports ports();
  Wires wires();
  Nodes nodes();
  Instances instances();

  bool operator==(const FlatModule&) const;
  bool operator!=(const FlatModule&) const;

  explicit FlatModule(const FlatNode&);
  explicit FlatModule(const FlatEltRef&);
};


class FlatInstance : public FlatNode {
  public:
  typedef internal::FlatInstancePortIterator PortIterator;
  typedef Container<PortIterator>            Ports;

  public:
  FlatModule getDownModule();

  // Access
  Ports ports();

  explicit FlatInstance(const FlatNode& n);
  explicit FlatInstance(const FlatEltRef&);
};

class FlatPort : protected FlatPortRef {
  public:
  bool isConnected();

  bool hasName(ID id);
  bool hasProperty(ID id);

  Names names();
  Properties properties();

  FlatModule getParentModule();
  FlatNode getNode();
  bool isInstancePort();
  bool isModulePort();

  bool operator==(const FlatPort&) const;
  bool operator!=(const FlatPort&) const;
  bool isValid();

  // Internal use
  FlatPortRef ref() const;

  explicit FlatPort(const FlatPortRef&);
};

class FlatInstancePort : public FlatPort {
  public:
  FlatModulePort getDownPort();
  FlatInstance getInstance();

  explicit FlatInstancePort(const FlatPort&);
  explicit FlatInstancePort(const FlatPortRef&);
};

class FlatModulePort : public FlatPort {
  public:
  FlatInstancePort getUpPort();

  explicit FlatModulePort(const FlatPort&);
  explicit FlatModulePort(const FlatPortRef&);
};

} // End namespace gbl

#include "private/gbl_impl.hh"
#include "private/gbl_iterators_impl.hh"

#endif

