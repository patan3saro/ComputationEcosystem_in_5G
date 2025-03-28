/**
 * Implementation of the CarApplication class.
 * 
 * This class provides vehicle-to-vehicle (V2V) and vehicle-to-infrastructure (V2I)
 * communication capabilities. It handles packet queuing, signal quality measurement, 
 * and periodic beaconing to broadcast vehicle status.
 */

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/core-module.h"
#include "ns3/nr-module.h"

#include "car-application.h"
#include "car-data-tag.h"
#include "packet-data-tag.h"
#include "globals.h"

// Terminal color codes for logging
#define RED_CODE "\033[91m"
#define GREEN_CODE "\033[32m"
#define END_CODE "\033[0m"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("CarApplication");
NS_OBJECT_ENSURE_REGISTERED(CarApplication);

TypeId 
CarApplication::GetTypeId()
{
    static TypeId tid = TypeId("CarApplication")
        .AddConstructor<CarApplication>()
        .SetParent<Application>()                
        .AddAttribute("Interval", "Broadcast Interval",
                      TimeValue(MilliSeconds(100)),
                      MakeTimeAccessor(&CarApplication::m_broadcast_time),
                      MakeTimeChecker());
    return tid;
}

TypeId 
CarApplication::GetInstanceTypeId() const
{
    return CarApplication::GetTypeId();
}

CarApplication::CarApplication()
{
    m_broadcast_time = MilliSeconds(100);  // Broadcast interval: every 100ms
    m_packetSize = 1000;                   // Default packet size: 1000 bytes
    m_time_limit = Seconds(0.5);           // Time limit for neighbor list
    m_port0 = 9999;                        // Port for communication with pedestrians
    m_port1 = 7777;                        // Port for receiving packets
    m_port_beacon = 8888;                  // Port for beacon broadcasts
    m_sync = true;                         // Enable synchronous beaconing by default
    
    // Set maximum queue size based on global configuration
    m_car_queue->SetMaxSize(QueueSize(std::to_string(vehicle_queue_length) + "p"));
}

CarApplication::~CarApplication()
{
    // No resources to clean up
}

void 
CarApplication::DlDataSinr(uint16_t cellId, uint16_t rnti, double avgSinr, uint16_t bwpId, uint8_t streamId)
{
    // Log signal quality data to CSV file for analysis
    std::ofstream logFile;
    std::string filename = output_data_csv + "/total_data_SINR_received_from_edge_packets_" + sim_type + ".csv";
    logFile.open(filename, std::ofstream::app);
    
    if (logFile.is_open()) {
        logFile << m_nodeId << ","
                << cellId << ","
                << rnti << ","
                << 10 * log10(avgSinr) << ","
                << bwpId << ","
                << "," // Extra comma in original code
                << streamId << ","
                << Now().GetSeconds() << "\n";
        logFile.close();
    }
}

void 
CarApplication::SetupReceiveSocket(Ptr<Socket> socket, uint16_t port)
{
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), port);
    if (socket->Bind(local) == -1) {
        NS_FATAL_ERROR("Failed to bind socket");
    }
}

void
CarApplication::StartApplication()
{
    NS_LOG_FUNCTION(this);
    
    // Create and set up the receiving socket
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    m_recv_socket1 = Socket::CreateSocket(GetNode(), tid);
    m_nodeId = GetNode()->GetId();
    
    SetupReceiveSocket(m_recv_socket1, m_port1);
    m_recv_socket1->SetRecvCallback(MakeCallback(&CarApplication::ReceivePacket, this));
    
    // Set up SINR monitoring
    Ptr<NrUeNetDevice> ueNetDevice = DynamicCast<NrUeNetDevice>(GetNode()->GetDevice(0)); 
    Ptr<NrUePhy> phy = ueNetDevice->GetPhy(0);
    phy->TraceConnectWithoutContext("DlDataSinr", MakeCallback(&CarApplication::DlDataSinr, this));
    
    // Create sending socket
    m_send_socket = Socket::CreateSocket(GetNode(), tid);
    
    // Schedule first beacon with randomized offset to avoid collisions
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    Time randomOffset = MicroSeconds(rand->GetValue(50, 200));
    Simulator::Schedule(m_broadcast_time + randomOffset, &CarApplication::BeaconInformation, this, m_sync);
}

