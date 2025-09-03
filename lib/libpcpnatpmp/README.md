Port Control Protocol (PCP) and NAT-PMP client library
======================================================

Library implements client side of PCP
([RFC 6887](https://datatracker.ietf.org/doc/html/rfc6887)) and
NAT-PMP ([RFC 6886](https://datatracker.ietf.org/doc/html/rfc6886)) protocols.
Switch to NAT-PMP is done automatically by version negotiation. This library
enables any network application to manage network edge device (e.g. to create
NAT mapping or ask router for specific flow treatment).

Supported platforms are 
Linux, Microsoft Windows (Vista and later) and macOS.

Components
----------

  - [lib](lib)                 - Client library
  - [cli-client](cli-client)   - Command-line interface client
  - [test-server](test-server) - Test server
  - [scapy](scapy)             - PCP layer for Scapy

Build instructions are located in [INSTALL.md](INSTALL.md) file.
More information about components are in each subdirectory's README.md file.
