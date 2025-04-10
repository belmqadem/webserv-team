#!/usr/bin/env python3

import os

print("Content-Type: text/html")
print()

cookies = os.environ.get("HTTP_COOKIE", "")

html = "<html><body><h2>Cookies</h2><ul>"

if cookies:
    for pair in cookies.split("; "):
        html += f"<li>{pair}</li>"
else:
    html += "<li>No cookies found</li>"

html += "</ul><a href='/test/'>Back</a></body></html>"

print(html)
