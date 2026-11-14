#!/bin/bash

cd /home/ws
colcon build
colcon test --ctest-args -LE linter --event-handlers console_direct+ && colcon test-result --verbose