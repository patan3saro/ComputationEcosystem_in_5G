#include "ns3/log.h"
#include "edge-application.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/ethernet-header.h"
#include "ns3/arp-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/mobility-module.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include <time.h>
#include "car-data-tag.h"
#include "packet-data-tag.h"
#include "globals.h"
#include <vector>

#define PURPLE_CODE "\033[95m"
#define CYAN_CODE "\033[96m"
#define TEAL_CODE "\033[36m"
#define BLUE_CODE "\033[94m"
#define GREEN_CODE "\033[32m"
#define YELLOW_CODE "\033[33m"
#define LIGHT_YELLOW_CODE "\033[93m"
#define RED_CODE "\033[91m"
#define BOLD_CODE "\033[1m"
#define END_CODE "\033[0m"


namespace ns3
{


  NS_LOG_COMPONENT_DEFINE("EdgeApplication");
  NS_OBJECT_ENSURE_REGISTERED(EdgeApplication);
  
  TypeId
  EdgeApplication::GetTypeId()
  {
    static TypeId tid = TypeId("EdgeApplication")
                            .AddConstructor<EdgeApplication>()
                            .SetParent<Application>();
    return tid;
  }

  TypeId
  EdgeApplication::GetInstanceTypeId() const
  {
    return EdgeApplication::GetTypeId();
  }

  EdgeApplication::EdgeApplication()
  {	
    m_port1 = 7777;
    m_port2 = 8888; 
  }
  EdgeApplication::~EdgeApplication()
  {
  }
  void EdgeApplication::SetupReceiveSocket(Ptr<Socket> socket, uint16_t port)
  {
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), port);
    if (socket->Bind(local) == -1)
    {
      NS_FATAL_ERROR("Failed to bind socket");
    }
  }
  void EdgeApplication::StartApplication()
  {       	

    //Receive sockets
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    m_recv_socket1 = Socket::CreateSocket(GetNode(), tid);
    m_recv_socket2 = Socket::CreateSocket(GetNode(), tid);
    
    SetupReceiveSocket(m_recv_socket1, m_port1);
    SetupReceiveSocket(m_recv_socket2, m_port2);
    
    Ptr <Node> node;
    m_recv_socket1->SetRecvCallback(MakeCallback(&EdgeApplication::HandleReadOne, this));
    m_recv_socket2->SetRecvCallback(MakeCallback(&EdgeApplication::HandleNeighbor, this));



	// Collegare il callback per i pacchetti ricevuti a livello IP
    Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();
    ipv4->TraceConnectWithoutContext ("Rx", MakeCallback (&EdgeApplication::HandleResponseFromVehicle, this));



    //Send Socket
    m_send_socket = Socket::CreateSocket(GetNode(), tid);
  
    NS_LOG_FUNCTION (this);
    
        //We will periodically (every 0.x second) check the list of neighbors, and remove old ones (older than y.z seconds)
    Simulator::Schedule (Seconds (0.1), &EdgeApplication::RemoveOldNeighbors, this);
    
  }

  void EdgeApplication::HandleReadOne(Ptr<Socket> socket)
  {     
    NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address from;
    Address localAddress;

    while ((packet = socket->RecvFrom(from)))
    {	
	NS_LOG_INFO(TEAL_CODE << "HandleReadOne : Edge Received a Packet of size: " << packet->GetSize() << " at time " << Now().GetSeconds()<<  END_CODE);
	NS_LOG_INFO(packet->ToString());
      

	//getting source address of the pedestrian
	InetSocketAddress i_from_addr = InetSocketAddress::ConvertFrom (from);
	Ipv4Address dest_ip = i_from_addr.GetIpv4 ();  

	PacketDataTag tag;
	if (packet->RemovePacketTag (tag))
	{
		tag.SetSourcePedestrianIPv4Address(dest_ip);
		tag.SetCarsInVCC(m_neighbors.size());
		packet->AddPacketTag (tag);
	}

	//HERE THE CHOSEN STRATEGY
	//Random strategy 	
	if (chosen_strategy == "Random"){
		this->RandomStrategy(packet, dest_ip);
	}
	//VCC first strategy
	else if (chosen_strategy == "VCCFirst"){
		bool vcc_active=true;
		bool edge_active=true;
		bool cloud_active=true;
		this->FirstVCCStrategy(packet, dest_ip, vcc_active, edge_active, cloud_active);
	}
	//EC and CC strategy
	else if (chosen_strategy == "EC_and_CC"){
		bool vcc_active=false;
		bool edge_active=true;
		bool cloud_active=true;
		this->FirstVCCStrategy(packet, dest_ip, vcc_active, edge_active, cloud_active);
	}
	//VCC and CC strategy
	else if (chosen_strategy == "VCC_and_CC"){
		bool vcc_active=true;
		bool edge_active=false;
		bool cloud_active=true;
		this->FirstVCCStrategy(packet, dest_ip, vcc_active, edge_active, cloud_active);
	}

    }  

  
  }


  void EdgeApplication::HandleResponseFromVehicle(Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t interface)
  {     
        
	
	//we create a csv file with packet infos in his tag
	PacketDataTag tag;
	if (packet->PeekPacketTag (tag))
	    {
			if (tag.GetIsFromVCCToPed()==1){
				std::ofstream myfile;
				myfile.open (output_data_csv+"/response_offloading_passing_in_gNB_"+sim_type+".csv", std::ofstream::app);
	   
				myfile <<tag.GetNodeId()<<","<<packet->GetSize()<<","<<tag.GetInitialTime()<<","<< Now().GetSeconds()<<","<<tag.GetWhere()<<","<<request_rate<<","<<cars_number<<","<<pedestrians_number<<","<<tag.GetUplinkTime()<<","<<tag.GetElaborationTime()<<","<<edge_computation_capacity
					<<","<<car_computation_capacity<<","<<tag.GetQueueingTime()<<","<<tag.GetWorkload()<<","<<tag.GetCarsInVCC()<<","<<vehicle_queue_length<<"\n";
					myfile.close();}
	    }
	else{
	    
	    	NS_LOG_INFO("EDGE in HandleResponseFromVehicle Received a Packet BUT WITHOUT TAG!!!!");
	    }

    }
 

  

  
