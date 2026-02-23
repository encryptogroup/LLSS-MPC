#!/bin/bash

# Check if script and runs N are provided
if [ $# -lt 2 ]; then
    echo "Usage: $0 <N_times> <./other_script.sh> [script_args...]"
    exit 1
fi

N=$1       # Number of runs
shift
COMMAND="$@" # The script and its arguments

# 1. Set TIMEFORMAT to output ONLY the real time in seconds (e.g., "3.145")
# The %E or %R format specifiers print the elapsed time.
# The default TIMEFORMAT works for the simple time command
# but we will use the external `/usr/bin/time` or a simple bash loop.

# Use a subshell to capture the total time to a temporary file
# The 'real' time is the first line in the POSIX output
REAL_TIME=$( { time -p for i in $(seq 1 $N); do $COMMAND >/dev/null 2>&1; done; } 2>&1 | awk '/real/ { print $2; exit }' )

# 2. Calculate the average using 'bc' for floating-point math
if [ -n "$REAL_TIME" ]; then
    AVERAGE=$(echo "scale=6; $REAL_TIME / $N" | bc)

    echo "---"
    echo "Total Runs: $N"
    echo "Total Real Time: ${REAL_TIME}s"
    echo "Average Real Time: **${AVERAGE}s**"
else
    echo "Error: Could not measure time or the command failed to execute."
fi
