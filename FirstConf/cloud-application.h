/**
 * CloudApplication - A cloud server application for task offloading in ns-3
 *
 * This class implements a cloud computing application that receives packets from
 * edge servers, processes them based on cloud computation capacity, and returns
 * results to the pedestrian nodes.
 */

#ifndef CLOUD_APPLICATION_H
#define CLOUD_APPLICATION_H

#include "ns3/socket.h"
#include "ns3/application.h"
#include "ns3/ipv4-address.h"

namespace ns3 {

/**
 * \brief Implementation of a cloud computing application
 *
 * This class handles packet reception, processing with simulated computation delay,
 * and response delivery for cloud-based task offloading.
 */
class CloudApplication : public Application 
{
public:
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
   * \brief Default constructor
   */
  CloudApplication();
  
  /**
   * \brief Destructor
   */
  virtual ~CloudApplication();
  
  /**
   * \brief Handle incoming packets on port 7777
   *
   * This method processes packets received at the cloud server,
   * simulates computation delay, and schedules response packets.
   *
   * \param socket The socket that received the packet
   */
  void HandleReadOne(Ptr<Socket> socket);
  
  /**
   * \brief Send a packet to the specified destination
   *
   * This method sends a processed packet back to the source pedestrian.
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
   * Called at the start of the simulation to set up sockets.
   */
  virtual void StartApplication();
  
  Ptr<Socket> m_recv_socket1;  ///< Socket for receiving packets on port m_port1
  Ptr<Socket> m_recv_socket2;  ///< Socket for receiving packets on a different port (unused)
  uint16_t m_port1;            ///< Port for receiving packets (7777)
  uint16_t m_port_sending;     ///< Port for sending processed packets (9999)
  Ptr<Socket> m_send_socket;   ///< Socket for sending processed packets
};

} // namespace ns3

#endif // CLOUD_APPLICATION_H
