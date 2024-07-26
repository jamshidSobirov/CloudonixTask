Here's the task description:

Programming Test
Cloudonix.io

OVERVIEW:
The test involves writing a simple Android application that calls into a C++ function and an HTTP server and then shows the results in an activity.
● Please implement the Android code in Java.
● The program can be published on GitHub or a comparable source code hosting service, or you can email it back.

REQUIREMENTS:

1. The application will be built to target Android API level 24, and using the latest NDK LTS release.

2. The application, either immediately when launched or with the press of a button, will call into a C++ function that retrieves the IP address of the device. The application will then send the IP address it found to an HTTP server, it will then display the results from the HTTP response to the user.

3. The C++ function will use the NDK getifaddrs() API (the NDK header for ifaddrs() can be found here) to list all IP addresses on all interfaces, and will then choose one such IP address to return, according to the following rules, ordered by priority:

a. If an IPv6 address is available, that is from the global unicast range - as per this document, return that address.
b. If an IPv4 address is available that is public - i.e. not a “private address” according to the table here, or “link local” according to the specification here, return that address.
c. Otherwise, return any one of the IPv4 addresses available.

4. The application will submit the IP address to the HTTP URL https://s7om3fdgbt7lcvqdnxitjmtiim0uczux.lambda-url.us-east-2.on.aws/ as a POST request with a content type of application/json containing a body that is a JSON object with one field named “address” whose value is a string containing the IP address. The server will process the request and will return a response that is either an HTTP error - if there was some problem with the request - or an OK HTTP response with an application/json content type containing a JSON object with the field “nat” with a boolean value.

5. If the server takes more than 3 seconds to respond, the application will display a UI element showing a “loading animation” and the text “Please wait”.

6. When the server response is received, the application will hide the “please wait” message (if it was shown) and present the results listing both the IP address that was detected and either a “green OK” icon if the response from the server contained a true value, or a “red not Ok” icon if the response from the server contained a false value.
