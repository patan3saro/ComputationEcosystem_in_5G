/**
 * PacketDataTag - A tag for carrying offloading metadata in packets
 *
 * This tag carries important metadata including timestamps, workload information,
 * and addressing data for task offloading between pedestrians, edge servers,
 * vehicles, and cloud servers.
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
#include "ns3/ipv4-address.h"

namespace ns3 {

/**
 * \brief A custom tag for offloading simulation
 *
 * This class provides functionality to carry metadata for task offloading
 * simulation between pedestrians, edge servers, vehicles, and cloud.
 * A tag can be added to any packet, but you cannot add a tag of the same type twice.
 */
class PacketDataTag : public Tag 
{
public:
  /**
   * \brief Default constructor
   */
  PacketDataTag();
  
  /**
   * \brief Constructor with node ID
   * 
   * \param nodeId The node identifier
   */
  PacketDataTag(uint32_t nodeId);
  
  /**
   * \brief Destructor
   */
  virtual ~PacketDataTag();
  
  // Functions inherited from ns3::Tag that must be implemented
  
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
  
  // Accessor functions
  
  /**
   * \brief Get the source pedestrian's IPv4 address
   * \return IPv4 address of the pedestrian
   */
  Ipv4Address GetSourcePedestrianIPv4Address();
  
  /**
   * \brief Get the node ID
   * \return Node ID that generated this packet
   */
  uint32_t GetNodeId();
  
  /**
   * \brief Get the initial timestamp
   * \return Time when packet was created
   */
  Time GetInitialTime();
  
  /**
   * \brief Get the final timestamp
   * \return Time when packet processing was completed
   */
  Time GetFinalTime();
  
  /**
   * \brief Get the offloading destination
   * \return Location code (0=Edge, 1=Cloud, 2=VCC)
   */
  uint32_t GetWhere();
  
  /**
   * \brief Get uplink transmission time
   * \return Time when packet arrived at processing node
   */
  Time GetUplinkTime();
  
  /**
   * \brief Get elaboration (processing) time
   * \return Time taken for processing the task
   */
  Time GetElaborationTime();
  
  /**
   * \brief Get time spent in queue
   * \return Time packet waited in queue
   */
  Time GetQueueingTime();
  
  /**
   * \brief Get task workload
   * \return Workload in millions of instructions
   */
  double GetWorkload();
  
  /**
   * \brief Get number of cars in VCC
   * \return Count of vehicles in vehicular cloud
   */
  uint32_t GetCarsInVCC();
  
  /**
   * \brief Check if packet is from VCC to pedestrian
   * \return 1 if from VCC to pedestrian, 0 otherwise
   */
  uint32_t GetIsFromVCCToPed();
  
  // Mutator functions
  
  /**
   * \brief Set the source pedestrian's IPv4 address
   * \param pedestrianIp IPv4 address of the pedestrian
   */
  void SetSourcePedestrianIPv4Address(Ipv4Address pedestrianIp);
  
  /**
   * \brief Set the node ID
   * \param nodeId Node identifier
   */
  void SetNodeId(uint32_t nodeId);
  
  /**
   * \brief Set the initial timestamp
   * \param initialTime Time when packet was created
   */
  void SetInitialTime(Time initialTime);
  
  /**
   * \brief Set the final timestamp
   * \param finalTime Time when packet processing was completed
   */
  void SetFinalTime(Time finalTime);
  
  /**
   * \brief Set the offloading destination
   * \param where Location code (0=Edge, 1=Cloud, 2=VCC)
   */
  void SetWhere(uint32_t where);
  
  /**
   * \brief Set uplink transmission time
   * \param uplinkTime Time when packet arrived at processing node
   */
  void SetUplinkTime(Time uplinkTime);
  
  /**
   * \brief Set elaboration (processing) time
   * \param elaborationTime Time taken for processing the task
   */
  void SetElaborationTime(Time elaborationTime);
  
  /**
   * \brief Set time spent in queue
   * \param queueingTime Time packet waited in queue
   */
  void SetQueueingTime(Time queueingTime);
  
  /**
   * \brief Set task workload
   * \param workload Workload in millions of instructions
   */
  void SetWorkload(double workload);
  
  /**
   * \brief Set number of cars in VCC
   * \param carsInVCC Count of vehicles in vehicular cloud
   */
  void SetCarsInVCC(uint32_t carsInVCC);
  
  /**
   * \brief Set if packet is from VCC to pedestrian
   * \param isFromVCCToPed 1 if from VCC to pedestrian, 0 otherwise
   */
  void SetIsFromVCCToPed(uint32_t isFromVCCToPed);
  
private:
  Ipv4Address m_pedestrian_ip;     ///< IP address of the pedestrian who sent the packet
  uint32_t m_userNodeId;           ///< User who generated the packet
  Time m_timestamp_initial;        ///< Time of packet generation
  Time m_timestamp_final;          ///< Time of completion
  uint32_t m_where;                ///< Where the packet is offloaded (0=Edge, 1=Cloud, 2=VCC)
  Time m_uplink_time;              ///< Arrival time to the processing node
  Time m_elaboration_time;         ///< Time needed for task processing
  Time m_queueing_time;            ///< Time spent in queue
  double m_workload;               ///< Task workload in millions of instructions
  uint32_t m_cars_in_VCC;          ///< Number of cars in VCC at this moment
  uint32_t m_is_from_VCC_to_ped;   ///< Flag indicating if packet is from VCC to pedestrian
};

} // namespace ns3

#endif // PACKET_DATA_TAG_H
