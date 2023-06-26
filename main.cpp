/*
 * File: main.cpp
 * --------------
 * Sample QT project
 */

#include <iostream>
#include "console.h"
#include "testing/SimpleTest.h"
using namespace std;

/*
 * This sample main brings up testing menu.
 */
int main() {
    if (runSimpleTests(SELECTED_TESTS)) {
        return 0;
    }
    cout << "All done, exiting" << endl;
    return 0;
}

PROVIDED_TEST("Sample use of SimpleTest")
{
    EXPECT_EQUAL(1 + 1, 2);
}