void 
EdgeApplication::HandleNeighbor (Ptr<Socket> socket)
{    

	Ptr<Packet> packet;
	Address from;
	while ((packet = socket->RecvFrom(from)))
	    {   
	    	         
		InetSocketAddress i_from_addr = InetSocketAddress::ConvertFrom (from);
		Ipv4Address source = i_from_addr.GetIpv4 ();     
		//Let's see if this packet is intended to this node
		// I update neighbors with available resources
		   
		CarDataTag tag;

		if (packet->PeekPacketTag (tag))
		{

			uint32_t queue_state = tag.GetQueueState();

			if (queue_state<tag.GetQueueMaxPackets() and queue_state>=0){
				UpdateNeighbor (source, tag.GetNodeId(), tag.GetPosition());
			}
			//I remove the neighbors with no available resources in queue
			else if (queue_state>=tag.GetQueueMaxPackets()) {

					for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
					{
						
							if (it->neighbor_ip == source)
							{
								NS_LOG_INFO (RED_CODE << Now () << " Node " << GetNode()->GetId()<<" resourceless neighbor " << it->neighbor_ip <<END_CODE);


								//I use the file to record vehicles which go out of the range of the RSU
								std::ofstream myfile;
								myfile.open (output_data_csv+"/dwell_time_"+sim_type+".csv", std::ofstream::app);

								myfile <<tag.GetNodeId()<<","<<cars_number<<","<<it->neighbor_ip<<","<<Now ()<<","<<"OUT"<<","<<"resourceless"<<","<< tag.GetPosition()<<"\n";

								myfile.close();


								m_neighbors.erase( it );				    
								break;
							}

					}
					       	
			}
		}
		    
		  

		    
	    }
}


