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
} // End namespace gbl

#include "private/gbl_impl.hh"
#include "private/gbl_iterators_impl.hh"

#endif

