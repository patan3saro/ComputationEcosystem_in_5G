#include "globals.h"


namespace ns3 {

	//strategy and configuration
	std::string output_data_csv;
	std::string chosen_strategy;
	//params
	double request_rate=1000;
	uint32_t cars_number=0;
	uint32_t pedestrians_number=0;
	uint32_t vehicle_queue_length=0;
	uint32_t packet_size;
	
	//addresses
	Ipv4Address cloud_ip("0.0.0.0");
	Ipv4Address gNb_ip("0.0.0.0");
	
	//Computation Capacities
	double cloud_computation_capacity=80000;
	double edge_computation_capacity=27650;
	double car_computation_capacity=1925;
	double task_workload;
	std::string sim_type;



	
} 
