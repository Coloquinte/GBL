
#ifndef GBL_TRANSLATOR_HH
#define GBL_TRANSLATOR_HH

#include "gbl_forward_declarations.hh"
#include "gbl_symbols.hh"

#include <string>
#include <unordered_map>
#include <mutex>
#include <cassert>

namespace gbl {

class Translator {
  public:
  ID getOrRegisterID(const std::string& name);
  std::string getString(ID id) const;

  Translator();

  private:
  std::unordered_map<std::string, ID> _string2id;
  std::vector<std::string>            _id2string;
  mutable std::mutex                  _lock;
};

inline ID
Translator::getOrRegisterID(const std::string& name) {
    std::lock_guard<std::mutex> guard(_lock);
    auto it = _string2id.find(name);
    if (it != _string2id.end()) {
        return it->second;
    }
    else {
        ID ret = _id2string.size();
        _id2string.push_back(name);
        _string2id.emplace(name, ret);
        return ret;
    }
}

inline std::string
Translator::getString(ID id) const {
    std::lock_guard<std::mutex> guard(_lock);
    return _id2string.at(id);
}

inline
Translator::Translator() {
    #define GBL_DECL_ARRAY
    #include "gbl_symbols.hh"
    #undef GBL_DECL_ARRAY

    for (ID id = Symbol::ENUM_NULL_SYMBOL; id < Symbol::ENUM_MAX_SYMBOL; ++id) {
        if (id != Symbol::ENUM_NULL_SYMBOL) {
            _id2string.emplace_back(symbolStrings[id]);
            _string2id.emplace(_id2string.back(), id);
        }
        else {
            _id2string.emplace_back();
        }
    }
}

} 

#endif

