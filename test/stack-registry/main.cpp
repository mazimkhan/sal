/*
 * PackageLicenseDeclared: Apache-2.0
 * Copyright (c) 2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include "sal/socket_api.h"
#include "sal/test/ctest_env.h"
#include "mbed/test_env.h"

struct socket_api test_api[SOCKET_MAX_STACKS+1];
struct socket_api expect_fail_api;

#define SOCKET_ABSTRACTION_LAYER_VERSION 1

int test_socket_stack_registry() {
    unsigned int i;
    socket_error_t err = SOCKET_ERROR_NONE;
    printf("Testing stack registry...\n");
    TEST_CLEAR();

    // Try to register a stack marked as Uninitialized
    expect_fail_api.stack = SOCKET_STACK_UNINIT;
    expect_fail_api.version = SOCKET_ABSTRACTION_LAYER_VERSION;
    err = socket_register_stack((&expect_fail_api));
    TEST_NEQ(err,SOCKET_ERROR_NONE);

    // Register a NULL socket api
    memset(&expect_fail_api, 0, sizeof(struct socket_api));
    expect_fail_api.version = SOCKET_ABSTRACTION_LAYER_VERSION;
    err = socket_register_stack(&expect_fail_api);
    TEST_NEQ(err,SOCKET_ERROR_NONE);

    // Register a socket api with one zeroed API.
    memset(&expect_fail_api, 1, sizeof(struct socket_api));
    expect_fail_api.version = SOCKET_ABSTRACTION_LAYER_VERSION;
    expect_fail_api.create = NULL;
    expect_fail_api.stack = static_cast<socket_stack_t>(SOCKET_STACK_UNINIT + 1);
    err = socket_register_stack(&expect_fail_api);
    TEST_NEQ(err,SOCKET_ERROR_NONE);

    // Register a socket api with the version set wrong
    memset(&expect_fail_api, 1, sizeof(struct socket_api));
    expect_fail_api.version = 0;
    expect_fail_api.stack = static_cast<socket_stack_t>(SOCKET_STACK_UNINIT + 1);
    err = socket_register_stack(&expect_fail_api);
    TEST_NEQ(err,SOCKET_ERROR_NONE);

    // Register two of the same socket api's
    memset(&test_api[0], 1, sizeof(struct socket_api));
    memset(&expect_fail_api, 1, sizeof(struct socket_api));
    test_api[0].stack = static_cast<socket_stack_t>(SOCKET_STACK_UNINIT + 1);
    test_api[0].version = SOCKET_ABSTRACTION_LAYER_VERSION;
    expect_fail_api.stack = static_cast<socket_stack_t>(SOCKET_STACK_UNINIT + 1);
    expect_fail_api.version = SOCKET_ABSTRACTION_LAYER_VERSION;
    err = socket_register_stack(&test_api[0]);
    TEST_EQ(err,SOCKET_ERROR_NONE);
    err = socket_register_stack(&expect_fail_api);
    TEST_NEQ(err,SOCKET_ERROR_NONE);

    // Register the same stack again, but with the stack ID changed

    test_api[0].stack = static_cast<socket_stack_t>(static_cast<uint32_t>(test_api[0].stack) + 1);
    test_api[0].version = SOCKET_ABSTRACTION_LAYER_VERSION;
    err = socket_register_stack(&test_api[0]);
    TEST_NEQ(err,SOCKET_ERROR_NONE);
    test_api[0].stack = static_cast<socket_stack_t>(static_cast<uint32_t>(test_api[0].stack) - 1);

    // Try to register a stack outside the accepted range
    expect_fail_api.stack = SOCKET_STACK_MAX;
    expect_fail_api.version = SOCKET_ABSTRACTION_LAYER_VERSION;
    err = socket_register_stack(&expect_fail_api);
    TEST_NEQ(err,SOCKET_ERROR_NONE);

    // One stack is already registered
    // Try to register the maximum number of stacks
    for (i = 1; i < SOCKET_MAX_STACKS; i++) {
        socket_stack_t stack = static_cast<socket_stack_t>(SOCKET_STACK_UNINIT + 1 + i);
        memset (&test_api[i],1,sizeof(struct socket_api));
        test_api[i].stack = stack;
        test_api[i].version = SOCKET_ABSTRACTION_LAYER_VERSION;
        err = socket_register_stack(&test_api[i]);
        TEST_EQ(err,SOCKET_ERROR_NONE);
    }
    // Then register one more.
    expect_fail_api.stack = static_cast<socket_stack_t>(SOCKET_MAX_STACKS + 1);
    expect_fail_api.version = SOCKET_ABSTRACTION_LAYER_VERSION;
    err = socket_register_stack(&expect_fail_api);
    TEST_NEQ(err,SOCKET_ERROR_NONE);

    // Extract an uninit socket api
    const struct socket_api *papi;
    papi = socket_get_api(SOCKET_STACK_UNINIT);
    TEST_EQ(papi, NULL);

    if (SOCKET_MAX_STACKS < SOCKET_STACK_MAX - 1) {
        // Get a valid, but unregistered stack
        papi = socket_get_api(static_cast<socket_stack_t>(SOCKET_STACK_MAX - 1));
        TEST_EQ(papi, NULL);
    }
    // Verify all registered stacks
    for (i = 0; i < SOCKET_MAX_STACKS; i++) {
        socket_stack_t stack = static_cast<socket_stack_t>(SOCKET_STACK_UNINIT + 1 + i);
        papi = socket_get_api(stack);
        TEST_EQ(papi, &test_api[i]);
    }

    return test_pass_global;
}

void app_start(int, char**) {
    int rc;
    MBED_HOSTTEST_SELECT(default);
    MBED_HOSTTEST_TIMEOUT(10);
    MBED_HOSTTEST_DESCRIPTION(Test the socket stack registry);
    MBED_HOSTTEST_START("STACK_REGISTRY");
    rc = test_socket_stack_registry();
    MBED_HOSTTEST_RESULT(rc);
}
