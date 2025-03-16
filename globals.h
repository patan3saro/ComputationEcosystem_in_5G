#ifndef GLOBAL_DATA
#define GLOBAL_DATA

#include "ns3/core-module.h"
#include "ns3/ipv4-header.h"
#include "ns3/mac48-address.h"
#include <time.h>

namespace ns3
{
	//strategy and configuration
	extern std::string output_data_csv;
	extern std::string chosen_strategy;
	//params
	extern double request_rate;
	extern uint32_t cars_number;
	extern uint32_t pedestrians_number;
	extern uint32_t vehicle_queue_length;
	extern uint32_t packet_size;

	//addresses
	extern Ipv4Address cloud_ip;
	extern Ipv4Address gNb_ip;
	
	//Computation capacities
	extern double cloud_computation_capacity;
	extern double edge_computation_capacity;
	extern double car_computation_capacity;
	extern double task_workload;
	extern std::string sim_type;


}

#endif 
