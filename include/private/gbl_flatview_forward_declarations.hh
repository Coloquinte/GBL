// Copyright (C) 2016 Gabriel Gouvine - All Rights Reserved

#ifndef GBL_FLATVIEW_FORWARD_DECLARATIONS_HH
#define GBL_FLATVIEW_FORWARD_DECLARATIONS_HH

#include "private/gbl_forward_declarations.hh"

namespace gbl {

class FlatModule;
class FlatNode;
class FlatInstance;
class FlatWire;
class FlatPort;
class FlatInstancePort;
class FlatModulePort;
class FlatView;

namespace internal {
class FlatTransform;

typedef TransformIterator<WireIterator,         FlatTransform> FlatWireIterator;
typedef TransformIterator<NodeIterator,         FlatTransform> FlatNodeIterator;
typedef TransformIterator<InstanceIterator,     FlatTransform> FlatInstanceIterator;
typedef TransformIterator<WirePortIterator,     FlatTransform> FlatWirePortIterator;
typedef TransformIterator<NodePortIterator,     FlatTransform> FlatNodePortIterator;
typedef TransformIterator<InstancePortIterator, FlatTransform> FlatInstancePortIterator;
typedef TransformIterator<ModulePortIterator,   FlatTransform> FlatModulePortIterator;
}

struct FlatRef {
  bool operator==(const FlatRef&) const;
  bool operator!=(const FlatRef&) const;

  const FlatView& _view;
  FlatSize _index;

  FlatRef(FlatSize index, const FlatView& view);
};

} // End namespace gbl


#endif

