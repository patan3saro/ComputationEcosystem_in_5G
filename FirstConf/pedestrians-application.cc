/**
 * Implementation of PedApplication class
 *
 * This class implements a pedestrian application that generates task offloading
 * requests and processes responses from edge/cloud/VCC nodes. It periodically
 * generates new requests following an exponential distribution.
 */

#include "ns3/log.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/csma-net-device.h"
#include "ns3/ethernet-header.h"
#include "ns3/arp-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/core-module.h"

#include "pedestrians-application.h"
#include "packet-data-tag.h"
#include "globals.h"

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
  : m_port0(9999),
    m_port1(7777)
{
  // Nothing else to initialize
}

PedApplication::~PedApplication()
{
  // Nothing to clean up
}

void 
PedApplication::SetupReceiveSocket(Ptr<Socket> socket, uint16_t port)
{
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), port);
  if (socket->Bind(local) == -1)
  {
    NS_FATAL_ERROR("Failed to bind socket");
  }
}

void 
PedApplication::StartApplication()
{
  // Create and set up the receiving socket
  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  m_recv_socket0 = Socket::CreateSocket(GetNode(), tid);
  SetupReceiveSocket(m_recv_socket0, m_port0);
  m_recv_socket0->SetRecvCallback(MakeCallback(&PedApplication::HandleReadOne, this));
  
  // Create sending socket
  m_send_socket = Socket::CreateSocket(GetNode(), tid);

  // Add randomness to the first packet time to avoid collision
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  Time randomOffset = MicroSeconds(rand->GetValue(50, 200));

  // Create the first offloading request packet
  Ptr<Packet> initialPacket = Create<Packet>(packet_size);
  
  // Add metadata tag to the packet
  PacketDataTag tag;
  tag.SetWorkload(task_workload);
  tag.SetNodeId(GetNode()->GetId());
  tag.SetInitialTime(randomOffset);
  initialPacket->AddPacketTag(tag);

  // Schedule sending the first packet after random offset
  Simulator::Schedule(randomOffset, &PedApplication::SendPacket, 
                     this, initialPacket, gNb_ip, m_port1);
}

void 
PedApplication::HandleReadOne(Ptr<Socket> socket)
{
  NS_LOG_FUNCTION(this << socket);
  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  
  while ((packet = socket->RecvFrom(from)))
  {
    uint32_t packetSize = packet->GetSize();
    
    NS_LOG_INFO(TEAL_CODE << "Pedestrian HandleReadOne: Received a Packet of size: " 
                << packetSize << " at time " << Now().GetSeconds() << END_CODE);
    NS_LOG_INFO(packet->ToString());
    
    // Log response data to CSV file
    std::ofstream logFile;
    std::string filename = output_data_csv + "/offloading_time_" + sim_type + ".csv";
    logFile.open(filename, std::ofstream::app);
    
    // Extract metadata from packet tag
    PacketDataTag tag;
    if (packet->PeekPacketTag(tag))
    {
      if (logFile.is_open()) {
        logFile << tag.GetNodeId() << ","
               << packetSize << ","
               << tag.GetInitialTime() << ","
               << Now().GetSeconds() << ","
               << tag.GetWhere() << ","
               << request_rate << ","
               << cars_number << ","
               << pedestrians_number << ","
               << tag.GetUplinkTime() << ","
               << tag.GetElaborationTime() << ","
               << edge_computation_capacity << ","
               << car_computation_capacity << ","
               << tag.GetQueueingTime() << ","
               << tag.GetWorkload() << ","
               << tag.GetCarsInVCC() << ","
               << vehicle_queue_length << "\n";
      }
      logFile.close();
    }
    else {
      NS_LOG_INFO("Pedestrian Received a Packet BUT WITHOUT TAG!!!!");
    }
  }
}

void 
PedApplication::SendPacket(Ptr<Packet> packet, Ipv4Address destination, uint16_t port)
{ 
  NS_LOG_FUNCTION(this);
  
  // Connect to destination and send packet
  m_send_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(destination), port));
  m_send_socket->Send(packet);

  // Log request data to CSV file
  std::ofstream logFile;
  std::string filename = output_data_csv + "/total_sent_packets_" + sim_type + ".csv";
  logFile.open(filename, std::ofstream::app);
  
  if (logFile.is_open()) {
    logFile << GetNode()->GetId() << "," << Now().GetSeconds() << "\n";
    logFile.close();
  }

  // Calculate next request time based on exponential distribution
  double reqRateSeconds = request_rate / 1000.0; // Convert milliseconds to seconds
  
  // Generate random offset using exponential distribution
  Ptr<ExponentialRandomVariable> randExpo = CreateObject<ExponentialRandomVariable>();
  double nextRequestTime = randExpo->GetValue(reqRateSeconds, 1);
  
  // Create next request packet
  Ptr<Packet> newPacket = Create<Packet>(packet_size);
  
  // Add metadata tag to new packet
  PacketDataTag tag;
  tag.SetWorkload(task_workload);
  tag.SetNodeId(GetNode()->GetId());
  tag.SetInitialTime(Seconds(Now().GetSeconds() + nextRequestTime));
  newPacket->AddPacketTag(tag);
  
  // Schedule sending the next packet
  Simulator::Schedule(Seconds(nextRequestTime), &PedApplication::SendPacket, 
                     this, newPacket, gNb_ip, m_port1);
}

} // namespace ns3
