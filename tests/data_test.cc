
#include "data_impl.hh"
#include "gbl_translate.hh"

#include <iostream>

using namespace gbl;
using namespace gbl::internal;
using namespace std;

const ID maxTestID = 1000;

void testNames() {
    cout << "Started test for names" << endl;
    DataImpl data;
    // Test names
    for (ID i=0; i<maxTestID; ++i) {
        if (data.hasName(i)) abort();
    }
    for (ID i=0; i<maxTestID; ++i) {
        if (!data.addName(i)) abort();
    }
    for (ID i=0; i<maxTestID; ++i) {
        if (!data.hasName(i)) abort();
    }
    for (ID i=0; i<maxTestID; ++i) {
        if (!data.eraseName(i)) abort();
    }
    for (ID i=0; i<maxTestID; ++i) {
        if (data.hasName(i)) abort();
    }
    cout << "Finished test for names" << endl;
}

void testProps() {
    cout << "Started test for properties" << endl;
    DataImpl data;
    // Test properties
    for (ID i=0; i<maxTestID; ++i) {
        if (data.hasProp(i)) abort();
    }
    for (ID i=0; i<maxTestID; ++i) {
        if (!data.addProp(i)) abort();
    }
    for (ID i=0; i<maxTestID; ++i) {
        if (!data.hasProp(i)) abort();
    }
    for (ID i=0; i<maxTestID; ++i) {
        if (!data.eraseProp(i)) abort();
    }
    for (ID i=0; i<maxTestID; ++i) {
        if (data.hasProp(i)) abort();
    }
    cout << "Finished test for properties" << endl;
}

void testAttrs() {
    cout << "Started test for attributes" << endl;
    DataImpl data;
    // Test attributes
    for (ID i=0; i<maxTestID; ++i) {
        if (data.hasAttr(i)) abort();
    }
    for (ID i=0; i<maxTestID; ++i) {
        Attribute attr;
        attr._id = i;
        attr._val._id = 0;
        if (!data.addAttr(attr)) abort();
    }
    for (ID i=0; i<maxTestID; ++i) {
        if (!data.hasAttr(i)) abort();
    }
    for (ID i=0; i<maxTestID; ++i) {
        if (!data.eraseAttr(i)) abort();
    }
    for (ID i=0; i<maxTestID; ++i) {
        if (data.hasAttr(i)) abort();
    }
    cout << "Finished test for attributes" << endl;
}

void testTranslator() {
    cout << "Started test for translation" << endl;

    #define GBL_DECL_ARRAY
    #include "gbl_symbols.hh"
    #undef GBL_DECL_ARRAY

    Translator t1;
    for (ID id = Symbol::ENUM_NULL_SYMBOL+1; id < Symbol::ENUM_MAX_SYMBOL; ++id) {
        if (t1.getOrRegisterID(string(symbolStrings[id])) != id) {
            abort();
        }
    }
    Translator t2;
    for (ID id = Symbol::ENUM_NULL_SYMBOL+1; id < Symbol::ENUM_MAX_SYMBOL; ++id) {
        if (t2.getString(id) != symbolStrings[id]) {
            abort();
        }
    }

    Translator t3;
    std::string testStrings[] = { "Bidule", "", "Truc", ";&#\\" };
    for (const std::string& s : testStrings) {
        ID id = t3.getOrRegisterID(s);
        if (t3.getString(id) != s) {
            abort();
        } 
    }
    cout << "Finished test for translation" << endl;
}

int main() {
    testNames();
    testProps();
    testAttrs();
    testTranslator();
    return 0;
}

