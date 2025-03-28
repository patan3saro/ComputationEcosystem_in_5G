/**
 * CarDataTag implementation - A tag class for attaching vehicle data to ns-3 packets
 * 
 * This class provides functionality to serialize and deserialize vehicle information
 * like position, timestamp, node ID, and queue status for transmission between nodes.
 */

#include "car-data-tag.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("CarDataTag");
NS_OBJECT_ENSURE_REGISTERED(CarDataTag);

CarDataTag::CarDataTag() 
{
    m_timestamp = Simulator::Now();
    m_nodeId = -1;
    m_queue_state = 0;
    m_queue_max_packets = 0;
}

CarDataTag::CarDataTag(uint32_t nodeId) 
{
    m_timestamp = Simulator::Now();
    m_nodeId = nodeId;
    m_queue_state = 0;
    m_queue_max_packets = 0;
}

CarDataTag::~CarDataTag() 
{
    // Nothing to clean up
}

TypeId 
CarDataTag::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::CarDataTag")
        .SetParent<Tag>()
        .AddConstructor<CarDataTag>();
    return tid;
}

TypeId 
CarDataTag::GetInstanceTypeId(void) const
{
    return CarDataTag::GetTypeId();
}

/**
 * Calculate serialized size for the tag
 * 
 * Size includes:
 *   - Vector for position (3 doubles)
 *   - Time for timestamp (1 double)
 *   - Node ID, queue state, and max queue size (3 uint32_t)
 */
uint32_t 
CarDataTag::GetSerializedSize(void) const
{
    return sizeof(Vector) + sizeof(ns3::Time) + 3 * sizeof(uint32_t);
}

/**
 * Serialize tag data to buffer
 * 
 * The order of serialization must match the order in Deserialize()
 */
void 
CarDataTag::Serialize(TagBuffer i) const
{
    // Store timestamp
    i.WriteDouble(m_timestamp.GetDouble());
    
    // Store position vector
    i.WriteDouble(m_currentPosition.x);
    i.WriteDouble(m_currentPosition.y);
    i.WriteDouble(m_currentPosition.z);
    
    // Store node ID and queue information
    i.WriteU32(m_nodeId);
    i.WriteU32(m_queue_state);
    i.WriteU32(m_queue_max_packets);
}

/**
 * Deserialize tag data from buffer
 * 
 * Reads data in the same order it was written by Serialize()
 */
void 
CarDataTag::Deserialize(TagBuffer i)
{
    // Extract timestamp
    m_timestamp = Time::FromDouble(i.ReadDouble(), Time::NS);
    
    // Extract position
    m_currentPosition.x = i.ReadDouble();
    m_currentPosition.y = i.ReadDouble();
    m_currentPosition.z = i.ReadDouble();
    
    // Extract node ID and queue information
    m_nodeId = i.ReadU32();
    m_queue_state = i.ReadU32();
    m_queue_max_packets = i.ReadU32();
}

/**
 * Print tag information for ASCII traces
 */
void 
CarDataTag::Print(std::ostream &os) const
{
    os << "Car Data --- Node: " << m_nodeId 
       << "\t(" << m_timestamp << ") "
       << "Pos (" << m_currentPosition << ")";
}

// Accessor methods
uint32_t 
CarDataTag::GetNodeId() 
{
    return m_nodeId;
}

Vector 
CarDataTag::GetPosition(void) 
{
    return m_currentPosition;
}

Time 
CarDataTag::GetTimestamp() 
{
    return m_timestamp;
}

std::string 
CarDataTag::GetDestinationMacAddress()
{
    return destinationMacAddress;
}

uint32_t 
CarDataTag::GetQueueState()
{
    return m_queue_state;
}

uint32_t 
CarDataTag::GetQueueMaxPackets()
{
    return m_queue_max_packets;
}

// Mutator methods
void 
CarDataTag::SetNodeId(uint32_t nodeId) 
{
    m_nodeId = nodeId;
}

void 
CarDataTag::SetPosition(Vector pos) 
{
    m_currentPosition = pos;
}

void 
CarDataTag::SetTimestamp(Time t) 
{
    m_timestamp = t;
}

void 
CarDataTag::SetDestinationMacAddress(std::string destinationMacAddr)
{
    destinationMacAddress = destinationMacAddr;
}

void 
CarDataTag::SetQueueState(uint32_t queueState)
{
    m_queue_state = queueState;
}

void 
CarDataTag::SetQueueMaxPackets(uint32_t queueMaxPackets)
{
    m_queue_max_packets = queueMaxPackets;
}

} // namespace ns3
