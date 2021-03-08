# squid-proxy-socket
In this assignment, we establish a socket connection to the squid proxy server </br>
The socket is created by passing the IP address, port number of the proxy and the encoded username and password for auth </br>
Then an HTTP request is sent to the proxy, and the response received from the server is read into index.html file after removing headers </br>
Then we send another request to get the logo (this is only if the host is info.in2p3.fr, but could be extended to other websites as well) </br>
The response received from the server is read into a logo.gif file after removing the HTTP headers </br>

## A few points
- This project doesn't handle HTTPS hosts or HTTPS redirects
- Yet to implement relative path redirects
- This project doesn't handle dynamic websites
- This project doesn't handle websites with chunked encodings
- The perl script is only a diff between the files

## How to run
1. Clone the repository to a directory of your choice and cd into the directory
2. Run `bash ./script.sh <host/path>`
    ```
    The following commands are used to execute the C file:
    $ gcc http_proxy_download.c -o http_proxy_download.out
    $ ./http_proxy_downoad.out <host/path> <proxy_ip> <port_no> <username> <password> <html_file> <image_file>
    ```
3. Write the curl output for the website onto _std_html.html_
4. Run the _autoevaluate_test.pl_ file to compare the html files
