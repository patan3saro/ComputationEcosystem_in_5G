/*
 * This tag combines position, velocity, and acceleration in one tag.
 */

#ifndef CUSTOM_DATA_TAG_H
#define CUSTOM_DATA_TAG_H

#include "ns3/tag.h"
#include "ns3/vector.h"
#include "ns3/nstime.h"
#include "ns3/core-module.h"
#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/wave-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"



namespace ns3
{
	/** We're creating a custom tag for simulation. A tag can be added to any packet, but you cannot add a tag of the same type twice.
	*/
class CarDataTag : public Tag {
public:

	//Functions inherited from ns3::Tag that you have to implement. 
	static TypeId GetTypeId(void);
	virtual TypeId GetInstanceTypeId(void) const;
	virtual uint32_t GetSerializedSize(void) const;
	virtual void Serialize (TagBuffer i) const;
	virtual void Deserialize (TagBuffer i);
	virtual void Print (std::ostream & os) const;

	//These are custom accessor & mutator functions
	Vector GetPosition(void);
	uint32_t GetNodeId();
	Time GetTimestamp ();
	std::string GetDestinationMacAddress();
	uint32_t GetQueueState();
	uint32_t GetQueueMaxPackets();

	void SetPosition (Vector pos);
	void SetNodeId (uint32_t node_id);
	void SetTimestamp (Time t);
	void SetDestinationMacAddress(std::string destinationMacAddr);
	void SetQueueState(uint32_t m_queue_state);
	void SetQueueMaxPackets(uint32_t m_queue_max_packets);



	CarDataTag();
	CarDataTag(uint32_t node_id);
	virtual ~CarDataTag();
private:
	std::string destinationMacAddress; 
	uint32_t m_nodeId;
	/** Current position */
	Vector m_currentPosition;
	/** Timestamp this tag was created */
	Time m_timestamp;
	uint32_t m_queue_state;
	uint32_t m_queue_max_packets;
};
}

#endif 