void EdgeApplication::UpdateNeighbor (Ipv4Address addr, uint32_t nodeId, Vector position)
{
    bool found = false;
    //Go over all neighbors, find one matching the address, and updates its 'last_beacon' time.
    for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
    {
        if (it->neighbor_ip == addr)
        {
            it->last_beacon = Now();
            found = true;
            break;
        }
    }
    if (!found) //If no node with this address exist, add a new table entry
    {
		NS_LOG_INFO ( GREEN_CODE << Now() << " : Node " << GetNode()->GetId() << " is adding a neighbor with IP="<<addr << END_CODE);
		NeighborInformation new_n;
		new_n.node_ID = nodeId; 
		new_n.neighbor_ip = addr;
		new_n.last_beacon = Now ();
		m_neighbors.push_back (new_n);



		//I add cars in a file needed to calculate the dwell time of vehicles 
		std::ofstream myfile;
		myfile.open (output_data_csv+"/dwell_time_"+sim_type+".csv", std::ofstream::app);
		
		myfile <<nodeId<<","<<cars_number<<","<<addr<<","<<Now ()<<","<<"IN"<<","<<"range"<<","<<position<<"\n";

		myfile.close();

    }
}




void EdgeApplication::RemoveNeighbor (Ipv4Address addr)
{
    //Go over the list of neighbors
    for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
	{	

		if (it->neighbor_ip == addr)
		{            
			NS_LOG_INFO (RED_CODE << Now () << " Node " << GetNode()->GetId()<<" is removing old neighbor because it is used" << it->neighbor_ip <<END_CODE);

			//I use the file to record vehicles which go out of the range of the RSU
			std::ofstream myfile;
			myfile.open (output_data_csv+"/dwell_time_"+sim_type+".csv", std::ofstream::app);
			//the position is not inmportant in this case because the car is out of the list not because of the range
			myfile <<it->node_ID<<","<<cars_number<<","<<it->neighbor_ip<<","<<Now ()<<","<<"OUT"<<","<<"used"<<"\n";

			myfile.close();

			//Remove an old entry from the table
			m_neighbors.erase (it);
			break;    
			
		}
    }
}

void EdgeApplication::PrintNeighbors ()
{
    std::cout << "Neighbor Info for Node: " << GetNode()->GetId() << std::endl;
    for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
    {
        std::cout << "\tMAC: " << it->neighbor_ip << "\tLast Contact: " << it->last_beacon << std::endl;
    }
}

void EdgeApplication::RemoveOldNeighbors ()
{
    //Go over the list of neighbors
    for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
    {
        //Get the time passed since the last time we heard from a node
        Time last_contact = Now () - it->last_beacon;

        if (last_contact >= Seconds (0.5)) //if it has been more than x seconds, we will remove it. You can change this to whatever value you want.
        {
			NS_LOG_INFO (RED_CODE << Now () << " Node " << GetNode()->GetId()<<" is removing old neighbor " << it->neighbor_ip <<END_CODE);


			//I use the file to record vehicles which go out of the range of the RSU
			std::ofstream myfile;
			myfile.open (output_data_csv+"/dwell_time_"+sim_type+".csv", std::ofstream::app);
		
			//Vector m_position = GetNodes (it->node_ID)->GetPosition();

			myfile <<it->node_ID<<","<<cars_number<<","<<it->neighbor_ip<<","<<Now ()<<","<<"OUT"<<","<<"range"<<"\n";

			myfile.close();

			//Remove an old entry from the table
			m_neighbors.erase (it);
			break;
        }    
    }
    //Check the list again after x seconds.
    Simulator::Schedule (Seconds (0.1), &EdgeApplication::RemoveOldNeighbors, this);

}
  
  void EdgeApplication::HandleDequeue (){
  
  	Ptr <const Packet> packet_tmp0 = m_edge_queue->Peek();
	
	Ptr <Packet> packet_tmp1 =  Create<Packet> (4000);
	

	if (packet_tmp0){
		Ipv4Address destination;
		double elaboration_time = 0;

		PacketDataTag tag;
		if (packet_tmp0->PeekPacketTag (tag))
		{
			elaboration_time=tag.GetWorkload()/edge_computation_capacity;

			tag.SetElaborationTime(Seconds(elaboration_time));
			tag.SetQueueingTime(Seconds(Now().GetSeconds()));
			
			destination = tag.GetSourcePedestrianIPv4Address();
			packet_tmp1->AddPacketTag (tag);
		}else
			NS_LOG_INFO("ERROR with TAG in Edge HandleDequeue ()");

		NS_LOG_INFO(YELLOW_CODE << "Packet elaboration time:"  << Seconds (elaboration_time)<< END_CODE);
		uint16_t m_port_ped= 9999;
		Simulator::Schedule(Seconds(elaboration_time), &EdgeApplication::SendPacket, this,  packet_tmp1, destination, m_port_ped);
		Simulator::Schedule(Seconds(elaboration_time), &ns3::DropTailQueue<Packet >::Dequeue, m_edge_queue);
		//calling recursively till the queue at the edge is non empty
		Simulator::Schedule(Seconds(elaboration_time), &EdgeApplication::HandleDequeue, this);
	}else
		NS_LOG_INFO("NO PACKET in Edge queue HandleDequeue ()");

  }


  void EdgeApplication::SendPacket(Ptr<Packet> packet, Ipv4Address destination, uint16_t port)
  {
	NS_LOG_FUNCTION (this << packet << destination << port);
	m_send_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(destination), port));
	m_send_socket->Send(packet);
	NS_LOG_INFO(PURPLE_CODE << "Edge sending Packet of size" << packet->GetSize() << " at time " << Now().GetSeconds() << END_CODE);
	NS_LOG_INFO("Content: " << packet->ToString());
    
  }