void 
CarApplication::HandleDequeue()
{
    // Peek at the front packet in the queue without removing it
    Ptr<const Packet> queuedPacket = m_car_queue->Peek();
    
    // Create a new packet for response
    Ptr<Packet> responsePacket = Create<Packet>(4000);    
    
    if (queuedPacket) {
        double elaborationTime = 0;
        Ipv4Address destination;
        
        // Extract metadata from the queued packet
        PacketDataTag tag;
        if (queuedPacket->PeekPacketTag(tag)) {   
            // Calculate processing time based on workload and computation capacity
            elaborationTime = tag.GetWorkload() / car_computation_capacity;
            destination = tag.GetSourcePedestrianIPv4Address();
            
            // Update tag with processing information
            tag.SetElaborationTime(Seconds(elaborationTime));
            tag.SetQueueingTime(Seconds(Now().GetSeconds()));
            tag.SetIsFromVCCToPed(1); // 1 indicates packet from vehicle to pedestrian
            
            // Attach tag to response packet
            responsePacket->AddPacketTag(tag);
            
            NS_LOG_INFO("Car schedule send() message to pedestrian");
            
            // Schedule packet sending after processing
            Simulator::Schedule(Seconds(elaborationTime), &CarApplication::SendPacket, 
                               this, responsePacket, destination, m_port0);
            
            // Schedule dequeuing after processing
            Simulator::Schedule(Seconds(elaborationTime), 
                               &ns3::DropTailQueue<Packet>::Dequeue, m_car_queue);
            
            // Schedule async beacon to signal resources are free
            Simulator::Schedule(Seconds(elaborationTime), 
                               &CarApplication::BeaconInformation, this, false);
            
            // Continue processing queue after this packet is done
            Simulator::Schedule(Seconds(elaborationTime), 
                               &CarApplication::HandleDequeue, this);
        } else {
            NS_LOG_INFO("Problem with tag in a car in function HandleDequeue()");
        }
    } else {
        NS_LOG_INFO("No packet in a car in function HandleDequeue()");
    } 
}

