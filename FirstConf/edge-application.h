/**
 * EdgeApplication - Header for edge server application for task offloading
 *
 * This class implements an edge computing application that receives packets from
 * pedestrian nodes, processes them based on available resources, and either
 * processes them locally, offloads them to available vehicles (VCC), or sends
 * them to the cloud based on the selected offloading strategy.
 */

#ifndef NS3_UDP_ARQ_EDGE_APPLICATION_H
#define NS3_UDP_ARQ_EDGE_APPLICATION_H

#include "ns3/socket.h"
#include "ns3/application.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ssid.h"
#include "ns3/vector.h"

#include "edge-application.h"

namespace ns3 {

/**
 * \brief Structure to store information about neighbor vehicles
 */
typedef struct {
  uint32_t node_ID;       ///< Node identifier
  Ipv4Address neighbor_ip; ///< IP address of neighbor
  Time last_beacon;       ///< Time of last received beacon
} NeighborInformation;

/**
 * \brief Implementation of an edge computing application for task offloading
 *
 * This class handles packet reception from pedestrians, processing with simulated 
 * computation delay, and implements different offloading strategies (Random, VCCFirst, etc.)
 */
class EdgeApplication : public Application 
{
public:
  /**
   * \brief Default constructor
   */
  EdgeApplication();
  
  /**
   * \brief Destructor
   */
  virtual ~EdgeApplication();
  
  /**
   * \brief Get the TypeId
   *
   * \return The TypeId for this class
   */
  static TypeId GetTypeId();
  
  /**
   * \brief Get the instance TypeId
   *
   * \return The TypeId for this instance
   */
  virtual TypeId GetInstanceTypeId() const;
  
  /**
   * \brief Handle incoming packets on port 7777
   *
   * \param socket The socket that received the packet
   */
  void HandleReadOne(Ptr<Socket> socket);
  
  /**
   * \brief Handle packets received from vehicles passing through the edge node
   *
   * \param packet The packet received
   * \param ipv4 The IPv4 stack that received the packet
   * \param interface The interface index on which the packet was received
   */
  void HandleResponseFromVehicle(Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t interface);
  
  /**
   * \brief Process the next packet in the edge server queue
   */
  void HandleDequeue();
  
  /**
   * \brief Send a packet to the specified destination
   *
   * \param packet The packet to send
   * \param destination The destination IPv4 address
   * \param port The destination port number
   */
  void SendPacket(Ptr<Packet> packet, Ipv4Address destination, uint16_t port);
  
  /**
   * \brief Handle incoming beacon packets from neighbor vehicles
   *
   * \param socket The socket that received the packet
   */
  void HandleNeighbor(Ptr<Socket> socket);
  
  /**
   * \brief Remove a neighbor from the neighbor list
   *
   * \param addr IPv4 address of the neighbor to remove
   */
  void RemoveNeighbor(Ipv4Address addr);
  
  /**
   * \brief Update a neighbor's last contact time, or add a new neighbor
   *
   * \param addr IPv4 address of the neighbor
   * \param nodeId Node ID of the neighbor
   * \param position Current position of the neighbor
   */
  void UpdateNeighbor(Ipv4Address addr, uint32_t nodeId, Vector position);
  
  /**
   * \brief Print a list of neighbors for debugging
   */
  void PrintNeighbors();
  
  /**
   * \brief Remove neighbors that haven't been heard from after some time
   */
  void RemoveOldNeighbors();
  
  /**
   * \brief Random strategy for task offloading decision
   *
   * \param packet The packet to offload
   * \param destIp The source IP of the pedestrian that sent the packet
   */
  void RandomStrategy(Ptr<Packet> packet, Ipv4Address destIp);
  
  /**
   * \brief Prioritized strategy for task offloading decision
   *
   * \param packet The packet to offload
   * \param destIp The source IP of the pedestrian that sent the packet
   * \param vccActive Whether VCC offloading is enabled
   * \param edgeActive Whether edge processing is enabled
   * \param cloudActive Whether cloud offloading is enabled
   */
  void FirstVCCStrategy(Ptr<Packet> packet, Ipv4Address destIp, 
                        bool vccActive, bool edgeActive, bool cloudActive);
  
  // Public members
  Ptr<DropTailQueue<Packet>> m_edge_queue = CreateObject<DropTailQueue<Packet>>();
  double finalElaborationInstant;

private:
  /**
   * \brief Set up a receiving socket on the specified port
   *
   * \param socket The socket to set up
   * \param port The port to bind to
   */
  void SetupReceiveSocket(Ptr<Socket> socket, uint16_t port);
  
  /**
   * \brief Initialize the application
   *
   * Called at the start of the simulation to set up sockets.
   */
  virtual void StartApplication();

  // Private members
  Ptr<Socket> m_recv_socket1;  ///< Socket for receiving data packets
  Ptr<Socket> m_recv_socket2;  ///< Socket for receiving beacon packets
  uint16_t m_port1;            ///< Port for receiving data packets (7777)
  uint16_t m_port2;            ///< Port for receiving beacon packets (8888)
  Ptr<Socket> m_send_socket;   ///< Socket for sending packets
  std::vector<NeighborInformation> m_neighbors;  ///< List of neighbor vehicles
};

} // namespace ns3

#endif // NS3_UDP_ARQ_EDGE_APPLICATION_H
