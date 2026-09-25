#pragma once

extern "C" {
#include <CAN26.h>
}

/**
 * @namespace can
 * @brief Utilities for interacting with CAN devices.
 */
namespace can {

/**
 * @brief Initialize the CAN interface.
 * 
 * This should only be called once.
 * 
 * @note If CAN initialization fails, the program will exit.
 */
void initCAN();

} // namespace can