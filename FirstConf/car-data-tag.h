/**
 * CarDataTag - A specialized packet tag for vehicular communications
 *
 * This tag combines position, velocity, and acceleration in one tag.
 * It provides a mechanism to attach vehicle data to packets for
 * inter-vehicle communication and mobility tracking.
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

namespace ns3 {

/**
 * \brief A custom tag for attaching vehicle data to ns-3 packets
 *
 * This class provides functionality to serialize and deserialize vehicle information
 * like position, timestamp, node ID, and queue status for transmission between nodes.
 * A tag can be added to any packet, but you cannot add a tag of the same type twice.
 */
class CarDataTag : public Tag {
public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);
    
    /**
     * \brief Get the instance's TypeId
     * \return the instance TypeId
     */
    virtual TypeId GetInstanceTypeId(void) const;
    
    /**
     * \brief Get the size of tag when serialized
     * \return serialized size in bytes
     */
    virtual uint32_t GetSerializedSize(void) const;
    
    /**
     * \brief Serialize the tag into the provided buffer
     * \param i Tag buffer to write to
     */
    virtual void Serialize(TagBuffer i) const;
    
    /**
     * \brief Deserialize the tag from the provided buffer
     * \param i Tag buffer to read from
     */
    virtual void Deserialize(TagBuffer i);
    
    /**
     * \brief Print the tag contents
     * \param os output stream to print to
     */
    virtual void Print(std::ostream &os) const;
    
    // Constructors/Destructor
    CarDataTag();
    CarDataTag(uint32_t nodeId);
    virtual ~CarDataTag();
    
    // Accessor methods
    Vector GetPosition(void);
    uint32_t GetNodeId();
    Time GetTimestamp();
    std::string GetDestinationMacAddress();
    uint32_t GetQueueState();
    uint32_t GetQueueMaxPackets();
    
    // Mutator methods
    void SetPosition(Vector pos);
    void SetNodeId(uint32_t nodeId);
    void SetTimestamp(Time t);
    void SetDestinationMacAddress(std::string destinationMacAddr);
    void SetQueueState(uint32_t queueState);
    void SetQueueMaxPackets(uint32_t queueMaxPackets);
    
private:
    std::string destinationMacAddress;   ///< Destination MAC address
    uint32_t m_nodeId;                   ///< Node identifier
    Vector m_currentPosition;            ///< Current position vector
    Time m_timestamp;                    ///< Timestamp when tag was created
    uint32_t m_queue_state;              ///< Current queue state
    uint32_t m_queue_max_packets;        ///< Maximum queue capacity
};

} // namespace ns3

#endif // CUSTOM_DATA_TAG_H
