/**
 * globals.cc - Implementation of global variables for the simulation
 *
 * This file contains the actual definitions of the global variables
 * declared in globals.h. These variables are used for configuration,
 * addressing, and simulation parameters across different components.
 */

#include "globals.h"

namespace ns3 {

//---------- Strategy and configuration ----------
// Path for output CSV files
std::string output_data_csv;

// Selected offloading strategy ("Random", "VCCFirst", "EC_and_CC", "VCC_and_CC")
std::string chosen_strategy;

// Simulation scenario type identifier
std::string sim_type;

//---------- Simulation parameters ----------
// Request generation rate in milliseconds
double request_rate = 1000;

// Number of vehicles in the simulation
uint32_t cars_number = 0;

// Number of pedestrians in the simulation
uint32_t pedestrians_number = 0;

// Maximum queue length in vehicles
uint32_t vehicle_queue_length = 0;

// Size of packets in bytes
uint32_t packet_size;

// Workload for each task in millions of instructions
double task_workload;

//---------- Network addresses ----------
// Cloud server IP address
Ipv4Address cloud_ip = Ipv4Address("0.0.0.0");

// gNodeB (Edge server) IP address  
Ipv4Address gNb_ip = Ipv4Address("0.0.0.0");

//---------- Computation capacities ----------
// Cloud server computation capacity (in MIPS)
double cloud_computation_capacity = 80000;

// Edge server computation capacity (in MIPS)
double edge_computation_capacity = 27650;

// Vehicle computation capacity (in MIPS)
double car_computation_capacity = 1925;

} // namespace ns3
