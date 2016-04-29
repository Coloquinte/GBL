
#ifndef GBL_FLATVIEW_HH
#define GBL_FLATVIEW_HH

#include "private/gbl_forward_declarations.hh"
#include "gbl.hh"

#include <vector>
#include <unordered_map>
#include <algorithm>

namespace gbl {

typedef std::uint64_t FlatSize;

class FlatView {
public:
    FlatView(Module topModule);

    Module getModule(FlatSize index) const;
    Instance getInstance(FlatSize index) const;
    FlatSize getParentModule() const;

private:
    struct InfoBase {
        // Index of the module (child or parent) in the FlatView
        Size _moduleIndex;
        // Index of the corresponding instance in the parent module
        Size _instanceIndex;
        // Offset between child and parent flat indexing ranges
        FlatSize _offset;
        InfoBase(Size modInd=-1, Size instInd=-1, FlatSize offs=-1)
            : _moduleIndex(modInd)
            , _instanceIndex(instInd)
            , _offset(offs)
            {}
    };
    struct ParentInfos {
        std::vector<FlatSize> _instEndIndexs;
        std::vector<InfoBase> _upInfos;
    };
    struct ChildInfos {
        std::vector<InfoBase> _downInfos;
    };

private:
    Size getModIndex(FlatSize flatIndex) const;
    InfoBase getInstInfo(Size modIndex, FlatSize flatIndex) const;
    InfoBase getInstInfo(FlatSize flatIndex) const;

private:
    Module _topMod;

    // Basic module <--> index range bookkeeping
    std::vector<internal::ModuleImpl*>   _mods;
    // Module to flat index range (_modEndIndexs[i] to _modEndIndexs[i+1] for module i)
    std::vector<FlatSize> _modEndIndexs;

    // From Module to FlatModule
    std::unordered_map<internal::ModuleImpl*, Size> _mod2Index;

    // Hierarchy bookkeeping
    // Up instances
    std::vector<ParentInfos> _parents;
    // Down instances
    std::vector<ChildInfos>  _children;

    // Contiguous indexing for wires
    // Contiguous indexing for ports
    // Contiguous indexing for transhierarchical wires
};

inline Module FlatView::getModule(FlatSize index) const {
    Size ind = std::distance(
        _modEndIndexs.begin(),
        std::upper_bound(_modEndIndexs.begin(), _modEndIndexs.end(), index)
    );
    assert(ind > 0 && ind <= _mods.size());
    return Module(_mods[ind-1]);
}

} // End namespace gbl

#endif

