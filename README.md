# Mini Network Traffic Analyzer

A Linux-based network traffic analyzer written in C using raw sockets.

The analyzer captures live network traffic directly from the network interface, parses Ethernet, IPv4, TCP, and UDP headers, tracks active connections, and displays real-time traffic statistics.

---

## Features

- Raw packet capture using AF_PACKET sockets
- Ethernet frame parsing
- IPv4 packet parsing
- TCP packet analysis
- UDP packet analysis
- Active connection tracking
- Packet and byte statistics
- First-seen and last-seen timestamps
- Real-time terminal dashboard

---

## Concepts Used

This project demonstrates:

- Raw Sockets
- Ethernet Frames
- IPv4 Header Processing
- TCP Header Processing
- UDP Header Processing
- Network Byte Order Conversion
- Connection Tracking
- Linux Networking APIs
- Systems Programming in C

---

## Architecture

```
Network Interface Card (NIC)
            │
            ▼
      Raw Socket
(AF_PACKET, SOCK_RAW)
            │
            ▼
        recvfrom()
            │
            ▼
      Ethernet Header
            │
            ▼
         IP Header
            │
            ▼
    TCP / UDP Header
            │
            ▼
   Connection Tracking
            │
            ▼
      Statistics Engine
            │
            ▼
      Terminal Dashboard
```

---

## Protocol Support

| Protocol | Protocol ID |
|-----------|------------|
| TCP | 6 |
| UDP | 17 |

Currently the analyzer processes IPv4 TCP and UDP traffic.

---

## Connection Information Tracked

For each connection:

- Source IP
- Destination IP
- Source Port
- Destination Port
- Protocol ID
- Total Packets
- Total Bytes
- First Seen Timestamp
- Last Seen Timestamp

---

## Requirements

- Linux
- GCC
- Root privileges (required for raw sockets)

---

## Compilation

```bash
gcc network_analyzer.c -o analyzer
```

---

## Running

```bash
sudo ./analyzer
```

---

## Example Output

```
========================= PROTOCOL TABLE =========================
Protocol ID: 6  | TCP
Protocol ID: 17 | UDP

========================= CONNECTION TABLE =========================

1 - 192.168.1.5:53210 ---> 142.250.183.78:443
Protocol ID: 6
Packets: 154
Bytes: 11842

========================= OVERVIEW BY TCP/UDP =========================

Total Packets : 154
Total Bytes (kB): 11

TCP Total Packets : 154
TCP Total Bytes (kB): 11

UDP Total Packets : 0
UDP Total Bytes (kB): 0
```

---

## Future Improvements

- ICMP support
- Incoming/Outgoing traffic classification
- Packet logging
- Export to CSV
- Protocol filtering
- Bandwidth monitoring
- Port scan detection
- SYN flood detection
- ncurses dashboard
- IPv6 support

---

## Author

Teja Sai Eswar

Indian Institute of Technology Kharagpur (IIT KGP)

Computer Networks and Internet Protocols Project
