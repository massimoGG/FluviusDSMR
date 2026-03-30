# DSMR P1 Port Reader

![Alt text](/Grafana.png?raw=true "Grafana Dashboard")

## Overview

This is a personal project aimed at reading the serial DSMR P1 port of my Fluvius (Belgium) digital electricity meter. The project is developed in C and serves as a proof of concept for future implementations. The ultimate goal is to transition this project to an STM32 microcontroller with Ethernet capabilities, allowing for more robust and efficient data handling.

## Project Description

The DSMR (Dutch Smart Meter Requirements) P1 port provides access to real-time data from digital electricity meters. This project focuses on reading and processing the data transmitted through this port, enabling users to monitor their electricity consumption and other relevant metrics.

### Current Implementation

- **Platform**: Raspberry Pi
- **Language**: C
- **Functionality**: 
  - Reads data from the DSMR P1 port.
  - Parses the data and convert to line protocol
  - Line protocol is then sent to InfluxDB

### Future Plans

- Transition the project to an STM32 microcontroller.
- Implement Ethernet connectivity for remote data access and monitoring.
- Enhance the functionality to include additional features such as data visualization and alerting.

## Getting Started

### Prerequisites

To run this project, you will need:

- A Raspberry Pi (or compatible device).
- A Fluvius digital electricity meter with a DSMR P1 port.
- A serial connection to the meter. (I bought this one: https://www.sossolutions.nl/slimme-meter-kabel-p1-kabel-3-meter)
- An InfluxDB server


/FLU5\253769484_A

0-0:96.1.4(50217)
0-0:96.1.1(3153414733313030373736353933)
0-0:1.0.0(260330174629S)
1-0:1.8.1(001209.869*kWh)
1-0:1.8.2(001126.192*kWh)
1-0:2.8.1(001040.710*kWh)
1-0:2.8.2(000468.914*kWh)
0-0:96.14.0(0001)
1-0:1.4.0(00.160*kW)
1-0:1.6.0(260324190000W)(03.506*kW)
0-0:98.1.0(7)(1-0:1.6.0)(1-0:1.6.0)(231101000000W)(632525252525W)(00.000*kW)(251001000000S)(250913201500S)(02.508*kW)(251101000000W)(251025123000S)(02.567*kW)(251201000000W)(251106180000W)(02.918*kW)(260101000000W)(251226153000W)(03.743*kW)(260201000000W)(260111151500W)(04.563*kW)(260301000000W)(260211100000W)(03.271*kW)
1-0:1.7.0(00.560*kW)
1-0:2.7.0(00.000*kW)
1-0:21.7.0(00.197*kW)
1-0:41.7.0(00.363*kW)
1-0:61.7.0(00.000*kW)
1-0:22.7.0(00.000*kW)
1-0:42.7.0(00.000*kW)
1-0:62.7.0(00.000*kW)
1-0:32.7.0(229.7*V)
1-0:52.7.0(229.5*V)
1-0:72.7.0(230.3*V)
1-0:31.7.0(001.05*A)
1-0:51.7.0(002.25*A)
1-0:71.7.0(000.00*A)
0-0:96.3.10(1)
0-0:17.0.0(999.9*kW)
1-0:31.4.0(999*A)
0-0:96.13.0()
0-1:24.1.0(003)
0-1:96.1.1(37464C4F32313231313030383534)
0-1:24.4.0(1)
0-1:24.2.3(260330174556S)(03806.070*m3)
!6797

