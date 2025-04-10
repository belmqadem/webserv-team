#!/usr/bin/env python3

import os
import cgi

print("Content-Type: text/html")

form = cgi.FieldStorage()
name = form.getvalue("name", "")
value = form.getvalue("value", "")

if name and value:
    print(f"Set-Cookie: {name}={value}; Path=/; HttpOnly; SameSite=Strict")
    print()
    print(f"<html><body><h2>Cookie '{name}' set to '{value}'</h2>")
    print("<a href='/pycgi/check_cookie.py'>Check Cookie</a></body></html>")
else:
    print()
    print("<html><body><h2>Missing name or value</h2></body></html>")
