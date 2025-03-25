#include "ns3/log.h"
#include "cloud-application.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/csma-net-device.h"
#include "ns3/ethernet-header.h"
#include "ns3/arp-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "globals.h"
#include "packet-data-tag.h"

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
  NS_LOG_COMPONENT_DEFINE("CloudApplication");
  NS_OBJECT_ENSURE_REGISTERED(CloudApplication);

  TypeId
  CloudApplication::GetTypeId()
  {
    static TypeId tid = TypeId("ns3::CloudApplication")
                            .AddConstructor<CloudApplication>()
                            .SetParent<Application>();
    return tid;
  }

  TypeId
  CloudApplication::GetInstanceTypeId() const
  {
    return CloudApplication::GetTypeId();
  }

  CloudApplication::CloudApplication()
  {
    m_port1 = 7777;
    m_port_sending = 9999;
  }
  CloudApplication::~CloudApplication()
  {
  }
  void CloudApplication::SetupReceiveSocket(Ptr<Socket> socket, uint16_t port)
  {
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), port);
    if (socket->Bind(local) == -1)
    {
      NS_FATAL_ERROR("Failed to bind socket");
    }
  }
  void CloudApplication::StartApplication()
  {
    //Receive sockets
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    m_recv_socket1 = Socket::CreateSocket(GetNode(), tid);

    SetupReceiveSocket(m_recv_socket1, m_port1);

    m_recv_socket1->SetRecvCallback(MakeCallback(&CloudApplication::HandleReadOne, this));

    //Send Socket
    m_send_socket = Socket::CreateSocket(GetNode(), tid);
  }

  void CloudApplication::HandleReadOne(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    
    while ((packet = socket->RecvFrom(from)))
    {
      NS_LOG_INFO(TEAL_CODE << "HandleReadOne : Cloud received a Packet of size: " << packet->GetSize() << " at time " << Now().GetSeconds() << END_CODE);
      NS_LOG_INFO(packet->ToString());
      
      Ipv4Address pedestrianAddress;
      
      	//setting packet infos in the tag
	    PacketDataTag tag;
          
	    if (packet->RemovePacketTag (tag))
	    {
	    	tag.SetUplinkTime(Seconds(Now().GetSeconds()));
	    	tag.SetElaborationTime(Seconds(tag.GetWorkload()/cloud_computation_capacity));
	    	tag.SetQueueingTime(Seconds(Now().GetSeconds()));
	    	
	    	pedestrianAddress = tag.GetSourcePedestrianIPv4Address();
	    	
		packet->AddPacketTag (tag);
	    }
   	
		

        //create new packet
	Ptr <Packet> packet_tmp = packet->Copy();
   
      Simulator::Schedule (Seconds (packet->GetSize()/cloud_computation_capacity), &CloudApplication::SendPacket, this, packet_tmp, pedestrianAddress, m_port_sending);
    }
    
  }

  void CloudApplication::SendPacket(Ptr<Packet> packet, Ipv4Address destination, uint16_t port)
  {
    NS_LOG_FUNCTION (this << packet << destination << port);
    m_send_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(destination), port));
    m_send_socket->Send(packet);
  }

} // namespace ns3
