
#include "testing.hh"
#include "private/data_impl.hh"
#include "private/gbl_translate.hh"

using namespace gbl;
using namespace gbl::internal;
using namespace std;

const ID maxTestID = 1000;

BOOST_AUTO_TEST_SUITE(DataTest)

BOOST_AUTO_TEST_CASE(testNames) {
    DataImpl data;
    // Test names
    for (ID i=0; i<maxTestID; ++i) {
        BOOST_CHECK (!data.hasName(i));
    }
    for (ID i=0; i<maxTestID; ++i) {
        BOOST_CHECK ( data.addName(i));
    }
    for (ID i=0; i<maxTestID; ++i) {
        BOOST_CHECK ( data.hasName(i));
    }
    for (ID i=0; i<maxTestID; ++i) {
        BOOST_CHECK ( data.eraseName(i));
    }
    for (ID i=0; i<maxTestID; ++i) {
        BOOST_CHECK (!data.hasName(i));
    }
}

BOOST_AUTO_TEST_CASE(testProps) {
    DataImpl data;
    // Test properties
    for (ID i=0; i<maxTestID; ++i) {
        BOOST_CHECK (!data.hasProp(i));
    }
    for (ID i=0; i<maxTestID; ++i) {
        BOOST_CHECK ( data.addProp(i));
    }
    for (ID i=0; i<maxTestID; ++i) {
        BOOST_CHECK ( data.hasProp(i));
    }
    for (ID i=0; i<maxTestID; ++i) {
        BOOST_CHECK ( data.eraseProp(i));
    }
    for (ID i=0; i<maxTestID; ++i) {
        BOOST_CHECK (!data.hasProp(i));
    }
}

BOOST_AUTO_TEST_CASE(testAttrs) {
    DataImpl data;
    // Test attributes
    for (ID i=0; i<maxTestID; ++i) {
        BOOST_CHECK (!data.hasAttr(i));
    }
    for (ID i=0; i<maxTestID; ++i) {
        Attribute attr;
        attr._id = i;
        attr._val._id = 0;
        BOOST_CHECK ( data.addAttr(attr));
    }
    for (ID i=0; i<maxTestID; ++i) {
        BOOST_CHECK ( data.hasAttr(i));
    }
    for (ID i=0; i<maxTestID; ++i) {
        BOOST_CHECK ( data.eraseAttr(i));
    }
    for (ID i=0; i<maxTestID; ++i) {
        BOOST_CHECK (!data.hasAttr(i));
    }
}

BOOST_AUTO_TEST_CASE(testTranslator) {

    #define GBL_DECL_ARRAY
    #include "gbl_symbols.hh"
    #undef GBL_DECL_ARRAY

    Translator t1;
    for (ID id = Symbol::ENUM_NULL_SYMBOL+1; id < Symbol::ENUM_MAX_SYMBOL; ++id) {
        BOOST_CHECK_EQUAL (t1.getOrRegisterID(string(symbolStrings[id])), id);
    }
    Translator t2;
    for (ID id = Symbol::ENUM_NULL_SYMBOL+1; id < Symbol::ENUM_MAX_SYMBOL; ++id) {
        BOOST_CHECK_EQUAL (t2.getString(id), symbolStrings[id]);
    }

    Translator t3;
    std::string testStrings[] = { "Bidule", "", "Truc", ";&#\\" };
    for (const std::string& s : testStrings) {
        ID id = t3.getOrRegisterID(s);
        BOOST_CHECK_EQUAL (t3.getString(id), s);
    }
}

BOOST_AUTO_TEST_SUITE_END()

