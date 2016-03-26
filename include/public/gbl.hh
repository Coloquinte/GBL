// Header for GBL's C++ interface
// Entirely a proxy for the C interface

#ifndef GBL_DB_HH
#define GBL_DB_HH

#include "gbl_common.hh"

namespace gbl {

/************************************************************************
 * Classes to be used internally to reference the storage
 ************************************************************************/

namespace internal {
class ModuleImpl;
}

class Module;
class Wire;
class Instance;
class Port;
class InstancePort;
class ModulePort;

class Module {
  public:
  // Forward containers
  class Ports;
  class Wires;
  class Nodes;
  class Instances;

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

  Module();
  Module(internal::ModuleImpl* ptr);
  Module(const Module & module);
  Module& operator=(const Module & module);
  ~Module();

  bool operator==(const Module&) const;
  bool operator!=(const Module&) const;
  bool isValid();

  protected:
  internal::ModuleImpl *_ptr;
};

class Wire {
  public:
  class Ports;

  public:
  void disconnectAll();
  void destroy();
  Module getParentModule();

  // Access
  Ports ports();

  Wire();
  Wire(internal::ModuleImpl *ptr, Size ind);

  bool operator==(const Wire&) const;
  bool operator!=(const Wire&) const;
  bool isValid();

  protected:
  internal::ModuleImpl *_ptr;
  Size _ind;

  friend Port;
};

class Node {
  public:
  class Ports;

  public:
  bool isModule();
  bool isInstance();
  Module getParentModule();

  void disconnectAll();

  // Access
  Ports ports();

  Node();
  Node(internal::ModuleImpl *ptr, Size ind);

  bool operator==(const Node&) const;
  bool operator!=(const Node&) const;
  bool isValid();

  protected:
  internal::ModuleImpl *_ptr;
  Size _ind;

  friend ModulePort;
  friend InstancePort;
};

class Instance : public Node {
  public:
  class Ports;

  public:
  Module getDownModule();

  void destroy();

  // Access
  Ports ports();

  void replaceModule(Module mod);

  Instance();
  Instance(const Node& n);
};

class Port {
  public:
  bool isConnected();
  void disconnect();
  void connect(Wire wire);

  // Direction information
  bool isReader();
  bool isDriver();
  bool isIn();
  bool isOut();
  bool isInOut();

  Module getParentModule();
  Node getNode();
  bool isInstancePort();
  bool isModulePort();

  Port();
  Port(internal::ModuleImpl *ptr, Size instInd, Size portInd);

  bool operator==(const Port&) const;
  bool operator!=(const Port&) const;
  bool isValid();

  protected:
  internal::ModuleImpl *_ptr;
  Size _instInd;
  Size _portInd;
};

class InstancePort : public Port {
  public:
  ModulePort getDownPort();
  Instance getInstance();

  InstancePort();
  explicit InstancePort(const Port& port);
};

class ModulePort : public Port {
  public:
  void destroy();

  InstancePort getUpPort(Instance inst);

  void resetDir();
  void addDirIn();
  void addDirOut();

  ModulePort();
  explicit ModulePort(const Port& port);
};

} // End namespace gbl

#include "gbl_impl.hh"

#endif

