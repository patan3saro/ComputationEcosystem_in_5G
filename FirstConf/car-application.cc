#include "ns3/log.h"
#include <ns3/log.h>
#include "ns3/simulator.h"
#include "car-application.h"
#include "car-data-tag.h"
#include "ns3/random-variable-stream.h"
#include "ns3/core-module.h"
#include "globals.h"
#include "ns3/nr-module.h"
#include "packet-data-tag.h"


#define RED_CODE "\033[91m"
#define GREEN_CODE "\033[32m"
#define END_CODE "\033[0m"


namespace ns3


{
 
  NS_LOG_COMPONENT_DEFINE("CarApplication");
  NS_OBJECT_ENSURE_REGISTERED(CarApplication);

TypeId 
CarApplication::GetTypeId()
{
    static TypeId tid = TypeId("CarApplication")
                            .AddConstructor<CarApplication>()
                            .SetParent<Application>()                
                            .AddAttribute ("Interval", "Broadcast Interval",
                      TimeValue (MilliSeconds(100)),
                      MakeTimeAccessor (&CarApplication::m_broadcast_time),
                      MakeTimeChecker()
                      )
                ;
    return tid;
}

TypeId 
CarApplication::GetInstanceTypeId() const
{
    return CarApplication::GetTypeId();
}

CarApplication::CarApplication()
{
    m_broadcast_time = MilliSeconds (100); //every 100ms
    m_packetSize = 1000; //x bytes
    m_time_limit = Seconds (0.5);
    //setting length of the queue
	m_car_queue->SetMaxSize(QueueSize(std::to_string(vehicle_queue_length)+"p"));
	m_port0 = 9999;
    m_port1 = 7777;
    m_port_beacon=8888;
	m_sync=true;
    
}
CarApplication::~CarApplication()
{
}

/*void CarApplication::DlCtrlSinr(uint16_t m_cellId, uint16_t m_rnti, double m_avgSinr, uint16_t m_bwpId, uint8_t streamId){
	
	std::cout << "DlCtrlSinr " <<10 * log(m_avgSinr) / log(10)  << "dB" << "Node: "<< m_rnti <<std::endl;
}*/


void CarApplication::DlDataSinr(uint16_t m_cellId, uint16_t m_rnti, double m_avgSinr, uint16_t m_bwpId, uint8_t streamId){

	//I create a file needed to calculate the failure rate caused by the vehicles 
	std::ofstream myfile;
	myfile.open (output_data_csv+"/total_data_SINR_received_from_edge_packets_"+sim_type+".csv", std::ofstream::app);
	myfile <<m_nodeId<<","<<m_cellId<<","<<m_rnti<<","<<10 * log10(m_avgSinr)<<","<<m_bwpId<<"," <<","<<streamId<<","<< Now().GetSeconds()<<"\n";
	myfile.close();
	
}

void CarApplication::SetupReceiveSocket(Ptr<Socket> socket, uint16_t port)
{
InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), port);
if (socket->Bind(local) == -1)
{
NS_FATAL_ERROR("Failed to bind socket");
}
}

void
CarApplication::StartApplication()
{
	NS_LOG_FUNCTION (this);
	//Receive sockets
	TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
	m_recv_socket1 = Socket::CreateSocket(GetNode(), tid);
	m_nodeId = 	GetNode()->GetId();

	SetupReceiveSocket(m_recv_socket1, m_port1);

	m_recv_socket1->SetRecvCallback(MakeCallback(&CarApplication::ReceivePacket, this));
	
	Ptr<NrUeNetDevice> m_ueNetDevice =DynamicCast <NrUeNetDevice> ( GetNode()->GetDevice(0)); 
	
	
		
	Ptr<NrUePhy> phy = m_ueNetDevice->GetPhy(0);
	//phy->TraceConnectWithoutContext ("DlCtrlSinr", MakeCallback(&CarApplication::DlCtrlSinr, this));
	phy->TraceConnectWithoutContext ("DlDataSinr", MakeCallback(&CarApplication::DlDataSinr, this));
        

	
	

	//Send Socket
	m_send_socket = Socket::CreateSocket(GetNode(), tid);
    
	//Let's create a bit of randomness with the first broadcast packet time to avoid collision
	Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable> ();
	Time random_offset = MicroSeconds (rand->GetValue(50,200));
	Simulator::Schedule (m_broadcast_time+random_offset, &CarApplication::BeaconInformation, this, m_sync);
}

