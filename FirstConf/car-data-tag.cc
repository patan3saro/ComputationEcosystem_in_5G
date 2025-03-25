#include "car-data-tag.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("CarDataTag");
NS_OBJECT_ENSURE_REGISTERED (CarDataTag);

CarDataTag::CarDataTag() {
	m_timestamp = Simulator::Now();
	m_nodeId = -1;
	m_queue_state = 0;
}
CarDataTag::CarDataTag(uint32_t node_id) {
	m_timestamp = Simulator::Now();
	m_nodeId = node_id;
	m_queue_state = 0;
}

CarDataTag::~CarDataTag() {
}

//Almost all car tags will have similar implementation of GetTypeId and GetInstanceTypeId
TypeId CarDataTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CarDataTag")
    .SetParent<Tag> ()
    .AddConstructor<CarDataTag> ();
  return tid;
}
TypeId CarDataTag::GetInstanceTypeId (void) const
{
  return CarDataTag::GetTypeId ();
}

/** The size required for the data contained within tag is:
 *   	size needed for a ns3::Vector for position +
 * 		size needed for a ns3::Time for timestamp + 
 * 		size needed for a uint32_t for node id
 */
uint32_t CarDataTag::GetSerializedSize (void) const
{
	return sizeof(Vector) + sizeof (ns3::Time) + 3*sizeof(uint32_t);
}
/**
 * The order of how you do Serialize() should match the order of Deserialize()
 */
void CarDataTag::Serialize (TagBuffer i) const
{
	//we store timestamp first
	i.WriteDouble(m_timestamp.GetDouble());

	//then we store the position
	i.WriteDouble (m_currentPosition.x);
	i.WriteDouble (m_currentPosition.y);
	i.WriteDouble (m_currentPosition.z);

	//Then we store the node ID
	i.WriteU32(m_nodeId);
	i.WriteU32(m_queue_state);
	i.WriteU32(m_queue_max_packets);
}
/** This function reads data from a buffer and store it in class's instance variables.
 */
void CarDataTag::Deserialize (TagBuffer i)
{
	//We extract what we stored first, so we extract the timestamp
	m_timestamp =  Time::FromDouble (i.ReadDouble(), Time::NS);;

	//Then the position
	m_currentPosition.x = i.ReadDouble();
	m_currentPosition.y = i.ReadDouble();
	m_currentPosition.z = i.ReadDouble();
	//Finally, we extract the node id
	m_nodeId = i.ReadU32();
	m_queue_state = i.ReadU32();
	m_queue_max_packets = i.ReadU32();

}
/**
 * This function can be used with ASCII traces if enabled. 
 */
void CarDataTag::Print (std::ostream &os) const
{
  os << "Car Data --- Node :" << m_nodeId <<  "\t(" << m_timestamp  << ")" << " Pos (" << m_currentPosition << ")";
}

//Your accessor and mutator functions 
uint32_t CarDataTag::GetNodeId() {
	return m_nodeId;
}

void CarDataTag::SetNodeId(uint32_t node_id) {
	m_nodeId = node_id;
}
Vector CarDataTag::GetPosition(void) {
	return m_currentPosition;
}

Time CarDataTag::GetTimestamp() {
	return m_timestamp;
}

std::string CarDataTag::GetDestinationMacAddress(){
     return destinationMacAddress;
}

uint32_t CarDataTag::GetQueueState(){
	return m_queue_state;
}

uint32_t CarDataTag::GetQueueMaxPackets(){
	return m_queue_max_packets;
}

void CarDataTag::SetPosition(Vector pos) {
	m_currentPosition = pos;
}

void CarDataTag::SetTimestamp(Time t) {
	m_timestamp = t;
}

void CarDataTag::SetDestinationMacAddress(std::string  destinationMacAddr){
	destinationMacAddress=destinationMacAddr;
}

void CarDataTag::SetQueueState(uint32_t queue_state){
	m_queue_state = queue_state;
}

void CarDataTag::SetQueueMaxPackets(uint32_t queue_max_packets){
	m_queue_max_packets = queue_max_packets;
}

} /* namespace ns3 */


