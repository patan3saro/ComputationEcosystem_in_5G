
#include "packet-data-tag.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("PacketDataTag");
NS_OBJECT_ENSURE_REGISTERED (PacketDataTag);

PacketDataTag::PacketDataTag() {
	m_timestamp_initial = Simulator::Now();
	m_userNodeId = -1;
	//default it is not from VCC to ped
	m_is_from_VCC_to_ped = 0;
}
PacketDataTag::PacketDataTag(uint32_t node_id) {
	m_timestamp_initial = Simulator::Now();
	m_userNodeId = node_id;
	//default it is not from VCC to ped
	m_is_from_VCC_to_ped = 0;
}

PacketDataTag::~PacketDataTag() {
}

//Almost all car tags will have similar implementation of GetTypeId and GetInstanceTypeId
TypeId PacketDataTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PacketDataTag")
    .SetParent<Tag> ()
    .AddConstructor<PacketDataTag> ();
  return tid;
}
TypeId PacketDataTag::GetInstanceTypeId (void) const
{
  return PacketDataTag::GetTypeId ();
}


uint32_t PacketDataTag::GetSerializedSize (void) const
{
	return 5*sizeof (ns3::Time) + 4*sizeof(uint32_t) + sizeof(double) + sizeof(Ipv4Address);
}
/**
 * The order of how you do Serialize() should match the order of Deserialize()
 */
void PacketDataTag::Serialize (TagBuffer i) const
{
	//writing pedestrian address in the buffer
	uint8_t buf[4];
	m_pedestrian_ip.Serialize(buf);
	i.Write(buf, 4);
	
	

	//we store timestamp first
	i.WriteDouble(m_timestamp_initial.GetDouble());
	i.WriteDouble(m_timestamp_final.GetDouble());
	i.WriteDouble(m_uplink_time.GetDouble());
	i.WriteDouble(m_elaboration_time.GetDouble());
	i.WriteDouble(m_queueing_time.GetDouble());

	//then we store the where
	i.WriteU32(m_where);


	//Then we store the node ID
	i.WriteU32(m_userNodeId);
	
	//Then we store the workload
	i.WriteDouble(m_workload);
	
	//then we store the amount of cars in the VCC
	i.WriteU32(m_cars_in_VCC);

	//then we store if it is the VCC who sends the packet to ped
	i.WriteU32(m_is_from_VCC_to_ped);

	
}
/** This function reads data from a buffer and store it in class's instance variables.
 */
void PacketDataTag::Deserialize (TagBuffer i)
{	//read the pedestrian address
	//writing pedestrian address in the buffer
	uint8_t buf[4];
	i.Read(buf,4);
	m_pedestrian_ip = Ipv4Address::Deserialize (buf);



	//We extract what we stored first, so we extract the timestamp
	m_timestamp_initial =  Time::FromDouble (i.ReadDouble(), Time::NS);
	m_timestamp_final =  Time::FromDouble (i.ReadDouble(), Time::NS);
	m_uplink_time =  Time::FromDouble (i.ReadDouble(), Time::NS);
	m_elaboration_time =  Time::FromDouble (i.ReadDouble(), Time::NS);
	m_queueing_time = Time::FromDouble (i.ReadDouble(), Time::NS);

	//Then the where
	m_where = i.ReadU32();
	//Then we extract the node id
	m_userNodeId = i.ReadU32();
	
	//Then we extract the workload
	m_workload = i.ReadDouble();
	
	//then we extract the amount of cars in VCC
	m_cars_in_VCC = i.ReadU32();

	//is set if it is the vcc who sends the packet to peds

	m_is_from_VCC_to_ped = i.ReadU32();

}
/**
 * This function can be used with ASCII traces if enabled. 
 */
void PacketDataTag::Print (std::ostream &os) const
{
  os << "Packet Data --- Node :" << m_userNodeId <<  "\t(" << m_timestamp_initial  << ")" << "\t(" << m_timestamp_final  << ")" <<" Where (" << m_where << ")";
}

//Your accessor and mutator functions 
Ipv4Address PacketDataTag::GetSourcePedestrianIPv4Address(){
	return m_pedestrian_ip;
}

uint32_t PacketDataTag::GetNodeId() {
	return m_userNodeId;
}

Time PacketDataTag::GetInitialTime() {
	return m_timestamp_initial;
}

Time PacketDataTag::GetFinalTime() {
	return m_timestamp_final;
}

uint32_t PacketDataTag::GetWhere() {
	return m_where;
}

Time PacketDataTag::GetUplinkTime() {
	return m_uplink_time;
}

Time PacketDataTag::GetElaborationTime() {
	return m_elaboration_time;
}

Time PacketDataTag::GetQueueingTime() {
	return m_queueing_time;
}

double PacketDataTag::GetWorkload(){
	return m_workload;

}

uint32_t PacketDataTag::GetCarsInVCC(){
	return m_cars_in_VCC;
}

uint32_t PacketDataTag::GetIsFromVCCToPed(){
	return m_is_from_VCC_to_ped;
}

//////

void PacketDataTag::SetSourcePedestrianIPv4Address(Ipv4Address pedestrian_ip) {
	m_pedestrian_ip = pedestrian_ip;
}

void PacketDataTag::SetNodeId(uint32_t node_id) {
	m_userNodeId = node_id;
}


void PacketDataTag::SetInitialTime(Time t) {
	m_timestamp_initial = t;
}

void PacketDataTag::SetFinalTime(Time t) {
	m_timestamp_final = t;
}

void PacketDataTag::SetWhere(uint32_t where) {
	m_where = where;
}

void PacketDataTag::SetUplinkTime(Time t) {
	m_uplink_time = t;
}

void PacketDataTag::SetElaborationTime(Time t) {
	m_elaboration_time = t;
}

void PacketDataTag::SetQueueingTime(Time t){
	m_queueing_time = t;
}

void PacketDataTag::SetWorkload(double workload){
	m_workload = workload;
}

void PacketDataTag::SetCarsInVCC(uint32_t cars_in_VCC){
	m_cars_in_VCC = cars_in_VCC;
}

void PacketDataTag::SetIsFromVCCToPed(uint32_t is_from_VCC_to_ped){
	m_is_from_VCC_to_ped = is_from_VCC_to_ped;
}

} /* namespace ns3 */


