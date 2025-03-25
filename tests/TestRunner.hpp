#pragma once

// Include both test setup implementations
#include "TestSetup.hpp"
#include "ThreadReversedTestSetup.hpp"
#define USE_THREAD_REVERSED_TESTS
// Define a macro that selects the test setup class
#ifndef USE_THREAD_REVERSED_TESTS
#define TestSetup TestSetup
#else
#define TestSetup ThreadReversedTestSetup
#endif