void CarApplication::HandleDequeue(){
	
	Ptr <const Packet> packet_tmp0 = m_car_queue->Peek();
	
	Ptr <Packet> packet_tmp1 =  Create<Packet> (4000);	
	
	
	if (packet_tmp0)
	{

		    double elaboration_time = 0;
		    Ipv4Address destination;

		    PacketDataTag tag;
		    if (packet_tmp0->PeekPacketTag (tag))
		    {   
			elaboration_time=tag.GetWorkload()/car_computation_capacity;
			destination=tag.GetSourcePedestrianIPv4Address();
			
		    	tag.SetElaborationTime(Seconds(elaboration_time));
		    	tag.SetQueueingTime(Seconds(Now().GetSeconds()));
				//1 means yes
				tag.SetIsFromVCCToPed(1);
			packet_tmp1->AddPacketTag (tag);

			NS_LOG_INFO ("Car schedule send() message to pedestrian");
		    Simulator::Schedule (Seconds(elaboration_time),&CarApplication::SendPacket,this, packet_tmp1, destination, m_port0);
		    Simulator::Schedule(Seconds(elaboration_time), &ns3::DropTailQueue<Packet >::Dequeue, m_car_queue);
			//Schedule async next broadcast after computation to signal that resources are free
			bool synchronous=false;
			Simulator::Schedule (Seconds(elaboration_time), &CarApplication::BeaconInformation, this, synchronous);
		    //calling recursively till the queue at the car of the VCC is non empty
	        Simulator::Schedule(Seconds(elaboration_time), &CarApplication::HandleDequeue, this);
		    }
		    else 
		    {
			NS_LOG_INFO("Problem with tag in a car in function HandleDequeue()");
		    } 


	    
	    } 
	    else
	    {
	            NS_LOG_INFO("No packet in a car in function HandleDequeue()");
	    } 
}

