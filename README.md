# lora-mesh

|Folder                                                       |Description                                                                 |
|-------------------------------------------------------------|----------------------------------------------------------------------------|
|[01_examples_to_test_hardware](/01_examples_to_test_hardware)|Example sketches to test the functionality of the T-Beams. Sources included.|
|[02_random_data_to_db](/02_random_data_to_db)                |Sending formatted random GPS payloads from a LoRa Node (LN) to a Gateway (GW) which is connected to a Socket Server and a Database.|
|[03_testing_protocol_functions](/03_testing_protocol_functions)|Testing functions used in the LoRaDiMe Protocol		   |

## Table of Contents

- [Intro](#intro)
- [Use Cases](#use-cases)


## Intro

This repository contains all of the sketches and files that I used and tested while trying to come up with a *Mesh Routing Algorithm* using the LoRa PHY Protocol. 

The goal of this project is to implement an *[RPL](https://tools.ietf.org/html/rfc6550)*-like routing algorithm, with the use of **8 LilyGO TTGO T-Beams v0.7**. 

One of the T-Beams is a **Gateway Node (GN)** and is connected to the remaining seven T-Beams via LoRa and a **Socket Server (SS)** via WiFi.

The Socket Server will then forward the **payloads**  of each of the seven T-Beams, or **LoRa Nodes (LN)**, to a database via a POST request to an Express Server. The Express Server then handles inserting the JSON data (*the LN payloads*) via a query into a PostgreSQL database.

**The payload contains the following information about each LN:**

- nodeID
- number of (LoRa) hops
- latitude (GPS data) 
- longitude (GPS data) 
- date and time 

## Use Cases

A system like this can be used to **track** certain **assets** like containers, hardware equipment and cars. It can easily be extended to share sensor data about an asset like monitoring its temperature or detecting tampering.

#### What about Star Topology?

Of course, asset tracking solutions already make use of LoRa and LoRaWAN using a star topology architecture with LoRaWAN gateways and mobile end devices.

This project is going to be a research into *how far* a network can be *extended* spacially, when its nodes have the ability to **relay/route** messages to a gateway. This also implies, that the particular node is **out of reach** of a gateway and therefore relies on other network participants within its reach to forward messages.

Other mesh routing projects using LoRa like **[Meshtastic](https://github.com/meshtastic/Meshtastic-device)** and **[Disaster.Radio](https://github.com/sudomesh/disaster-radio)** suggest that the performance of this kind of network is limited to a small number of devices. 

However, increasing the reach of a T-Beam by routing its LoRa packets with two or three hops could already cover an area of multiple square kilometers. This could mean having an **emergency routing mechanism** in case of nodes not being within reach of a gateway and **fewer expenses for gateways**.