void
CarApplication::ReceivePacket(Ptr<Socket> socket)
{  
    Ptr<Packet> packet;
    Address from;
    
    while ((packet = socket->RecvFrom(from))) {
        NS_LOG_INFO("Car ReceivePacket() : Node " << GetNode()->GetId() 
                   << " : Received a packet of Size:" << packet->GetSize());
        
        // Check packet integrity
        Ptr<ErrorModel> rem = CreateObject<RateErrorModel>();
        if (rem->IsCorrupt(packet) || !packet) {
            NS_LOG_INFO("Packet is corrupt in a car");
            continue;
        } 
        
        // Create a new packet to avoid pointer issues
        Ptr<Packet> packetCopy = Create<Packet>(packet->GetSize());
        packetCopy->RemoveAllPacketTags();
        
        // Process packet metadata
        PacketDataTag tag;
        if (packet->PeekPacketTag(tag)) {   
            // Update tag with current time
            tag.SetUplinkTime(Seconds(Now().GetSeconds()));
            packetCopy->AddPacketTag(tag);
            
            // Log packet reception to CSV file for analysis
            std::ofstream logFile;
            std::string filename = output_data_csv + "/total_VCC_received_from_edge_packets_" + sim_type + ".csv";
            logFile.open(filename, std::ofstream::app);
            
            if (logFile.is_open() && packetCopy->PeekPacketTag(tag)) {
                logFile << tag.GetNodeId() << ","
                       << packetCopy->GetSize() << ","
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
        } else {
            NS_LOG_INFO("Problem with tag in a car in function ReceivePacket()");
            continue;
        } 
        
        // Handle packet based on queue state
        if (m_car_queue->GetNPackets() == 0) {
            // Queue is empty - enqueue packet and process immediately
            Simulator::ScheduleNow(&ns3::DropTailQueue<Packet>::Enqueue, m_car_queue, packetCopy);
            Simulator::ScheduleNow(&CarApplication::HandleDequeue, this);
        } else if (m_car_queue->GetNPackets() > 0 && 
                  m_car_queue->GetNPackets() < m_car_queue->GetMaxSize().GetValue()) {
            // Queue has room - enqueue packet and update status
            Simulator::ScheduleNow(&ns3::DropTailQueue<Packet>::Enqueue, m_car_queue, packetCopy);
            Simulator::Schedule(MilliSeconds(5), &CarApplication::BeaconInformation, this, false);
        } else {
            // Queue is full - log dropped packet
            NS_LOG_INFO("A packet is sent with the full queue and it is deleted");
            
            std::ofstream logFile;
            std::string filename = output_data_csv + "/total_VCC_rejected_packets_cause_full_queue_" + sim_type + ".csv";
            logFile.open(filename, std::ofstream::app);
            
            if (logFile.is_open() && packetCopy->PeekPacketTag(tag)) {
                logFile << tag.GetNodeId() << ","
                       << packetCopy->GetSize() << ","
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
            } else {
                NS_LOG_INFO("ERROR: in function ReceivePacket() in tag");
            }
        }
    }
}

void 
CarApplication::SendPacket(Ptr<Packet> packet, Ipv4Address destination, uint16_t port)
{
    NS_LOG_FUNCTION(this << packet << destination << port);
    m_send_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(destination), port));
    m_send_socket->Send(packet);
}

void
CarApplication::BeaconInformation(bool sync) 
{
    NS_LOG_FUNCTION(this);
    
    // Only send beacon if vehicle has available resources
    if (m_car_queue->GetNPackets() < m_car_queue->GetMaxSize().GetValue()) {
        // Create beacon packet
        Ptr<Packet> packet = Create<Packet>(m_packetSize);
        
        // Populate car data tag with vehicle status
        CarDataTag tag;
        tag.SetNodeId(GetNode()->GetId());
        tag.SetPosition(GetNode()->GetObject<MobilityModel>()->GetPosition());
        tag.SetQueueState(m_car_queue->GetNPackets());
        tag.SetQueueMaxPackets(m_car_queue->GetMaxSize().GetValue());
        // Note: timestamp is set in the CarDataTag constructor
        
        // Attach tag to packet
        packet->AddPacketTag(tag);
        
        // Log beacon information
        std::ofstream logFile;
        std::string filename = output_data_csv + "/total_VCC_beacons_" + sim_type + ".csv";
        logFile.open(filename, std::ofstream::app);
        
        if (logFile.is_open()) {
            logFile << tag.GetNodeId() << ","
                   << packet->GetSize() << ","
                   << tag.GetPosition() << ","
                   << Now().GetSeconds() << ","
                   << tag.GetQueueState() << ","
                   << request_rate << ","
                   << cars_number << ","
                   << pedestrians_number << ","
                   << edge_computation_capacity << ","
                   << car_computation_capacity << ","
                   << tag.GetQueueMaxPackets() << ","
                   << vehicle_queue_length << "\n";
            logFile.close();
        }
        
        // Send beacon packet
        Simulator::ScheduleNow(&CarApplication::SendPacket, this, packet, gNb_ip, m_port_beacon);
    }
    
    // Schedule next periodic beacon if in synchronous mode
    if (sync) {
        Simulator::Schedule(m_broadcast_time, &CarApplication::BeaconInformation, this, sync);
    }
}

} // namespace ns3
