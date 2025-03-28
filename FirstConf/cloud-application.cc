/**
 * CloudApplication - Implementation of cloud server application for task offloading
 *
 * This class implements a cloud computing application that receives packets from
 * pedestrian nodes via edge servers, processes them based on computation capacity,
 * and returns results directly to the pedestrian nodes.
 */

#include "ns3/log.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/csma-net-device.h"
#include "ns3/ethernet-header.h"
#include "ns3/arp-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"

#include "cloud-application.h"
#include "globals.h"
#include "packet-data-tag.h"

// Terminal color codes for logging
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

namespace ns3 {

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
  : m_port1(7777),
    m_port_sending(9999)
{
  // Nothing else to initialize
}

CloudApplication::~CloudApplication()
{
  // Nothing to clean up
}

void 
CloudApplication::SetupReceiveSocket(Ptr<Socket> socket, uint16_t port)
{
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), port);
  if (socket->Bind(local) == -1)
  {
    NS_FATAL_ERROR("Failed to bind socket");
  }
}

void 
CloudApplication::StartApplication()
{
  // Create and set up receiving socket
  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  m_recv_socket1 = Socket::CreateSocket(GetNode(), tid);
  SetupReceiveSocket(m_recv_socket1, m_port1);
  m_recv_socket1->SetRecvCallback(MakeCallback(&CloudApplication::HandleReadOne, this));
  
  // Create sending socket
  m_send_socket = Socket::CreateSocket(GetNode(), tid);
}

void 
CloudApplication::HandleReadOne(Ptr<Socket> socket)
{
  NS_LOG_FUNCTION(this << socket);
  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  
  while ((packet = socket->RecvFrom(from)))
  {
    NS_LOG_INFO(TEAL_CODE << "HandleReadOne: Cloud received a Packet of size: " 
                << packet->GetSize() << " at time " 
                << Now().GetSeconds() << END_CODE);
    NS_LOG_INFO(packet->ToString());
    
    // Extract destination address from packet tag
    Ipv4Address pedestrianAddress;
    
    // Process packet metadata
    PacketDataTag tag;
    if (packet->RemovePacketTag(tag))
    {
      // Update tag with processing information
      tag.SetUplinkTime(Seconds(Now().GetSeconds()));
      tag.SetElaborationTime(Seconds(tag.GetWorkload() / cloud_computation_capacity));
      tag.SetQueueingTime(Seconds(Now().GetSeconds()));
      
      // Get pedestrian address for return packet
      pedestrianAddress = tag.GetSourcePedestrianIPv4Address();
      
      // Reattach the updated tag
      packet->AddPacketTag(tag);
    }
    
    // Create a copy of the packet for sending
    Ptr<Packet> packetCopy = packet->Copy();
    
    // Schedule packet transmission after processing delay
    double processingDelay = packet->GetSize() / cloud_computation_capacity;
    Simulator::Schedule(Seconds(processingDelay), 
                       &CloudApplication::SendPacket, 
                       this, 
                       packetCopy, 
                       pedestrianAddress, 
                       m_port_sending);
  }
}

void 
CloudApplication::SendPacket(Ptr<Packet> packet, Ipv4Address destination, uint16_t port)
{
  NS_LOG_FUNCTION(this << packet << destination << port);
  m_send_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(destination), port));
  m_send_socket->Send(packet);
}

} // namespace ns3