void
CarApplication::ReceivePacket (Ptr<Socket> socket)
{  
	Ptr<Packet> packet;
	Address from;
		
	while ((packet = socket->RecvFrom(from)))
	{
		NS_LOG_INFO ("Car ReceivePacket() : Node " << GetNode()->GetId() << " : Received a packet of Size:" << packet->GetSize());
		
		//checking the integrity of the packet
		
		Ptr<ErrorModel> rem = CreateObject<RateErrorModel> ();

		if (rem->IsCorrupt (packet) or !packet)
		{
			NS_LOG_INFO("Packet is corrupt in a car");
		} 
		
		// I create a new packet to avoid problems with pointers

		Ptr <Packet> packet_tmp = Create<Packet> (packet->GetSize());
		
		packet_tmp->RemoveAllPacketTags();
		
		PacketDataTag tag;
		if (packet->PeekPacketTag (tag))
		{   

			tag.SetUplinkTime(Seconds(Now().GetSeconds()));
			packet_tmp->AddPacketTag (tag);
			
			//I create a file needed to calculate the failure rate caused by the vehicles 
			std::ofstream myfile;
			myfile.open (output_data_csv+"/total_VCC_received_from_edge_packets_"+sim_type+".csv", std::ofstream::app);
			if (packet_tmp->PeekPacketTag (tag))
			{

				myfile <<tag.GetNodeId()<<","<<packet_tmp->GetSize()<<","<<tag.GetInitialTime()<<","<<				               Now().GetSeconds()<<","<<tag.GetWhere()<<","<<request_rate<<","<<cars_number<<","<<pedestrians_number<<","<<edge_computation_capacity
				<<","<<car_computation_capacity<<","<<tag.GetWorkload()<<","<<tag.GetCarsInVCC()<<","<<vehicle_queue_length<<"\n";
			}
			myfile.close();
		    
		}
		else {
			NS_LOG_INFO("Problem with tag in a car in function ReceivePacket()");
		} 


		if (m_car_queue->GetNPackets() == 0){

			Simulator::ScheduleNow(&ns3::DropTailQueue<Packet >::Enqueue, m_car_queue, packet_tmp);

			Simulator::ScheduleNow(&CarApplication::HandleDequeue, this);

		}
		else if (m_car_queue->GetNPackets() > 0 and m_car_queue->GetNPackets() <=m_car_queue->GetMaxSize().GetValue()-1){
			
			Simulator::ScheduleNow(&ns3::DropTailQueue<Packet >::Enqueue, m_car_queue, packet_tmp);
			Simulator::Schedule(MilliSeconds (5),&CarApplication::BeaconInformation, this, false);
		}
		else {
			NS_LOG_INFO("A packet is sent with the full queue and it is deleted");

			//I create a file needed to calculate the failure rate caused by the vehicles 
			std::ofstream myfile;
			myfile.open (output_data_csv+"/total_VCC_rejected_packets_cause_full_queue_"+sim_type+".csv", std::ofstream::app);
			if (packet_tmp->PeekPacketTag (tag))
			{

				myfile <<tag.GetNodeId()<<","<<packet_tmp->GetSize()<<","<<tag.GetInitialTime()<<","<<				               Now().GetSeconds()<<","<<tag.GetWhere()<<","<<request_rate<<","<<cars_number<<","<<pedestrians_number<<","<<edge_computation_capacity
				<<","<<car_computation_capacity<<","<<tag.GetWorkload()<<","<<tag.GetCarsInVCC()<<","<<vehicle_queue_length<<"\n";
				
			}

			else 
			{

				NS_LOG_INFO("ERROR: in function  ReceivePacket() in tag");
			}
			myfile.close();
		} 


		  

	 	
	   }
}


void CarApplication::SendPacket(Ptr<Packet> packet, Ipv4Address destination, uint16_t port)
{
	NS_LOG_FUNCTION (this << packet << destination << port);
	m_send_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(destination), port));
	m_send_socket->Send(packet);
}

void
CarApplication::BeaconInformation(bool sync) 
{
	NS_LOG_FUNCTION (this);
	//check if the vehicle has available resources
	//and sends beacon just if resources available
	if (m_car_queue->GetNPackets()<m_car_queue->GetMaxSize().GetValue()){
		//Setup transmission parameters
		Ptr<Packet> packet = Create <Packet> (m_packetSize);
		//let's attach our car data tag to it
		CarDataTag tag;
		tag.SetNodeId ( GetNode()->GetId());
		tag.SetPosition ( GetNode()->GetObject<MobilityModel>()->GetPosition());
		tag.SetQueueState(m_car_queue->GetNPackets());
		tag.SetQueueMaxPackets(m_car_queue->GetMaxSize().GetValue());
		//timestamp is set in the default constructor of the CarDataTag class as Simulator::Now()

		//attach the tag to the packet
		packet->AddPacketTag (tag);


		std::ofstream myfile;
		myfile.open (output_data_csv+"/total_VCC_beacons_"+sim_type+".csv", std::ofstream::app);
myfile <<tag.GetNodeId()<<","<<packet->GetSize()<<","<<tag.GetPosition()<<","<<	Now().GetSeconds()<<","<<tag.GetQueueState()<<","<<request_rate<<","<<cars_number<<","<<pedestrians_number<<","<<edge_computation_capacity
				<<","<<car_computation_capacity<<","<<tag.GetQueueMaxPackets()<<","<<vehicle_queue_length<<"\n";
		myfile.close();

		//beaconing
		Simulator::ScheduleNow (&CarApplication::SendPacket, this, packet,  gNb_ip, m_port_beacon);
	}
	if (sync){
	//Schedule next broadcast 
	Simulator::Schedule (m_broadcast_time, &CarApplication::BeaconInformation, this, sync);
	}
}
}//end of ns3