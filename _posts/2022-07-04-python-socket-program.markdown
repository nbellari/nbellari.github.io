---
layout: post
title: A simple python socket program
categories: python programming
date:  2022-07-04 15:23:05 +0530
---

I wrote a simple python socket program just to get a feel of it. Basically I wanted to see how the socket API looks like. As it turns out, it looks very similar to the C socket API. Here is the code:

```python
import socket
import sys
import argparse

def be_a_client(host, port):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print("Client socket created successfully")
    except socket.error as err:
        print("Client socket creation failed: %s" % err)
        sys.exit(1)

    # Name resolution not actually required, it is taken care of
    try:
        ip = socket.gethostbyname(host)
    except socket.gaierror:
        print("Error resolving hostname")
        sys.exit(1)

    try:
        s.connect((ip, port))
        print("Connected successfully to " + host + " at port: " + str(port))
    except socket.error as err:
        print("Failed to connect: %s" % err)
        sys.exit(1)

    msg = s.recv(256)
    print("Received message from server: " + msg.decode())

    # At the end, close the connection
    s.close()


def be_a_server(port):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print("Server socket created successfully!")
    except socket.error as err:
        print("Server socket creation failed: %s" % err)
        sys.exit(1)

    try:
        s.bind(("localhost", port))
    except:
        print("Failed to bind to socket")
        sys.exit(1)

    s.listen(5)

    while True:
        c, addr = s.accept()
        print("Got connection from %s:%d" % (addr[0], addr[1]))

        c.send("Hi, this is the server speaking!".encode())
        c.close()


# Initialize the argumments
parser = argparse.ArgumentParser(description="Python Socket demonstration")
mode_group = parser.add_mutually_exclusive_group()
mode_group.add_argument("-c", "--client", action="store_true", help="Start in client mode")
mode_group.add_argument("-s", "--server", action="store_true", help="Start in server mode")
parser.add_argument("port", type=int, help="port to connect to or listen on")

args = parser.parse_args()

if (args.client):
    be_a_client("localhost", args.port)
elif (args.server):
    be_a_server(args.port)
else:
    print("should specify client or server")
```

It should also be noted that the `socket` module provides a `create_server` function which does {create socket, bind to poart and listen} in a single step.

A couple of things to note in this code are: the main socket calls are kept in the try/except block to catch any exceptions and the message that gets sent and received are a byte array and not a string - so one has to use `encode()` and `decode()` functions to convert between each other.
 
It was a good refresher on the `ArgumentParser` as well. It is a very elegant way to handle arguments in python code.
