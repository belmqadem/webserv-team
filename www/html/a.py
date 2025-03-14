# filepath: /home/mchihab/Desktop/webserv/www/html/a.py
import os
import urllib.parse
import time

# Parse query string
query_string = os.environ.get("QUERY_STRING", "")
params = urllib.parse.parse_qs(query_string)

# Extract 'name' parameter
name = params.get("name", [""])[0]
print("Content-Type: text/plain")
print("")
print(f"Received name parameter: {name}")