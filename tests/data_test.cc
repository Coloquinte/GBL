
#include "data_impl.hh"

#include <iostream>

using namespace gbl;
using namespace gbl::internal;
using namespace std;

const ID maxTestID = 10000;

void testNames() {
    cout << "Started test for names" << endl;
    DataImpl data;
    // Test names
    for (ID i=0; i<maxTestID; ++i) {
        if (data.hasName(i)) abort();
    }
    for (ID i=0; i<maxTestID; ++i) {
        data.addName(i);
    }
    for (ID i=0; i<maxTestID; ++i) {
        if (!data.hasName(i)) abort();
    }
    for (ID i=0; i<maxTestID; ++i) {
        data.removeName(i);
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
        data.addProp(i);
    }
    for (ID i=0; i<maxTestID; ++i) {
        if (!data.hasProp(i)) abort();
    }
    for (ID i=0; i<maxTestID; ++i) {
        data.removeProp(i);
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
        data.addAttr(attr);
    }
    for (ID i=0; i<maxTestID; ++i) {
        if (!data.hasAttr(i)) abort();
    }
    for (ID i=0; i<maxTestID; ++i) {
        data.removeAttr(i);
    }
    for (ID i=0; i<maxTestID; ++i) {
        if (data.hasAttr(i)) abort();
    }
    cout << "Finished test for attributes" << endl;
}

int main() {
    testNames();
    testProps();
    testAttrs();
    return 0;
}

