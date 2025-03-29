#!/bin/bash
# CGI Load Tester - Sends multiple batched requests to test CGI
# Usage: ./test_cgi.sh [total_clients] [batch_size] [delay_seconds] [endpoint]

# Default values
TOTAL_CLIENTS=${1:-10}      # Total number of requests to make
BATCH_SIZE=${2:-3}          # How many clients to launch at once
DELAY=${3:-1}               # Delay between batches in seconds
ENDPOINT=${4:-"test.php"}   # Which endpoint to test
SERVER="http://localhost:8080"

echo "=== CGI Load Test ==="
echo "Total clients: $TOTAL_CLIENTS"
echo "Batch size: $BATCH_SIZE"
echo "Delay between batches: $DELAY seconds"
echo "Testing endpoint: $SERVER/$ENDPOINT"
echo "========================"
echo "Press Enter to start or Ctrl+C to cancel..."
read

COUNT=0
SUCCESS=0
FAILED=0

test_endpoint() {
  local id=$1
  local output=$(curl -s -w "%{http_code}" "$SERVER/$ENDPOINT?client=$id" -o /dev/null)
  
  if [ "$output" = "200" ]; then
    SUCCESS=$((SUCCESS+1))
    echo "Client $id: Success (HTTP $output)"
  else
    FAILED=$((FAILED+1))
    echo "Client $id: Failed (HTTP $output)"
  fi
}

for ((i=1; i<=TOTAL_CLIENTS; i+=$BATCH_SIZE)); do
  echo "Starting batch $((i/BATCH_SIZE + 1))..."
  
  # Start a batch of clients in parallel
  for ((j=i; j<i+BATCH_SIZE && j<=TOTAL_CLIENTS; j++)); do
    test_endpoint $j &
    COUNT=$((COUNT+1))
  done
  
  # Wait for this batch to complete
  wait
  
  # Display progress
  echo "Progress: $COUNT/$TOTAL_CLIENTS completed ($SUCCESS success, $FAILED failed)"
  
  # Sleep before next batch unless this was the last batch
  if [ $i -lt $TOTAL_CLIENTS ]; then
    echo "Sleeping for $DELAY seconds..."
    sleep $DELAY
  fi
done

echo "=== Test Complete ==="
echo "Total requests: $COUNT"
echo "Successful: $SUCCESS"
echo "Failed: $FAILED"
echo "Success rate: $(echo "scale=2; $SUCCESS*100/$COUNT" | bc)%"