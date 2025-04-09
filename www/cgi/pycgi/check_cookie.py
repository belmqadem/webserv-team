#!/usr/bin/env python3

print("Content-Type: text/html\n")

try:
    with open("/home/abel-mqa/Desktop/webserv/www/cookie/cookie.txt", "r") as f:
        cookies = f.read().strip()
except FileNotFoundError:
    cookies = "No cookies saved."

print(f"<html><body><pre>{cookies}</pre></body></html>")
