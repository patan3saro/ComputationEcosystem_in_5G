/**
 * EdgeApplication - Implementation of edge server application for task offloading
 *
 * This class implements an edge computing application that receives packets from
 * pedestrian nodes, processes them based on available resources, and either
 * processes them locally, offloads them to available vehicles (VCC), or sends
 * them to the cloud based on the selected offloading strategy.
 */

#include "ns3/log.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/ethernet-header.h"
#include "ns3/arp-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/mobility-module.h"
#include "ns3/nstime.h"

#include "edge-application.h"
#include "car-data-tag.h"
#include "packet-data-tag.h"
#include "globals.h"

#include <vector>
#include <time.h>

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
  : m_port1(7777),
    m_port2(8888)
{
  // Nothing else to initialize
}

EdgeApplication::~EdgeApplication()
{
  // Nothing to clean up
}

void 
EdgeApplication::SetupReceiveSocket(Ptr<Socket> socket, uint16_t port)
{
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), port);
  if (socket->Bind(local) == -1)
  {
    NS_FATAL_ERROR("Failed to bind socket");
  }
}

void 
EdgeApplication::StartApplication()
{
  // Create and set up receiving sockets
  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  m_recv_socket1 = Socket::CreateSocket(GetNode(), tid);
  m_recv_socket2 = Socket::CreateSocket(GetNode(), tid);
  
  SetupReceiveSocket(m_recv_socket1, m_port1);
  SetupReceiveSocket(m_recv_socket2, m_port2);
  
  // Set receive callbacks
  m_recv_socket1->SetRecvCallback(MakeCallback(&EdgeApplication::HandleReadOne, this));
  m_recv_socket2->SetRecvCallback(MakeCallback(&EdgeApplication::HandleNeighbor, this));

  // Set up IP-level packet reception tracing
  Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();
  ipv4->TraceConnectWithoutContext("Rx", MakeCallback(&EdgeApplication::HandleResponseFromVehicle, this));

  // Create sending socket
  m_send_socket = Socket::CreateSocket(GetNode(), tid);
  
  NS_LOG_FUNCTION(this);
  
  // Schedule periodic check to remove old neighbors (every 0.1 seconds)
  Simulator::Schedule(Seconds(0.1), &EdgeApplication::RemoveOldNeighbors, this);
}

void 
EdgeApplication::HandleReadOne(Ptr<Socket> socket)
{
  NS_LOG_FUNCTION(this << socket);
  Ptr<Packet> packet;
  Address from;
  Address localAddress;

  while ((packet = socket->RecvFrom(from)))
  {
    NS_LOG_INFO(TEAL_CODE << "HandleReadOne: Edge Received a Packet of size: " 
                << packet->GetSize() << " at time " << Now().GetSeconds() << END_CODE);
    NS_LOG_INFO(packet->ToString());
    
    // Get source address of the pedestrian
    InetSocketAddress fromAddr = InetSocketAddress::ConvertFrom(from);
    Ipv4Address destIp = fromAddr.GetIpv4();

    // Update packet tag with source address and VCC information
    PacketDataTag tag;
    if (packet->RemovePacketTag(tag))
    {
      tag.SetSourcePedestrianIPv4Address(destIp);
      tag.SetCarsInVCC(m_neighbors.size());
      packet->AddPacketTag(tag);
    }

    // Apply the selected offloading strategy
    if (chosen_strategy == "Random") {
      this->RandomStrategy(packet, destIp);
    }
    else if (chosen_strategy == "VCCFirst") {
      bool vccActive = true;
      bool edgeActive = true;
      bool cloudActive = true;
      this->FirstVCCStrategy(packet, destIp, vccActive, edgeActive, cloudActive);
    }
    else if (chosen_strategy == "EC_and_CC") {
      bool vccActive = false;
      bool edgeActive = true;
      bool cloudActive = true;
      this->FirstVCCStrategy(packet, destIp, vccActive, edgeActive, cloudActive);
    }
    else if (chosen_strategy == "VCC_and_CC") {
      bool vccActive = true;
      bool edgeActive = false;
      bool cloudActive = true;
      this->FirstVCCStrategy(packet, destIp, vccActive, edgeActive, cloudActive);
    }
  }
}

