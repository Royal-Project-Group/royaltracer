# SimpleWebServerWithBrowser.py
from http.server import SimpleHTTPRequestHandler, HTTPServer
import webbrowser
import threading

def open_browser():
    webbrowser.open_new('http://localhost:8000')

def run(server_class=HTTPServer, handler_class=SimpleHTTPRequestHandler):
    server_address = ('', 8000)  # Serve on all addresses, port 8000
    httpd = server_class(server_address, handler_class)
    print("Starting httpd...")

    # Open the web browser
    browser_thread = threading.Thread(target=open_browser)
    browser_thread.start()

    # Start the server
    httpd.serve_forever()

if __name__ == "__main__":
    run()
