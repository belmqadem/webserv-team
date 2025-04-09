#!/usr/bin/env python3

import os
import cgi

form = cgi.FieldStorage()
name = form.getfirst("name")
value = form.getfirst("value")

if name and value:
    # Save to server-side file
    with open("/home/abel-mqa/Desktop/webserv/www/cookie/cookie.txt", "a") as f:
        f.write(f"{name}={value}\n")

    # Set cookie header
    print(f"Set-Cookie: {name}={value}; Path=/")

print("Content-Type: text/html\n")
print(f"<html><body><p>Cookie '{name}' set with value '{value}' and saved on the server.</p></body></html>")
