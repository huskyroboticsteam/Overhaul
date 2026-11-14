uint8 IDLE=0
uint8 PENDING=1
uint8 VERIFIED=2
uint8 FAILED=3
uint8 EXPIRED=4

builtin_interfaces/Time stamp
uint64 command_id
float64 requested_velocity
float64 measured_velocity
# Seconds since the last valid feedback; -1 and NaN velocity mean unknown.
float64 feedback_age
bool feedback_valid
bool ready
uint8 command_result
string fault
