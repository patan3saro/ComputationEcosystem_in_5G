/**
 * Implementation of PacketDataTag class
 *
 * This class provides a tag type that can be attached to packets to carry
 * metadata like timestamps, workload, and addressing information between
 * simulation nodes during offloading operations.
 */

#include "packet-data-tag.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("PacketDataTag");
NS_OBJECT_ENSURE_REGISTERED(PacketDataTag);

PacketDataTag::PacketDataTag() 
  : m_timestamp_initial(Simulator::Now()),
    m_userNodeId(-1),
    m_is_from_VCC_to_ped(0), // Default: not from VCC to pedestrian
    m_where(0),
    m_cars_in_VCC(0),
    m_workload(0.0)
{
  // Other members are automatically initialized to their default values
}

PacketDataTag::PacketDataTag(uint32_t nodeId) 
  : m_timestamp_initial(Simulator::Now()),
    m_userNodeId(nodeId),
    m_is_from_VCC_to_ped(0), // Default: not from VCC to pedestrian
    m_where(0),
    m_cars_in_VCC(0),
    m_workload(0.0)
{
  // Other members are automatically initialized to their default values
}

PacketDataTag::~PacketDataTag() 
{
  // Nothing to clean up
}

TypeId 
PacketDataTag::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::PacketDataTag")
    .SetParent<Tag>()
    .AddConstructor<PacketDataTag>();
  return tid;
}

TypeId 
PacketDataTag::GetInstanceTypeId(void) const
{
  return PacketDataTag::GetTypeId();
}

uint32_t 
PacketDataTag::GetSerializedSize(void) const
{
  return 5 * sizeof(ns3::Time) + 4 * sizeof(uint32_t) + sizeof(double) + sizeof(Ipv4Address);
}

/**
 * Serialize tag data to buffer
 * 
 * The order of serialization must match the order in Deserialize()
 */
void 
PacketDataTag::Serialize(TagBuffer i) const
{
  // Write pedestrian IP address
  uint8_t buf[4];
  m_pedestrian_ip.Serialize(buf);
  i.Write(buf, 4);
  
  // Write timestamps
  i.WriteDouble(m_timestamp_initial.GetDouble());
  i.WriteDouble(m_timestamp_final.GetDouble());
  i.WriteDouble(m_uplink_time.GetDouble());
  i.WriteDouble(m_elaboration_time.GetDouble());
  i.WriteDouble(m_queueing_time.GetDouble());
  
  // Write location indicator
  i.WriteU32(m_where);
  
  // Write node ID
  i.WriteU32(m_userNodeId);
  
  // Write workload
  i.WriteDouble(m_workload);
  
  // Write number of cars in VCC
  i.WriteU32(m_cars_in_VCC);
  
  // Write VCC-to-pedestrian flag
  i.WriteU32(m_is_from_VCC_to_ped);
}

/**
 * Deserialize tag data from buffer
 * 
 * Reads data in the same order it was written by Serialize()
 */
void 
PacketDataTag::Deserialize(TagBuffer i)
{
  // Read pedestrian IP address
  uint8_t buf[4];
  i.Read(buf, 4);
  m_pedestrian_ip = Ipv4Address::Deserialize(buf);
  
  // Read timestamps
  m_timestamp_initial = Time::FromDouble(i.ReadDouble(), Time::NS);
  m_timestamp_final = Time::FromDouble(i.ReadDouble(), Time::NS);
  m_uplink_time = Time::FromDouble(i.ReadDouble(), Time::NS);
  m_elaboration_time = Time::FromDouble(i.ReadDouble(), Time::NS);
  m_queueing_time = Time::FromDouble(i.ReadDouble(), Time::NS);
  
  // Read location indicator
  m_where = i.ReadU32();
  
  // Read node ID
  m_userNodeId = i.ReadU32();
  
  // Read workload
  m_workload = i.ReadDouble();
  
  // Read number of cars in VCC
  m_cars_in_VCC = i.ReadU32();
  
  // Read VCC-to-pedestrian flag
  m_is_from_VCC_to_ped = i.ReadU32();
}

/**
 * Print tag information (for ASCII traces)
 */
void 
PacketDataTag::Print(std::ostream &os) const
{
  os << "Packet Data --- Node: " << m_userNodeId 
     << "\t(Initial: " << m_timestamp_initial << ")" 
     << "\t(Final: " << m_timestamp_final << ")" 
     << " Where: (" << m_where << ")";
}

//------------------------------------------------------------------------------
// Accessor methods
//------------------------------------------------------------------------------

Ipv4Address 
PacketDataTag::GetSourcePedestrianIPv4Address()
{
  return m_pedestrian_ip;
}

uint32_t 
PacketDataTag::GetNodeId()
{
  return m_userNodeId;
}

Time 
PacketDataTag::GetInitialTime()
{
  return m_timestamp_initial;
}

Time 
PacketDataTag::GetFinalTime()
{
  return m_timestamp_final;
}

uint32_t 
PacketDataTag::GetWhere()
{
  return m_where;
}

Time 
PacketDataTag::GetUplinkTime()
{
  return m_uplink_time;
}

Time 
PacketDataTag::GetElaborationTime()
{
  return m_elaboration_time;
}

Time 
PacketDataTag::GetQueueingTime()
{
  return m_queueing_time;
}

double 
PacketDataTag::GetWorkload()
{
  return m_workload;
}

uint32_t 
PacketDataTag::GetCarsInVCC()
{
  return m_cars_in_VCC;
}

uint32_t 
PacketDataTag::GetIsFromVCCToPed()
{
  return m_is_from_VCC_to_ped;
}

//------------------------------------------------------------------------------
// Mutator methods
//------------------------------------------------------------------------------

void 
PacketDataTag::SetSourcePedestrianIPv4Address(Ipv4Address pedestrianIp)
{
  m_pedestrian_ip = pedestrianIp;
}

void 
PacketDataTag::SetNodeId(uint32_t nodeId)
{
  m_userNodeId = nodeId;
}

void 
PacketDataTag::SetInitialTime(Time t)
{
  m_timestamp_initial = t;
}

void 
PacketDataTag::SetFinalTime(Time t)
{
  m_timestamp_final = t;
}

void 
PacketDataTag::SetWhere(uint32_t where)
{
  m_where = where;
}

void 
PacketDataTag::SetUplinkTime(Time t)
{
  m_uplink_time = t;
}

void 
PacketDataTag::SetElaborationTime(Time t)
{
  m_elaboration_time = t;
}

void 
PacketDataTag::SetQueueingTime(Time t)
{
  m_queueing_time = t;
}

void 
PacketDataTag::SetWorkload(double workload)
{
  m_workload = workload;
}

void 
PacketDataTag::SetCarsInVCC(uint32_t carsInVCC)
{
  m_cars_in_VCC = carsInVCC;
}

void 
PacketDataTag::SetIsFromVCCToPed(uint32_t isFromVCCToPed)
{
  m_is_from_VCC_to_ped = isFromVCCToPed;
}

} // namespace ns3
