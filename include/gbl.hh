// Copyright (C) 2016 Gabriel Gouvine - All Rights Reserved

#ifndef GBL_DB_HH
#define GBL_DB_HH

#include "private/gbl_forward_declarations.hh"

namespace gbl {

/************************************************************************
 * Proxy classes to access the database
 ************************************************************************/

class Wire {
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

  protected:
  EltRef _ref;

  friend Port;
};

class Node {
  public:
  typedef internal::NodePortIterator  PortIterator;
  typedef Container<PortIterator>     Ports;

  public:
  bool isModule() const;
  bool isInstance() const;
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

  protected:
  EltRef _ref;

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
  Module(const Module& module);
  explicit Module(const Node& n);
  Module& operator=(const Module & module);
  ~Module();
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

class Port {
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
  Wire getWire();

  bool isInstancePort();
  bool isModulePort();

  Port() {}
  Port(internal::ModuleImpl *ptr, Size instInd, Size portInd);

  bool operator==(const Port&) const;
  bool operator!=(const Port&) const;
  bool isValid();

  // Internal use
  PortRef ref() const;

  protected:
  PortRef _ref;
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

  ModulePort() {}
  explicit ModulePort(const Port& port);
};

/************************************************************************
 * Flat view of the database
 ************************************************************************/

class FlatWire {
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

  FlatWire(const Wire&, const FlatRef&);

  bool operator==(const FlatWire&) const;
  bool operator!=(const FlatWire&) const;

  Wire getObject();
  // Global index for the wire
  FlatSize getIndex();

  protected:
  Wire _object;
  FlatRef _ref;

  friend FlatPort;
};

class FlatNode {
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

  FlatNode(const Node&, const FlatRef&);

  bool operator==(const FlatNode&) const;
  bool operator!=(const FlatNode&) const;

  Node getObject();
  // Global index for the node; a FlatInstance and its corresponding FlatModule share the same
  FlatSize getIndex();

  protected:
  Node _object;
  FlatRef _ref;

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
  bool isTop();

  // Access
  Ports ports();
  Wires wires();
  Nodes nodes();
  Instances instances();

  FlatModule(const FlatNode&);
  FlatModule(const Module&, const FlatRef&);
  Module getObject();
};


class FlatInstance : public FlatNode {
  public:
  typedef internal::FlatInstancePortIterator PortIterator;
  typedef Container<PortIterator>            Ports;

  public:
  FlatModule getDownModule();

  // Access
  Ports ports();

  FlatInstance(const FlatNode& n);
  FlatInstance(const Instance&, const FlatRef&);
  Instance getObject();
};

class FlatPort {
  public:
  bool isConnected();

  bool hasName(ID id);
  bool hasProperty(ID id);

  Names names();
  Properties properties();

  FlatModule getParentModule();
  FlatNode getNode();
  FlatWire getWire();

  bool isInstancePort();
  bool isModulePort();

  FlatPort(const Port&, const FlatRef&);

  bool operator==(const FlatPort&) const;
  bool operator!=(const FlatPort&) const;

  Port getObject();
  // Global index for the port; a FlatInstancePort and its corresponding FlatModulePort share the same
  FlatSize getIndex();

  protected:
  Port _object;
  FlatRef _ref;
};

class FlatInstancePort : public FlatPort {
  public:
  FlatModulePort getDownPort();
  FlatInstance getInstance();

  FlatInstancePort(const FlatPort&);
  FlatInstancePort(const InstancePort&, const FlatRef&);
  InstancePort getObject();
};

class FlatModulePort : public FlatPort {
  public:
  FlatInstancePort getUpPort();
  bool isTopPort();

  FlatModulePort(const FlatPort&);
  FlatModulePort(const ModulePort&, const FlatRef&);
  ModulePort getObject();
};

} // End namespace gbl

#include "private/gbl_impl.hh"
#include "private/gbl_iterators_impl.hh"
#include "private/gbl_flatview.hh"

#endif

