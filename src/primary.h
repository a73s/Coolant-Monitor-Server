#include <cstdint>

constexpr uint16_t MDNS_PORT = 46239;
constexpr short LOOPS_PER_SEC = 15;
constexpr short MIN_LOOP_TIME_MS = 1000/LOOPS_PER_SEC;

int primary();