void EdgeApplication::RandomStrategy(Ptr<Packet> packet, Ipv4Address dest_ip){


	//before to choose i check if there are resources available in VCC and EC
	//this is not necessary for the cloud because the assumption is that it has infinite processors
	uint16_t m_max_num = 2;
	uint16_t m_min_num = 0;
	

	if (m_edge_queue->GetNPackets()==100){
		//we choose VCC or cloud
		m_min_num = 1;
	
	} else if (m_neighbors.size()<=0){
		//we choose edge or cloud
		m_max_num = 1;
	
	} else if (m_edge_queue->GetNPackets()==100 and m_neighbors.size()<=0){
		//if in EC and VCC there is not resource available we choose always the cloud 
		m_min_num = 1;
		m_max_num = m_min_num;
	}// else we choose between all the paradigms
	
	
	//Offloading strategy
	Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable>();
	int v1=x->GetInteger(m_min_num, m_max_num);
	
	//setting packet infos in the tag
	 PacketDataTag tag;          
	 if (packet->RemovePacketTag (tag))
	  {
		tag.SetWhere(v1);
		packet->AddPacketTag (tag);
	  }
      
	
    
	switch (v1){

	//Edge offloading
	case 0: {
	
 		NS_LOG_INFO(YELLOW_CODE << "Edge Offloading"  << Now().GetSeconds() << END_CODE);
		NS_LOG_INFO(YELLOW_CODE << "Edge packets in queue"  << m_edge_queue->GetNPackets() << END_CODE);
		
		PacketDataTag tag;
		
		if (packet->RemovePacketTag (tag))
		{	//this is valid also as enqueueing instant
			tag.SetUplinkTime(Seconds(Now().GetSeconds()));
			packet->AddPacketTag (tag);
		}
	
		
		if (m_edge_queue->GetNPackets() == 0){
			Simulator::ScheduleNow(&ns3::DropTailQueue<Packet >::Enqueue, m_edge_queue, packet);
			Simulator::ScheduleNow (&EdgeApplication::HandleDequeue, this);
		
		}
		else if (m_edge_queue->GetNPackets() > 0){
		
			Simulator::ScheduleNow(&ns3::DropTailQueue<Packet >::Enqueue, m_edge_queue, packet);
			
		}
		
		  //I create a file needed to calculate the failure rate i.e. satisfied requests over total requests for the EDGE paradigm 
		  std::ofstream myfile;
		  myfile.open (output_data_csv+"/total_EDGE_sent_packets_"+sim_type+".csv", std::ofstream::app);
		  
		myfile <<tag.GetNodeId()<<","<<packet->GetSize()<<","<<tag.GetInitialTime()<<","<<				               Now().GetSeconds()<<","<<tag.GetWhere()<<","<<request_rate<<","<<cars_number<<","<<pedestrians_number<<","<<edge_computation_capacity
		<<","<<car_computation_capacity<<","<<tag.GetWorkload()<<","<<tag.GetCarsInVCC()<<","<<vehicle_queue_length<<"\n";
		myfile.close();
		  
		  break;
	
	}
	//Cloud offloading
	case 1: {
	
		NS_LOG_INFO(YELLOW_CODE << "From Edge to Cloud at time"  << Now().GetSeconds() << END_CODE);
		
		Ipv4Address dest_ip = cloud_ip;
		
		  
		//sending packet to cloud
		Simulator::ScheduleNow (&EdgeApplication::SendPacket, this,  packet, dest_ip, m_port1);
		
		  //I create a file needed to calculate the failure rate i.e. satisfied requests over total requests for the CLOUD paradigm 
		  std::ofstream myfile;
		  myfile.open (output_data_csv+"/total_CLOUD_sent_packets_"+sim_type+".csv", std::ofstream::app);
		  
		  PacketDataTag tag;
		  if (packet->PeekPacketTag (tag))
		    {
		   
			myfile <<tag.GetNodeId()<<","<<packet->GetSize()<<","<<tag.GetInitialTime()<<","<<				               Now().GetSeconds()<<","<<tag.GetWhere()<<","<<request_rate<<","<<cars_number<<","<<pedestrians_number<<","<<edge_computation_capacity
			<<","<<car_computation_capacity<<","<<tag.GetWorkload()<<","<<tag.GetCarsInVCC()<<","<<vehicle_queue_length<<"\n";
			
		    }
		    myfile.close();
	 break;
	}
		//VCC offloading
	case 2: {
		
		 	NS_LOG_INFO(YELLOW_CODE << "From Edge to VC at time"  << Now().GetSeconds() << END_CODE);
			//Setup transmission parameter
			NS_LOG_FUNCTION (this);
			//we take a node random from the list 
			//Offloading strategy
			Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable>();
			int idx=x->GetInteger(0,m_neighbors.size()-1);
			//destination IP
		  	Ipv4Address address_tmp = m_neighbors[idx].neighbor_ip;
		   	 	
			Simulator::ScheduleNow (&EdgeApplication::SendPacket, this, packet, address_tmp, m_port1);
			Simulator::ScheduleNow (&EdgeApplication::RemoveNeighbor, this, address_tmp);
			
	  //I create a file needed to calculate the failure rate i.e. sent requests over total requests for the VCC paradigm 
	  std::ofstream myfile;
	  myfile.open (output_data_csv+"/total_VCC_sent_packets_"+sim_type+".csv", std::ofstream::app);
	  
	  PacketDataTag tag;
	    if (packet->PeekPacketTag (tag))
	    {
	   
		myfile <<tag.GetNodeId()<<","<<packet->GetSize()<<","<<tag.GetInitialTime()<<","<<				               Now().GetSeconds()<<","<<tag.GetWhere()<<","<<request_rate<<","<<cars_number<<","<<pedestrians_number<<","<<edge_computation_capacity
		<<","<<car_computation_capacity<<","<<tag.GetWorkload()<<","<<tag.GetCarsInVCC()<<","<<vehicle_queue_length<<"\n";
		
	    }
	    myfile.close();
	    
	  break;
	}
		

  }  
}

