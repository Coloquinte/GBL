// Copyright (C) 2016 Gabriel Gouvine - All Rights Reserved

#ifndef GBL_FLATVIEW_HH
#define GBL_FLATVIEW_HH

#include "gbl.hh"
#include "private/gbl_flatview_forward_declarations.hh"

namespace gbl {

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

#include "private/gbl_flatview_impl.hh"

#endif

