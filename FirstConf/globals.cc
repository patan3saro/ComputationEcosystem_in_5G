/**
 * globals.h - Global variables and configuration for the simulation
 *
 * This file defines global variables for configuration parameters,
 * addressing, computation capacities, and simulation settings
 * that are shared across different components of the simulation.
 */

#ifndef NS3_GLOBALS_H
#define NS3_GLOBALS_H

#include "ns3/ipv4-address.h"
#include <string>

namespace ns3 {

/**
 * Strategy and output configuration
 */
// Path for output CSV files
extern std::string output_data_csv;
// Selected offloading strategy ("Random", "VCCFirst", "EC_and_CC", "VCC_and_CC")
extern std::string chosen_strategy;
// Simulation scenario type identifier
extern std::string sim_type;

/**
 * Simulation parameters
 */
// Request generation rate in milliseconds
extern double request_rate;
// Number of vehicles in the simulation
extern uint32_t cars_number;
// Number of pedestrians in the simulation
extern uint32_t pedestrians_number;
// Maximum queue length in vehicles
extern uint32_t vehicle_queue_length;
// Size of packets in bytes
extern uint32_t packet_size;
// Workload for each task in millions of instructions
extern double task_workload;

/**
 * Network addresses
 */
// Cloud server IP address
extern Ipv4Address cloud_ip;
// gNodeB (Edge server) IP address
extern Ipv4Address gNb_ip;

/**
 * Computation capacities (in MIPS - Million Instructions Per Second)
 */
// Cloud server computation capacity
extern double cloud_computation_capacity;
// Edge server computation capacity
extern double edge_computation_capacity;
// Vehicle computation capacity
extern double car_computation_capacity;

} // namespace ns3

#endif // NS3_GLOBALS_H