void 
EdgeApplication::HandleResponseFromVehicle(Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t interface)
{
  // Process packet responses from vehicles passing through the edge server
  PacketDataTag tag;
  if (packet->PeekPacketTag(tag))
  {
    if (tag.GetIsFromVCCToPed() == 1) {
      // Log response data to CSV file
      std::ofstream logFile;
      std::string filename = output_data_csv + "/response_offloading_passing_in_gNB_" + sim_type + ".csv";
      logFile.open(filename, std::ofstream::app);
      
      if (logFile.is_open()) {
        logFile << tag.GetNodeId() << ","
                << packet->GetSize() << ","
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
        logFile.close();
      }
    }
  }
  else {
    NS_LOG_INFO("EDGE in HandleResponseFromVehicle Received a Packet BUT WITHOUT TAG!!!!");
  }
}

void 
EdgeApplication::HandleNeighbor(Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;
  
  while ((packet = socket->RecvFrom(from)))
  {
    InetSocketAddress fromAddr = InetSocketAddress::ConvertFrom(from);
    Ipv4Address source = fromAddr.GetIpv4();
    
    // Process vehicle beacon information
    CarDataTag tag;
    if (packet->PeekPacketTag(tag))
    {
      uint32_t queueState = tag.GetQueueState();

      if (queueState < tag.GetQueueMaxPackets() && queueState >= 0) {
        // Update neighbor list if vehicle has capacity
        UpdateNeighbor(source, tag.GetNodeId(), tag.GetPosition());
      }
      else if (queueState >= tag.GetQueueMaxPackets()) {
        // Remove from neighbor list if vehicle has no available resources
        for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++)
        {
          if (it->neighbor_ip == source)
          {
            NS_LOG_INFO(RED_CODE << Now() << " Node " << GetNode()->GetId() 
                        << " resourceless neighbor " << it->neighbor_ip << END_CODE);

            // Log vehicle departure due to resource exhaustion
            std::ofstream logFile;
            std::string filename = output_data_csv + "/dwell_time_" + sim_type + ".csv";
            logFile.open(filename, std::ofstream::app);
            
            if (logFile.is_open()) {
              logFile << tag.GetNodeId() << ","
                      << cars_number << ","
                      << it->neighbor_ip << ","
                      << Now() << ","
                      << "OUT" << ","
                      << "resourceless" << ","
                      << tag.GetPosition() << "\n";
              logFile.close();
            }

            m_neighbors.erase(it);
            break;
          }
        }
      }
    }
  }
}

void 
EdgeApplication::UpdateNeighbor(Ipv4Address addr, uint32_t nodeId, Vector position)
{
  bool found = false;
  
  // Check if neighbor already exists in the list
  for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++)
  {
    if (it->neighbor_ip == addr)
    {
      // Update last beacon time
      it->last_beacon = Now();
      found = true;
      break;
    }
  }
  
  if (!found) {
    // Add new neighbor to the list
    NS_LOG_INFO(GREEN_CODE << Now() << " : Node " << GetNode()->GetId() 
                << " is adding a neighbor with IP=" << addr << END_CODE);
    
    NeighborInformation newNeighbor;
    newNeighbor.node_ID = nodeId;
    newNeighbor.neighbor_ip = addr;
    newNeighbor.last_beacon = Now();
    m_neighbors.push_back(newNeighbor);

    // Log vehicle arrival
    std::ofstream logFile;
    std::string filename = output_data_csv + "/dwell_time_" + sim_type + ".csv";
    logFile.open(filename, std::ofstream::app);
    
    if (logFile.is_open()) {
      logFile << nodeId << ","
              << cars_number << ","
              << addr << ","
              << Now() << ","
              << "IN" << ","
              << "range" << ","
              << position << "\n";
      logFile.close();
    }
  }
}

