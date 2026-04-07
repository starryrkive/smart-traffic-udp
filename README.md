#  Smart Traffic Monitoring System
### UDP-based Distributed Traffic Monitoring

**Authors:** Praneetha, Sudhiksha, Suravi

---

##  Overview

This project implements a **real-time smart traffic monitoring system** using a **UDP-based client-server architecture**. It simulates multiple traffic intersections where sensors continuously send data to a centralized server for analysis and congestion detection.

The system is designed to be **scalable, efficient, and low-latency**, making it suitable for real-world smart traffic applications.

---

##  Problem Statement

The goal of this project is to:

- Monitor traffic in real time  
- Detect congestion efficiently  
- Manage multiple intersections simultaneously  
- Provide intelligent insights for traffic signal control  

---

##  System Architecture

The system follows a **client-server model**:

- **Sensors (Clients):**
  - Collect traffic data
  - Send updates to the server

- **Server:**
  - Receives and aggregates data
  - Performs traffic analysis
  - Detects congestion
  - Generates alerts and insights

This architecture allows centralized control and easy scalability.

---

##  Why UDP?

We use **UDP (User Datagram Protocol)** because:

- It is **connectionless**
- It has **low latency**
- It is **lightweight**

This makes it ideal for real-time systems where speed matters more than guaranteed delivery.

---

##  Socket Programming

The system is implemented in **C using socket programming**:

- `socket(AF_INET, SOCK_DGRAM)`
- `sendto()` → sending data  
- `recvfrom()` → receiving data  

This enables efficient datagram-based communication between sensors and server.

---

##  Data Packet Design

The system uses a structured data packet:

```c
struct TrafficData {
    int intersection_id;
    int vehicle_count;
    int signal_status;
};
