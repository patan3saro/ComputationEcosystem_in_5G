# NS-3 Task Offloading Simulation Framework

This framework implements a task offloading simulation for edge computing scenarios in the NS-3 network simulator. It models the interactions between pedestrians, edge servers, vehicles (Vehicular Cloud Computing - VCC), and cloud servers to evaluate different offloading strategies and their performance.

## Overview

The simulation framework models a scenario where pedestrians generate computation tasks that can be processed locally, at an edge server, in vehicles (VCC), or in a remote cloud. The goal is to evaluate different offloading strategies in terms of latency, energy efficiency, and resource utilization.

## Components

### 1. Globals (globals.h / globals.cc)

**Purpose**: Defines global configuration parameters shared across all components of the simulation.

**Key Elements**:
- Strategy configuration (`chosen_strategy`)
- Simulation parameters (request rate, number of vehicles/pedestrians)
- Network addresses (cloud and edge IPs)
- Computation capacities for each node type (cloud, edge, vehicle)

### 2. PacketDataTag (packet-data-tag.h / packet-data-tag.cc)

**Purpose**: Implements a custom tag attached to packets to carry metadata about tasks.

**Key Elements**:
- Task workload in millions of instructions
- Timestamps for tracking task lifecycle (creation, processing, completion)
- Location indicators (where task was processed)
- Source addressing information
- Queue state information

### 3. CarDataTag (car-data-tag.h / car-data-tag.cc)

**Purpose**: Implements a custom tag used for vehicle beacon messages.

**Key Elements**:
- Vehicle position
- Queue state information (available capacity)
- Node identifier
- Timestamps

### 4. PedApplication (pedestrians-application.h / pedestrians-application.cc)

**Purpose**: Models pedestrian nodes that generate offloading requests.

**Key Elements**:
- Periodically generates task offloading requests following an exponential distribution
- Processes responses from computing nodes
- Logs performance metrics (response time, processing time)

### 5. CarApplication (car-application.h / car-application.cc)

**Purpose**: Models vehicle nodes that can process offloaded tasks.

**Key Elements**:
- Processes offloaded tasks based on vehicle computing capacity
- Maintains a queue for pending tasks
- Broadcasts beacons with vehicle status and queue information
- Manages neighbors list

### 6. EdgeApplication (edge-application.h / edge-application.cc)

**Purpose**: Models the edge server that coordinates offloading decisions.

**Key Elements**:
- Implements various offloading strategies (Random, VCCFirst, etc.)
- Maintains a list of available vehicle nodes
- Can process tasks locally or forward them to cloud/vehicles
- Tracks vehicle entering/leaving communication range

### 7. CloudApplication (cloud-application.h / cloud-application.cc)

**Purpose**: Models the remote cloud server with high computation capacity.

**Key Elements**:
- Processes offloaded tasks based on cloud computing capacity
- Returns results to source pedestrians
- Typically has highest latency but also highest computation power

## Offloading Strategies

1. **Random**: Randomly selects between Edge, VCC, and Cloud for task processing.
2. **VCCFirst**: Prioritizes VCC, then Edge, then Cloud (if resources available).
3. **EC_and_CC**: Uses only Edge and Cloud resources (no VCC).
4. **VCC_and_CC**: Uses only VCC and Cloud resources (no Edge).

## Data Flow

1. Pedestrian generates task request with PacketDataTag containing workload information.
2. Request is sent to Edge server.
3. Edge server applies selected offloading strategy to decide where to process the task.
4. Task is processed either:
   - Locally at the Edge
   - Forwarded to an available vehicle (VCC)
   - Forwarded to the Cloud
5. Processing node computes the task based on workload and computation capacity.
6. Result is sent back to the originating pedestrian.
7. Metrics are collected for analysis.

## Vehicle Discovery

1. Vehicles periodically broadcast beacon messages with their position and available resources.
2. Edge server maintains a list of vehicles in range with available processing capacity.
3. Vehicles are removed from the list when:
   - They leave communication range
   - Their queue is full
   - They are selected for task processing

## Metrics Collection

The simulation collects various metrics in CSV files:
- Offloading time (end-to-end delay)
- Number of tasks processed by each paradigm
- Vehicle dwell time in communication range
- Queue states at different nodes
- SINR (Signal-to-Interference-plus-Noise Ratio) for link quality

## Integration with NS-3

The simulation leverages NS-3's capabilities:
- Network models (5G NR for communication)
- Mobility models (for pedestrians and vehicles)
- Packet management
- Discrete event simulation

## How to Use

1. Configure simulation parameters in a main file (not shown here).
2. Choose an offloading strategy.
3. Run the simulation using NS-3.
4. Analyze the output CSV files for performance metrics.

## Dependencies

- NS-3 simulator (version 3.35 or higher recommended)
- C++ compiler with C++14 support
