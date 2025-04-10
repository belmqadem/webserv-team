#!/usr/bin/env python3

import os

print("Content-Type: text/html\n")
print("<html><body><h2>Cookies</h2>")

cookie = os.environ.get("HTTP_COOKIE")
if cookie:
    print("<pre>")
    for c in cookie.split("; "):
        print(c)
    print("</pre>")
else:
    print("<p>No cookies found</p>")

print('<br><a href="/index.html">Back</a></body></html>')