void EdgeApplication::FirstVCCStrategy(Ptr<Packet> packet, Ipv4Address dest_ip, bool vcc_active, bool edge_active, bool cloud_active){


	//VCC offloading
	if (m_neighbors.size()>0 and vcc_active){	
	 	NS_LOG_INFO(YELLOW_CODE << "From Edge to VC at time"  << Now().GetSeconds() << END_CODE);
		//Setup transmission parameter
		NS_LOG_FUNCTION (this);
		
		//we take a node random from the list 
		//Offloading strategy
		Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable>();
		int idx=x->GetInteger(0,m_neighbors.size()-1);
		//destination MAC
	  	Ipv4Address address_tmp = m_neighbors[idx].neighbor_ip;
	  	
	  	
	       //setting packet infos in the tag
	       PacketDataTag tag;
	  
	       if (packet->RemovePacketTag (tag))
	       {
				tag.SetWhere(2);
				packet->AddPacketTag (tag);
	       }
	   	
	  	Simulator::ScheduleNow (&EdgeApplication::SendPacket, this, packet, address_tmp, m_port1);
	  	Simulator::ScheduleNow (&EdgeApplication::RemoveNeighbor, this, address_tmp);

		//I create a file needed to calculate the failure rate i.e. sent requests over total requests for the VCC paradigm 
		std::ofstream myfile;
		myfile.open (output_data_csv+"/total_VCC_sent_packets_"+sim_type+".csv", std::ofstream::app);

		if (packet->PeekPacketTag (tag))
		{

		myfile <<tag.GetNodeId()<<","<<packet->GetSize()<<","<<tag.GetInitialTime()<<","<<				               Now().GetSeconds()<<","<<tag.GetWhere()<<","<<request_rate<<","<<cars_number<<","<<pedestrians_number<<","<<edge_computation_capacity
		<<","<<car_computation_capacity<<","<<tag.GetWorkload()<<","<<tag.GetCarsInVCC()<<","<<vehicle_queue_length<<"\n";
		}

	  myfile.close();
	}
	//Edge offloading
	else if (m_edge_queue->GetNPackets()<m_edge_queue->GetMaxSize().GetValue() and edge_active){
 
		NS_LOG_INFO(YELLOW_CODE << "Edge packets in queue"  << m_edge_queue->GetNPackets() << END_CODE);
		NS_LOG_INFO(YELLOW_CODE << "Edge Offloading"  << Now().GetSeconds() << END_CODE);
		
		PacketDataTag tag;
		
		if (packet->RemovePacketTag (tag))
		{	//this is valid also as enqueueing instant
			tag.SetUplinkTime(Seconds(Now().GetSeconds()));
			tag.SetWhere(0);
			packet->AddPacketTag (tag);
		}
	
		
		if (m_edge_queue->GetNPackets() == 0){
			
			Simulator::ScheduleNow(&ns3::DropTailQueue<Packet >::Enqueue, m_edge_queue, packet);
			Simulator::ScheduleNow (&EdgeApplication::HandleDequeue, this);
		
		}
		else if (m_edge_queue->GetNPackets() > 0){
		
			Simulator::ScheduleNow(&ns3::DropTailQueue<Packet >::Enqueue, m_edge_queue, packet);
			
		}
		
		  //I create a file needed to calculate the failure rate i.e. satisfied requests over total requests for the EDGE paradigm 
		  std::ofstream myfile;
		  myfile.open (output_data_csv+"/total_EDGE_sent_packets_"+sim_type+".csv", std::ofstream::app);
		  
		   
			myfile <<tag.GetNodeId()<<","<<packet->GetSize()<<","<<tag.GetInitialTime()<<","<<				               Now().GetSeconds()<<","<<tag.GetWhere()<<","<<request_rate<<","<<cars_number<<","<<pedestrians_number<<","<<edge_computation_capacity
			<<","<<car_computation_capacity<<","<<tag.GetWorkload()<<","<<tag.GetCarsInVCC()<<","<<vehicle_queue_length<<"\n";
			myfile.close();

	}
	//Cloud offloading
	else if (cloud_active) {
		
		NS_LOG_INFO(YELLOW_CODE << "From Edge to Cloud at time"  << Now().GetSeconds() << END_CODE);
		//setting cloud ip from global variables
		Ipv4Address dest_ip = cloud_ip;
		
		  	
	  	    //setting packet infos in the tag
		    PacketDataTag tag;
		  
		    if (packet->RemovePacketTag (tag))
		    {
			tag.SetWhere(1);
			packet->AddPacketTag (tag);
		    }
		
		//sending packet to cloud
		Simulator::ScheduleNow (&EdgeApplication::SendPacket, this,  packet, dest_ip, m_port1);
		
		  //I create a file needed to calculate the failure rate i.e. satisfied requests over total requests for the CLOUD paradigm 
		  std::ofstream myfile;
		  myfile.open (output_data_csv+"/total_CLOUD_sent_packets_"+sim_type+".csv", std::ofstream::app);
		  
		  if (packet->PeekPacketTag (tag))
		    {
		   
			myfile <<tag.GetNodeId()<<","<<packet->GetSize()<<","<<tag.GetInitialTime()<<","<<				               Now().GetSeconds()<<","<<tag.GetWhere()<<","<<request_rate<<","<<cars_number<<","<<pedestrians_number<<","<<edge_computation_capacity
			<<","<<car_computation_capacity<<","<<tag.GetWorkload()<<","<<tag.GetCarsInVCC()<<","<<vehicle_queue_length<<"\n";
			
		    }
		    
		  myfile.close();
	
	}

}

} // namespace ns3
