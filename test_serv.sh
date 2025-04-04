#!/bin/bash
# security_test.sh - Test web server for common security vulnerabilities

HOST="localhost"
PORT="5050"
VERBOSE=0
LOGFILE="security_test.log"

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    -h|--host) HOST="$2"; shift 2 ;;
    -p|--port) PORT="$2"; shift 2 ;;
    -v|--verbose) VERBOSE=1; shift ;;
    -l|--log) LOGFILE="$2"; shift 2 ;;
    *) echo "Unknown parameter: $1"; exit 1 ;;
  esac
done

BASE_URL="http://$HOST:$PORT"
> "$LOGFILE"  # Clear log file

log() {
  echo "[$(date +%T)] $1" | tee -a "$LOGFILE"
}

test_request() {
  local method="$1"
  local path="$2"
  local expected_code="$3"
  local description="$4"
  local data="$5"
  
  local url="$BASE_URL$path"
  local opts="-s -o /dev/null -w '%{http_code}' --max-time 3"  # Add timeout
  
  # Special handling for file uploads
  if [[ "$data" == "--form"* ]]; then
    # For file uploads, we need to use the form data directly
    opts="$opts -X $method $data"
  else
    # Normal request handling
    [[ "$method" != "GET" ]] && opts="$opts -X $method"
    if [[ "$method" == "POST" && -z "$data" ]]; then
      data=" "  # Just a space to have a minimal request body
    fi
    [[ -n "$data" ]] && opts="$opts -d '$data'"
  fi
  
  if [[ $VERBOSE -eq 1 ]]; then
    log "Testing: $description ($method $path)"
    log "Command: curl $opts $url"
  fi
  
  local code=$(eval "curl $opts '$url'" 2>/dev/null)
  
  # Handle timeout (empty response)
  if [[ -z "$code" ]]; then
    log "⚠️ TIMEOUT: $description - Request timed out"
    return 1
  fi
  
  # Check if code matches any of the expected codes (separated by |)
  if [[ "$expected_code" == *"|"* ]]; then
    local match=0
    for expected in ${expected_code//|/ }; do
      if [[ "$code" == "$expected" ]]; then
        match=1
        break
      fi
    done
    
    if [[ $match -eq 1 ]]; then
      [[ $VERBOSE -eq 1 ]] && log "✅ PASS: $description ($code)"
      return 0
    else
      log "❌ FAIL: $description - Expected one of: $expected_code, Got: $code"
      return 1
    fi
  else
    # Original single code check
    if [[ "$code" == "$expected_code" ]]; then
      [[ $VERBOSE -eq 1 ]] && log "✅ PASS: $description ($code)"
      return 0
    else
      log "❌ FAIL: $description - Expected: $expected_code, Got: $code"
      return 1
    fi
  fi
}

# Create a test file for upload tests
echo "Test content for upload" > /tmp/test_upload.txt

log "Starting security tests against $BASE_URL"
log "----------------------------------------"

# # 1. Path Traversal Tests
# log "\n=== Path Traversal Tests ==="
# test_request "GET" "/../../../etc/passwd" "404|403" "Block access to system files"
# test_request "GET" "/..%2F..%2F..%2Fetc%2Fpasswd" "400|403" "Block encoded path traversal"
# test_request "GET" "/images/../../../../../../etc/passwd" "404|403" "Block nested path traversal"
# test_request "GET" "/cgi-bin/%2e%2e/%2e%2e/etc/passwd" "400|403" "Block double-encoded traversal"

# # 2. Directory Listing Tests
# log "\n=== Directory Listing Tests ==="
# test_request "GET" "/cgi-bin/" "404|403" "Prevent listing of CGI directory" 
# test_request "GET" "/.git/" "404|403" "Block access to .git directory"
# test_request "GET" "/.." "403|404" "Block parent directory listing"

# # 3. DELETE Method Tests
# log "\n=== DELETE Method Tests ==="
# test_request "DELETE" "/index.html" "404|405" "Block DELETE on static files"
# test_request "DELETE" "/cgi-bin/test.php" "404|405" "Block DELETE on CGI scripts"
# test_request "DELETE" "/config.conf" "404|403" "Block DELETE on server config"

# 4. File Upload Tests
# log "\n=== File Upload Tests ==="
# test_request "POST" "/upload" "201|403|404" "Handle upload to upload location"
# test_request "POST" "/upload.php" "200|404" "Handle upload to PHP script" \
#   "--form file=@/home/nmellal/Projects/webserv-team/www/uploads/diagram.png"

# 5. Special Character Tests
# log "\n=== Special Character Tests ==="
# test_request "GET" "/%20index.html" "400|404" "Block null byte injection"
# test_request "GET" "/|whoami" "400|404" "Block command injection characters"
# test_request "GET" "/;cat%20/etc/passwd" "400|404" "Block command separator"

# # 6. Script Execution Tests
# log "\n=== Script Execution Tests ==="
# test_request "GET" "/uploads/test.php" "403|404" "Block execution in uploads directory"
# test_request "GET" "/images/test.php.jpg" "200|404" "Prevent script execution in images"

# # 7. Resource Exhaustion Tests
log "\n=== Resource Tests ==="
test_request "GET" "/?$( printf 'a%.0s' {1..10000} )" "414|400|404" "Reject extremely long query strings"
log "Testing POST body size limit (this may take a moment)..."
test_request "POST" "/" "413|400" "Reject oversized POST body" \
  "$(dd if=/dev/zero bs=1M count=10 2>/dev/null | base64 | head -c 1000000)"

log "\n=== Security Test Completed ==="
FAILS=$(grep -c FAIL "$LOGFILE")
log "Total failures: $FAILS"
[[ "$FAILS" -eq 0 ]] && log "🔒 Your server passed all security tests!"

# Clean up
rm -f /tmp/test_upload.txt

echo "Detailed results available in $LOGFILE"