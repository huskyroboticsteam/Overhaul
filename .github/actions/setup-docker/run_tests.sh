#!/bin/bash

colcon test --ctest-args -LE linter --event-handlers console_direct+ && colcon test-result --verbose