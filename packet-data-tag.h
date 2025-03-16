/*
 * This tag combines position, velocity, and acceleration in one tag.
 */

#ifndef PACKET_DATA_TAG_H
#define PACKET_DATA_TAG_H

#include "ns3/tag.h"
#include "ns3/vector.h"
#include "ns3/nstime.h"
#include "ns3/core-module.h"
#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/mobility-module.h"



namespace ns3
{
	/** We're creating a custom tag for simulation. A tag can be added to any packet, but you cannot add a tag of the same type twice.
	*/
class PacketDataTag : public Tag {
public:

	//Functions inherited from ns3::Tag that you have to implement. 
	static TypeId GetTypeId(void);
	virtual TypeId GetInstanceTypeId(void) const;
	virtual uint32_t GetSerializedSize(void) const;
	virtual void Serialize (TagBuffer i) const;
	virtual void Deserialize (TagBuffer i);
	virtual void Print (std::ostream & os) const;

	//These are custom accessor & mutator functions
	Ipv4Address GetSourcePedestrianIPv4Address();
	uint32_t GetNodeId();
	Time GetInitialTime();
	Time GetFinalTime();
	uint32_t GetWhere();
	Time GetUplinkTime();
	Time GetElaborationTime();
	Time GetQueueingTime();
	double GetWorkload();
	uint32_t GetCarsInVCC();
	uint32_t GetIsFromVCCToPed();
	
	void SetSourcePedestrianIPv4Address (Ipv4Address m_pedestrian_ip);
	void SetNodeId (uint32_t node_id);
	void SetInitialTime (Time initial_time);
	void SetFinalTime (Time final_time);
	void SetWhere (uint32_t where);
	void SetUplinkTime(Time m_uplink_time);
	void SetElaborationTime(Time m_elaboration_time);
	void SetQueueingTime(Time m_queueing_time);
	void SetWorkload(double m_workload);
	void SetCarsInVCC(uint32_t m_cars_in_VCC);
	void SetIsFromVCCToPed(uint32_t m_is_from_VCC_to_ped);

	PacketDataTag();
	PacketDataTag(uint32_t node_id);
	virtual ~PacketDataTag();
private:
	
	//Ip of the pedestrian who sends the packet
	Ipv4Address m_pedestrian_ip; 
	/** User who generated the packet */
	uint32_t m_userNodeId;
	/** Time of the generation of the packet */
	Time m_timestamp_initial;
	/** Timestamp of the final time */
	Time m_timestamp_final;
	/** Where the packet is offloaded*/
	uint32_t m_where;
	/** Arrival time of the packet to the paradigm */
	Time m_uplink_time;
	/** All the time needed for elaboration */
	Time m_elaboration_time;
	/*The time that the packet spends in the queue*/
	Time m_queueing_time;
	//The workload os the task in the packet in Million instructions
	double m_workload;
	//amount of cars in VCC in this moment 
	uint32_t m_cars_in_VCC;

	uint32_t m_is_from_VCC_to_ped;
};
}

#endif 