void 
EdgeApplication::RemoveNeighbor(Ipv4Address addr)
{
  // Find and remove neighbor with specified address
  for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++)
  {
    if (it->neighbor_ip == addr)
    {
      NS_LOG_INFO(RED_CODE << Now() << " Node " << GetNode()->GetId()
                  << " is removing old neighbor because it is used " << it->neighbor_ip << END_CODE);

      // Log vehicle departure due to being used for computation
      std::ofstream logFile;
      std::string filename = output_data_csv + "/dwell_time_" + sim_type + ".csv";
      logFile.open(filename, std::ofstream::app);
      
      if (logFile.is_open()) {
        logFile << it->node_ID << ","
                << cars_number << ","
                << it->neighbor_ip << ","
                << Now() << ","
                << "OUT" << ","
                << "used" << "\n";
        logFile.close();
      }

      m_neighbors.erase(it);
      break;
    }
  }
}

void 
EdgeApplication::PrintNeighbors()
{
  std::cout << "Neighbor Info for Node: " << GetNode()->GetId() << std::endl;
  for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++)
  {
    std::cout << "\tMAC: " << it->neighbor_ip 
              << "\tLast Contact: " << it->last_beacon << std::endl;
  }
}

void 
EdgeApplication::RemoveOldNeighbors()
{
  // Remove neighbors that haven't been heard from for 0.5 seconds
  for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++)
  {
    Time lastContact = Now() - it->last_beacon;

    if (lastContact >= Seconds(0.5))
    {
      NS_LOG_INFO(RED_CODE << Now() << " Node " << GetNode()->GetId()
                  << " is removing old neighbor " << it->neighbor_ip << END_CODE);

      // Log vehicle departure due to being out of range
      std::ofstream logFile;
      std::string filename = output_data_csv + "/dwell_time_" + sim_type + ".csv";
      logFile.open(filename, std::ofstream::app);
      
      if (logFile.is_open()) {
        logFile << it->node_ID << ","
                << cars_number << ","
                << it->neighbor_ip << ","
                << Now() << ","
                << "OUT" << ","
                << "range" << "\n";
        logFile.close();
      }

      m_neighbors.erase(it);
      break;
    }
  }
  
  // Schedule next check after 0.1 seconds
  Simulator::Schedule(Seconds(0.1), &EdgeApplication::RemoveOldNeighbors, this);
}

void 
EdgeApplication::HandleDequeue()
{
  // Process next packet in edge server queue
  Ptr<const Packet> queuedPacket = m_edge_queue->Peek();
  Ptr<Packet> responsePacket = Create<Packet>(4000);

  if (queuedPacket) {
    Ipv4Address destination;
    double elaborationTime = 0;

    PacketDataTag tag;
    if (queuedPacket->PeekPacketTag(tag))
    {
      elaborationTime = tag.GetWorkload() / edge_computation_capacity;
      tag.SetElaborationTime(Seconds(elaborationTime));
      tag.SetQueueingTime(Seconds(Now().GetSeconds()));
      
      destination = tag.GetSourcePedestrianIPv4Address();
      responsePacket->AddPacketTag(tag);
    }
    else {
      NS_LOG_INFO("ERROR with TAG in Edge HandleDequeue()");
    }

    NS_LOG_INFO(YELLOW_CODE << "Packet elaboration time: " << Seconds(elaborationTime) << END_CODE);
    
    uint16_t pedestrianPort = 9999;
    
    // Schedule sending, dequeuing, and recursive call after computation time
    Simulator::Schedule(Seconds(elaborationTime), &EdgeApplication::SendPacket, 
                       this, responsePacket, destination, pedestrianPort);
    Simulator::Schedule(Seconds(elaborationTime), &ns3::DropTailQueue<Packet>::Dequeue, m_edge_queue);
    Simulator::Schedule(Seconds(elaborationTime), &EdgeApplication::HandleDequeue, this);
  }
  else {
    NS_LOG_INFO("NO PACKET in Edge queue HandleDequeue()");
  }
}

