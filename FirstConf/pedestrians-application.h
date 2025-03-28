/**
 * PedApplication - Application for pedestrian nodes in task offloading simulation
 *
 * This class implements an application that models pedestrian behavior,
 * generating offloading requests with exponentially distributed inter-arrival
 * times and processing responses from edge/cloud/VCC nodes.
 */

#ifndef NS3_UDP_ARQ_PED_APPLICATION_H
#define NS3_UDP_ARQ_PED_APPLICATION_H

#include "ns3/socket.h"
#include "ns3/application.h"
#include "ns3/ipv4-address.h"

namespace ns3 {

/**
 * \brief Application that models a pedestrian generating offloading requests
 *
 * This class implements the pedestrian behavior in the offloading scenario,
 * generating task requests and processing responses from computing nodes.
 */
class PedApplication : public Application 
{
public:
  /**
   * \brief Default constructor
   */
  PedApplication();
  
  /**
   * \brief Destructor
   */
  virtual ~PedApplication();
  
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
   * \brief Handle incoming response packets
   *
   * This function processes packets received from computing nodes
   * (Edge, Cloud, VCC) after task processing.
   *
   * \param socket The socket that received the packet
   */
  void HandleReadOne(Ptr<Socket> socket);
  
  /**
   * \brief Send an offloading request packet
   *
   * Creates and sends a task offloading request to the destination node.
   * After sending, schedules the next request according to the configured rate.
   *
   * \param packet The packet to send
   * \param destination The destination IPv4 address
   * \param port The destination port number
   */
  void SendPacket(Ptr<Packet> packet, Ipv4Address destination, uint16_t port);
  
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
   * Called at the start of the simulation to set up sockets and schedule
   * the first packet transmission.
   */
  virtual void StartApplication();
  
  Ptr<Socket> m_recv_socket0;  ///< Socket for receiving processed task responses (port 9999)
  Ptr<Socket> m_recv_socket1;  ///< Alternative receiving socket (unused)
  Ptr<Socket> m_send_socket;   ///< Socket for sending task requests
  uint16_t m_port0;            ///< Port for receiving responses (9999)
  uint16_t m_port1;            ///< Port for sending requests (7777)
};

} // namespace ns3

#endif // NS3_UDP_ARQ_PED_APPLICATION_H
