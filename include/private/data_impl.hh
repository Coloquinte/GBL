// Copyright (C) 2016 Gabriel Gouvine - All Rights Reserved

#ifndef GBL_DATA_IMPL_HH
#define GBL_DATA_IMPL_HH

#include "gbl_forward_declarations.hh"

#include <vector>

namespace gbl {
namespace internal {

struct Attribute {
  enum AttrType {
    Id,
    Int64
  };
  union AttrVal {
    ID _id;
    std::int64_t _int64;
  };

  ID       _id;
  AttrType _type;
  AttrVal  _val;
};

struct DataImpl {
  std::vector<ID>        _names;
  std::vector<ID>        _props;
  std::vector<Attribute> _attrs;

  bool hasName(ID name) const;
  bool hasProp(ID prop) const;
  bool hasAttr(ID attr) const;

  bool addName(ID name);
  bool addProp(ID prop);
  bool addAttr(Attribute attr);

  bool eraseName(ID name);
  bool eraseProp(ID prop);
  bool eraseAttr(ID attr);

  Attribute getAttr(ID attr) const;

  NameIterator beginNames() const { return &_names[0]; }
  NameIterator endNames  () const { return &_names[0] + _names.size(); }
  PropertyIterator beginProps() const { return &_props[0]; }
  PropertyIterator endProps  () const { return &_props[0] + _props.size(); }

  void clear();
};

inline bool DataImpl::hasName(ID name) const {
  for(ID id : _names){
    if (id == name) return true;
  }
  return false;
}
inline bool DataImpl::hasProp(ID prop) const {
  for(ID id : _props){
    if (id == prop) return true;
  }
  return false;
}
inline bool DataImpl::hasAttr(ID attr) const {
  for(Attribute a : _attrs){
    if (a._id == attr) return true;
  }
  return false;
}
inline Attribute DataImpl::getAttr(ID attr) const {
  for(Attribute a : _attrs){
    if (a._id == attr) {
      return a;
    }
  }
  // Bad behaviour: should error
  return Attribute();
}
inline bool DataImpl::addName(ID name) {
  if (hasName(name)) {
    return false;
  }
  else {
    _names.push_back(name);
    return true;
  }
}
inline bool DataImpl::addProp(ID prop) {
  if (hasProp(prop)) {
    return false;
  }
  else {
    _props.push_back(prop);
    return true;
  }
}
inline bool DataImpl::addAttr(Attribute attr) {
  if (hasAttr(attr._id)) {
    return false;
  }
  else {
    _attrs.push_back(attr);
    return true;
  }
}
inline bool DataImpl::eraseName(ID name) {
  for(Size i=0; i<_names.size(); ++i){
    if(_names[i] == name) {
      std::swap(_names[i], _names.back());
      _names.pop_back();
      return true;
    }
  }
  return false;
}
inline bool DataImpl::eraseProp(ID prop) {
  for(Size i=0; i<_props.size(); ++i){
    if(_props[i] == prop) {
      std::swap(_props[i], _props.back());
      _props.pop_back();
      return true;
    }
  }
  return false;
}
inline bool DataImpl::eraseAttr(ID attr) {
  for(Size i=0; i<_attrs.size(); ++i){
    if(_attrs[i]._id == attr) {
      std::swap(_attrs[i], _attrs.back());
      _attrs.pop_back();
      return true;
    }
  }
  return false;
}
inline void DataImpl::clear() {
  _names.clear();
  _props.clear();
  _attrs.clear();
}

} // End namespace internal
} // End namespace gbl

#endif