void 
EdgeApplication::SendPacket(Ptr<Packet> packet, Ipv4Address destination, uint16_t port)
{
  NS_LOG_FUNCTION(this << packet << destination << port);
  m_send_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(destination), port));
  m_send_socket->Send(packet);
  NS_LOG_INFO(PURPLE_CODE << "Edge sending Packet of size " << packet->GetSize() 
              << " at time " << Now().GetSeconds() << END_CODE);
  NS_LOG_INFO("Content: " << packet->ToString());
}

void 
EdgeApplication::RandomStrategy(Ptr<Packet> packet, Ipv4Address destIp)
{
  // Determine valid destination range based on available resources
  uint16_t maxNum = 2;  // 0=Edge, 1=Cloud, 2=VCC
  uint16_t minNum = 0;
  
  if (m_edge_queue->GetNPackets() == 100) {
    // Edge queue full, can only use VCC or Cloud
    minNum = 1;
  }
  else if (m_neighbors.size() <= 0) {
    // No vehicles available, can only use Edge or Cloud
    maxNum = 1;
  }
  else if (m_edge_queue->GetNPackets() == 100 && m_neighbors.size() <= 0) {
    // Only Cloud is available
    minNum = 1;
    maxNum = minNum;
  }
  
  // Random selection within valid range
  Ptr<UniformRandomVariable> randomVar = CreateObject<UniformRandomVariable>();
  int choice = randomVar->GetInteger(minNum, maxNum);
  
  // Update packet tag with destination choice
  PacketDataTag tag;
  if (packet->RemovePacketTag(tag)) {
    tag.SetWhere(choice);
    packet->AddPacketTag(tag);
  }
  
  switch (choice) {
    case 0: {  // Edge offloading
      NS_LOG_INFO(YELLOW_CODE << "Edge Offloading " << Now().GetSeconds() << END_CODE);
      NS_LOG_INFO(YELLOW_CODE << "Edge packets in queue " << m_edge_queue->GetNPackets() << END_CODE);
      
      // Update packet tag and handle queue operations
      if (packet->RemovePacketTag(tag)) {
        tag.SetUplinkTime(Seconds(Now().GetSeconds()));
        packet->AddPacketTag(tag);
      }
      
      if (m_edge_queue->GetNPackets() == 0) {
        Simulator::ScheduleNow(&ns3::DropTailQueue<Packet>::Enqueue, m_edge_queue, packet);
        Simulator::ScheduleNow(&EdgeApplication::HandleDequeue, this);
      }
      else if (m_edge_queue->GetNPackets() > 0) {
        Simulator::ScheduleNow(&ns3::DropTailQueue<Packet>::Enqueue, m_edge_queue, packet);
      }
      
      // Log edge offloading decision
      std::ofstream logFile;
      std::string filename = output_data_csv + "/total_EDGE_sent_packets_" + sim_type + ".csv";
      logFile.open(filename, std::ofstream::app);
      
      if (logFile.is_open()) {
        logFile << tag.GetNodeId() << ","
                << packet->GetSize() << ","
                << tag.GetInitialTime() << ","
                << Now().GetSeconds() << ","
                << tag.GetWhere() << ","
                << request_rate << ","
                << cars_number << ","
                << pedestrians_number << ","
                << edge_computation_capacity << ","
                << car_computation_capacity << ","
                << tag.GetWorkload() << ","
                << tag.GetCarsInVCC() << ","
                << vehicle_queue_length << "\n";
        logFile.close();
      }
      
      break;
    }
    
    case 1: {  // Cloud offloading
      NS_LOG_INFO(YELLOW_CODE << "From Edge to Cloud at time " << Now().GetSeconds() << END_CODE);
      
      // Send packet to cloud
      Simulator::ScheduleNow(&EdgeApplication::SendPacket, this, packet, cloud_ip, m_port1);
      
      // Log cloud offloading decision
      std::ofstream logFile;
      std::string filename = output_data_csv + "/total_CLOUD_sent_packets_" + sim_type + ".csv";
      logFile.open(filename, std::ofstream::app);
      
      if (logFile.is_open() && packet->PeekPacketTag(tag)) {
        logFile << tag.GetNodeId() << ","
                << packet->GetSize() << ","
                << tag.GetInitialTime() << ","
                << Now().GetSeconds() << ","
                << tag.GetWhere() << ","
                << request_rate << ","
                << cars_number << ","
                << pedestrians_number << ","
                << edge_computation_capacity << ","
                << car_computation_capacity << ","
                << tag.GetWorkload() << ","
                << tag.GetCarsInVCC() << ","
                << vehicle_queue_length << "\n";
        logFile.close();
      }
      
      break;
    }
    
    case 2: {  // VCC (Vehicle Cloud Computing) offloading
      NS_LOG_INFO(YELLOW_CODE << "From Edge to VC at time " << Now().GetSeconds() << END_CODE);
      
      // Select a random vehicle from neighbors
      Ptr<UniformRandomVariable> randomVar = CreateObject<UniformRandomVariable>();
      int idx = randomVar->GetInteger(0, m_neighbors.size() - 1);
      Ipv4Address vehicleAddress = m_neighbors[idx].neighbor_ip;
      
      // Send packet to selected vehicle and remove it from available list
      Simulator::ScheduleNow(&EdgeApplication::SendPacket, this, packet, vehicleAddress, m_port1);
      Simulator::ScheduleNow(&EdgeApplication::RemoveNeighbor, this, vehicleAddress);
      
      // Log VCC offloading decision
      std::ofstream logFile;
      std::string filename = output_data_csv + "/total_VCC_sent_packets_" + sim_type + ".csv";
      logFile.open(filename, std::ofstream::app);
      
      if (logFile.is_open() && packet->PeekPacketTag(tag)) {
        logFile << tag.GetNodeId() << ","
                << packet->GetSize() << ","
                << tag.GetInitialTime() << ","
                << Now().GetSeconds() << ","
                << tag.GetWhere() << ","
                << request_rate << ","
                << cars_number << ","
                << pedestrians_number << ","
                << edge_computation_capacity << ","
                << car_computation_capacity << ","
                << tag.GetWorkload() << ","
                << tag.GetCarsInVCC() << ","
                << vehicle_queue_length << "\n";
        logFile.close();
      }
      
      break;
    }
  }
}

