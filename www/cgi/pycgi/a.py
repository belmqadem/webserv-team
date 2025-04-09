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

    # File Upload Handling
    print("<h2>File Upload</h2>")
    if "file" in form:
        fileitem = form["file"]
        if fileitem.file:
            # It's an uploaded file
            filename = fileitem.filename
            filedata = fileitem.file.read()
            filesize = len(filedata)
            filetype = "binary" if b'\0' in filedata else "text"

            print("<table>")
            print(f"<tr><td>Filename</td><td>{filename}</td></tr>")
            print(f"<tr><td>File Size</td><td>{filesize} bytes</td></tr>")
            print(f"<tr><td>File Type</td><td>{filetype}</td></tr>")
            print("</table>")

            # Save the file
            try:
                upload_dir = "../uploads"
                if not os.path.exists(upload_dir):
                    os.makedirs(upload_dir)

                file_path = os.path.join(upload_dir, os.path.basename(filename))
                with open(file_path, 'wb') as f:
                    f.write(filedata)
                print(f"<p style='color: green;'>File successfully saved to {file_path}</p>")
            except Exception as e:
                print(f"<p style='color: red;'>Error saving file: {str(e)}</p>")
        else:
            print("<p>File field found but no file was uploaded.</p>")
    else:
        print("<p>No file was uploaded.</p>")

    # Test Form
    print("<h2>Test Form</h2>")
    print("<form method='post' enctype='multipart/form-data'>")
    print("<p><label>Name: <input type='text' name='name'></label></p>")
    print("<p><label>Message: <textarea name='message'></textarea></label></p>")
    print("<p><label>File: <input type='file' name='file'></label></p>")
    print("<p><input type='submit' value='Submit'></p>")
    print("</form>")

    # System Information
    print("<h2>System Information</h2>")
    print("<table>")
    print(f"<tr><td>Python Version</td><td>{sys.version}</td></tr>")
    print(f"<tr><td>Python Executable</td><td>{sys.executable}</td></tr>")
    print(f"<tr><td>Current Directory</td><td>{os.getcwd()}</td></tr>")
    print("</table>")

    print("</body></html>")

if __name__ == "__main__":
    main()