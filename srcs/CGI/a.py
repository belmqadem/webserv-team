#!/usr/bin/python3

import os
import sys

content_length = int(os.environ.get("CONTENT_LENGTH", 0))
body = sys.stdin.read(content_length)

print("Content-Type: text/plain")
print("")
print(f"Received POST Data:\n{body}")
