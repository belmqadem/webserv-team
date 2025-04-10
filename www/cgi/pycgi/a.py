#!/usr/bin/env python3

import cgi
import os
import sys
import cgitb
cgitb.enable()  # Enable detailed error reporting

def main():
    # Print HTTP headers first
    print("Content-type: text/html")
    print()  # Empty line to separate headers from body

    # Start HTML output
    print("<html>")
    print("<head><title>Python CGI Test</title>")
    print("<style>")
    print("body { font-family: Arial, sans-serif; margin: 20px; }")
    print("h1 { color: #0066cc; }")
    print("h2 { color: #009900; margin-top: 20px; }")
    print("table { border-collapse: collapse; margin: 10px 0; }")
    print("th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }")
    print("th { background-color: #f2f2f2; }")
    print("</style>")
    print("</head>")
    print("<body>")

    print("<h1>Python CGI Test Script</h1>")
    print("<p>This script demonstrates Python CGI functionality.</p>")

    # Environment Variables
    print("<h2>Environment Variables</h2>")
    print("<table>")
    print("<tr><th>Variable</th><th>Value</th></tr>")

    important_vars = [
        "REQUEST_METHOD", "CONTENT_TYPE", "CONTENT_LENGTH",
        "QUERY_STRING", "REMOTE_ADDR", "SERVER_NAME",
        "SERVER_PROTOCOL", "HTTP_USER_AGENT"
    ]

    for var in important_vars:
        if var in os.environ:
            print(f"<tr><td>{var}</td><td>{os.environ.get(var)}</td></tr>")
    print("</table>")

    # Form Data Processing
    print("<h2>Form Data</h2>")
    form = cgi.FieldStorage()

    if len(form) > 0:
        print("<table>")
        print("<tr><th>Field Name</th><th>Value</th></tr>")
        for key in form.keys():
            if key != "file":  # Skip file uploads for this table
                value = form.getvalue(key)
                print(f"<tr><td>{key}</td><td>{value}</td></tr>")
        print("</table>")
    else:
        print("<p>No form data received.</p>")

    print("<h2>System Information</h2>")
    print("<table>")
    print(f"<tr><td>Python Version</td><td>{sys.version}</td></tr>")
    print(f"<tr><td>Python Executable</td><td>{sys.executable}</td></tr>")
    print(f"<tr><td>Current Directory</td><td>{os.getcwd()}</td></tr>")
    print("</table>")

    print("</body></html>")

if __name__ == "__main__":
    main()