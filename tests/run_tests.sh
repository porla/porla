#!/usr/bin/env bash

# Function to check if port is open
wait_for_port() {
    local port=$1
    local timeout=$2
    local elapsed=0

    while ! curl -s --max-time 1 http://localhost:$port > /dev/null 2>&1; do
        if [ $elapsed -ge $timeout ]; then
            echo "Timeout waiting for port $port"
            kill $PID 2>/dev/null
            wait $PID 2>/dev/null
            exit 1
        fi
        sleep 0.5
        elapsed=$((elapsed + 1))
    done
}

# Launch Porla and return a PID
launch_porla() {
    BINARY_OUTPUT_FILE=$(mktemp)
    "../build/porla" --db=:memory: > "$BINARY_OUTPUT_FILE" 2>&1 &
    PID=$!

    wait_for_port 1337 10
    echo "$PID $BINARY_OUTPUT_FILE"
}

TOTAL=0
PASSED=0
FAILED=0

declare -A HURL_OUTPUTS
declare -A HURL_RESULTS

for hurl_file in hurl/*.hurl; do
    TOTAL=$((TOTAL + 1))

    filename=$(basename "$hurl_file")
    echo -n "running: $filename"

    read PID OUTPUT_FILE <<< "$(launch_porla)"

    HURL_OUTPUT=$(hurl "$hurl_file" 2>&1)
    HURL_EXIT=$?

    HURL_OUTPUTS["$filename"]="$HURL_OUTPUT"
    HURL_RESULTS["$filename"]=$HURL_EXIT
    
    if [ $HURL_EXIT -eq 0 ]; then
        PASSED=$((PASSED + 1))
        echo "  ✓ PASSED"
    else
        FAILED=$((FAILED + 1))
        echo "  ✗ FAILED (exit code: $HURL_EXIT)"
        echo $HURL_OUTPUT
        cat $OUTPUT_FILE
    fi

    kill -INT $PID
    wait $PID 2>/dev/null
done

exit 0
