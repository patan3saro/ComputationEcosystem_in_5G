#include "ns3/log.h"
#include "pedestrians-application.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/csma-net-device.h"
#include "ns3/ethernet-header.h"
#include "ns3/arp-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/core-module.h"
#include "packet-data-tag.h"
#include "globals.h"

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
  NS_LOG_COMPONENT_DEFINE("PedApplication");
  NS_OBJECT_ENSURE_REGISTERED(PedApplication);

  TypeId
  PedApplication::GetTypeId()
  {
    static TypeId tid = TypeId("ns3::PedApplication")
                            .AddConstructor<PedApplication>()
                            .SetParent<Application>();
    return tid;
  }

  TypeId
  PedApplication::GetInstanceTypeId() const
  {
    return PedApplication::GetTypeId();
  }

  PedApplication::PedApplication()
  {
    m_port0 = 9999;
    m_port1 = 7777;
  }
  PedApplication::~PedApplication()
  {
  }
  void PedApplication::SetupReceiveSocket(Ptr<Socket> socket, uint16_t port)
  {
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), port);
    if (socket->Bind(local) == -1)
    {
      NS_FATAL_ERROR("Failed to bind socket");
    }
  }
  void PedApplication::StartApplication()
  {
    //Receive sockets
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    m_recv_socket0 = Socket::CreateSocket(GetNode(), tid);
    
    
    SetupReceiveSocket(m_recv_socket0, m_port0);


    m_recv_socket0->SetRecvCallback(MakeCallback(&PedApplication::HandleReadOne, this));
    //Send Socket
    m_send_socket = Socket::CreateSocket(GetNode(), tid);

    //Let's create a bit of randomness with the first broadcast packet time to avoid collision
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable> ();
    Time random_offset = MicroSeconds (rand->GetValue(50,200));

    Ptr <Packet> packet0 =  Create<Packet> (packet_size);

    PacketDataTag tag;
    tag.SetWorkload(task_workload);
    tag.SetNodeId (GetNode()->GetId());
    tag.SetInitialTime (random_offset);
    packet0->AddPacketTag (tag);

    Simulator::Schedule (random_offset, &PedApplication::SendPacket, this, packet0, gNb_ip, m_port1);
  }

  void PedApplication::HandleReadOne(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from)))
    {
      uint32_t packetSize=packet->GetSize();
      
      NS_LOG_INFO(TEAL_CODE << "Pedestrian HandleReadOne : Received a Packet of size: " << packetSize << " at time " << Now().GetSeconds() << END_CODE);
      NS_LOG_INFO(packet->ToString());
      
      std::ofstream myfile;
	    myfile.open (output_data_csv+"/offloading_time_"+sim_type+".csv", std::ofstream::app);
	    //we create a csv file with packet infos in his tag
	    PacketDataTag tag;
	    if (packet->PeekPacketTag (tag))
	    {
	   
		myfile <<tag.GetNodeId()<<","<<packetSize<<","<<tag.GetInitialTime()<<","<<  							         Now().GetSeconds()<<","<<tag.GetWhere()<<","<<request_rate<<","<<cars_number<<","<<pedestrians_number<<","<<tag.GetUplinkTime()<<","<<tag.GetElaborationTime()<<","<<edge_computation_capacity
		<<","<<car_computation_capacity<<","<<tag.GetQueueingTime()<<","<<tag.GetWorkload()<<","<<tag.GetCarsInVCC()<<","<<vehicle_queue_length<<"\n";
		myfile.close();
	    }
	    else{
	    
	    NS_LOG_INFO("Pedestrian Received a Packet BUT WITHOUT TAG!!!!");
	    }

    }
  }
  
    void PedApplication::SendPacket(Ptr<Packet> packet, Ipv4Address destination, uint16_t port)
  { 
      NS_LOG_FUNCTION (this);
      

			//timestamp is set in the default constructor of the PacketDataTag class as Simulator::Now()
      //NS_LOG_INFO ("Pedestrian sends Message of size " << packet->GetSize()<< " destination IP: " << destination<< " port:" << port);
      m_send_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(destination), port));
      m_send_socket->Send(packet);

      //I create a file needed to calculate the failure rate i.e. satisfied requests over total requests per each paradigm 
      std::ofstream myfile;
      myfile.open (output_data_csv+"/total_sent_packets_"+sim_type+".csv", std::ofstream::app);
      myfile <<GetNode()->GetId()<<","<<Now().GetSeconds()<<"\n";
      myfile.close();

      double reqRateMillis = request_rate/1000;//requestrate in milliseconds double



      //getting sched_time from exponential 
      Ptr<ExponentialRandomVariable> rand_expo = CreateObject<ExponentialRandomVariable> ();
      double scheduling_instant_random_expon_offset = rand_expo->GetValue(reqRateMillis, 1);//milliseconds to x seconds bound
     

      Ptr <Packet> new_packet = Create <Packet> (packet_size);
      PacketDataTag tag;
      tag.SetWorkload(task_workload);
      tag.SetNodeId (GetNode()->GetId());
      tag.SetInitialTime (Seconds(Now().GetSeconds()+scheduling_instant_random_expon_offset));
      new_packet->AddPacketTag (tag);
                        
      Simulator::Schedule (Seconds(scheduling_instant_random_expon_offset), &PedApplication::SendPacket, this, new_packet, gNb_ip, m_port1);

  }

} 