void 
EdgeApplication::FirstVCCStrategy(Ptr<Packet> packet, Ipv4Address destIp, 
                                  bool vccActive, bool edgeActive, bool cloudActive)
{
  // VCC offloading (first priority if active and vehicles available)
  if (m_neighbors.size() > 0 && vccActive) {
    NS_LOG_INFO(YELLOW_CODE << "From Edge to VC at time " << Now().GetSeconds() << END_CODE);
    
    // Select a random vehicle from neighbors
    Ptr<UniformRandomVariable> randomVar = CreateObject<UniformRandomVariable>();
    int idx = randomVar->GetInteger(0, m_neighbors.size() - 1);
    Ipv4Address vehicleAddress = m_neighbors[idx].neighbor_ip;
    
    // Update packet tag
    PacketDataTag tag;
    if (packet->RemovePacketTag(tag)) {
      tag.SetWhere(2);  // 2 = VCC
      packet->AddPacketTag(tag);
    }
    
    // Send packet to selected vehicle and remove it from available list
    Simulator::ScheduleNow(&EdgeApplication::SendPacket, this, packet, vehicleAddress, m_port1);
    Simulator::ScheduleNow(&EdgeApplication::RemoveNeighbor, this, vehicleAddress);

    // Log VCC offloading decision
    std::ofstream logFile;
    std::string filename = output_data_csv + "/total_VCC_sent_packets_" + sim_type + ".csv";
    logFile.open(filename, std::ofstream::app);
    
    if (logFile.is_open() && packet->PeekPacketTag(tag)) {
      logFile << tag.GetNodeId() << ","
              << packet->GetSize() << ","
              << tag.GetInitialTime() << ","
              << Now().GetSeconds() << ","
              << tag.GetWhere() << ","
              << request_rate << ","
              << cars_number << ","
              << pedestrians_number << ","
              << edge_computation_capacity << ","
              << car_computation_capacity << ","
              << tag.GetWorkload() << ","
              << tag.GetCarsInVCC() << ","
              << vehicle_queue_length << "\n";
      logFile.close();
    }
  }
  // Edge offloading (second priority if active and queue not full)
  else if (m_edge_queue->GetNPackets() < m_edge_queue->GetMaxSize().GetValue() && edgeActive) {
    NS_LOG_INFO(YELLOW_CODE << "Edge packets in queue " << m_edge_queue->GetNPackets() << END_CODE);
    NS_LOG_INFO(YELLOW_CODE << "Edge Offloading " << Now().GetSeconds() << END_CODE);
    
    // Update packet tag
    PacketDataTag tag;
    if (packet->RemovePacketTag(tag)) {
      tag.SetUplinkTime(Seconds(Now().GetSeconds()));
      tag.SetWhere(0);  // 0 = Edge
      packet->AddPacketTag(tag);
    }
    
    // Handle queue operations
    if (m_edge_queue->GetNPackets() == 0) {
      Simulator::ScheduleNow(&ns3::DropTailQueue<Packet>::Enqueue, m_edge_queue, packet);
      Simulator::ScheduleNow(&EdgeApplication::HandleDequeue, this);
    }
    else if (m_edge_queue->GetNPackets() > 0) {
      Simulator::ScheduleNow(&ns3::DropTailQueue<Packet>::Enqueue, m_edge_queue, packet);
    }
    
    // Log edge offloading decision
    std::ofstream logFile;
    std::string filename = output_data_csv + "/total_EDGE_sent_packets_" + sim_type + ".csv";
    logFile.open(filename, std::ofstream::app);
    
    if (logFile.is_open()) {
      logFile << tag.GetNodeId() << ","
              << packet->GetSize() << ","
              << tag.GetInitialTime() << ","
              << Now().GetSeconds() << ","
              << tag.GetWhere() << ","
              << request_rate << ","
              << cars_number << ","
              << pedestrians_number << ","
              << edge_computation_capacity << ","
              << car_computation_capacity << ","
              << tag.GetWorkload() << ","
              << tag.GetCarsInVCC() << ","
              << vehicle_queue_length << "\n";
      logFile.close();
    }
  }
  // Cloud offloading (last priority if active)
  else if (cloudActive) {
    NS_LOG_INFO(YELLOW_CODE << "From Edge to Cloud at time " << Now().GetSeconds() << END_CODE);
    
    // Update packet tag
    PacketDataTag tag;
    if (packet->RemovePacketTag(tag)) {
      tag.SetWhere(1);  // 1 = Cloud
      packet->AddPacketTag(tag);
    }
    
    // Send packet to cloud
    Simulator::ScheduleNow(&EdgeApplication::SendPacket, this, packet, cloud_ip, m_port1);
    
    // Log cloud offloading decision
    std::ofstream logFile;
    std::string filename = output_data_csv + "/total_CLOUD_sent_packets_" + sim_type + ".csv";
    logFile.open(filename, std::ofstream::app);
    
    if (logFile.is_open() && packet->PeekPacketTag(tag)) {
      logFile << tag.GetNodeId() << ","
              << packet->GetSize() << ","
              << tag.GetInitialTime() << ","
              << Now().GetSeconds() << ","
              << tag.GetWhere() << ","
              << request_rate << ","
              << cars_number << ","
              << pedestrians_number << ","
              << edge_computation_capacity << ","
              << car_computation_capacity << ","
              << tag.GetWorkload() << ","
              << tag.GetCarsInVCC() << ","
              << vehicle_queue_length << "\n";
      logFile.close();
    }
  }
}

} // namespace ns3
