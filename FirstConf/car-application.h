#ifndef CAR_APPLICATION_H
#define CAR_APPLICATION_H

#include "ns3/application.h"
#include "ns3/udp-socket.h"
#include "ns3/drop-tail-queue.h"
#include "packet-data-tag.h"
#include <vector>

namespace ns3 {

/**
 * \brief Application for vehicle-to-vehicle and vehicle-to-infrastructure communication
 * 
 * Handles packet queuing, scheduling, and neighbor discovery through beacons
 */
class CarApplication : public Application
{
public:
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  
  CarApplication();
  virtual ~CarApplication();
  
  // Public methods
  void BeaconInformation(bool sync);
  void HandleDequeue();
  void ReceivePacket(Ptr<Socket> socket);
  void SendPacket(Ptr<Packet> packet, Ipv4Address destination, uint16_t port);
  void SetBroadcastInterval(Time interval);
  void DlDataSinr(uint16_t cellId, uint16_t rnti, double avgSinr, uint16_t bwpId, uint8_t streamId);
  
  // Public members
  Ptr<DropTailQueue<Packet>> m_car_queue = CreateObject<DropTailQueue<Packet>>();
  double scheduling_time = 0;
  uint32_t m_nodeId;

private:
  // Private methods
  virtual void StartApplication();
  void SetupReceiveSocket(Ptr<Socket> socket, uint16_t port);
  
  // Private members
  uint16_t m_port0;
  uint16_t m_port1;
  uint16_t m_port_beacon;
  Ptr<Socket> m_recv_socket1;  ///< Socket to receive on a specific port
  Ptr<Socket> m_send_socket;   ///< Socket to listen on a specific port
  Time m_broadcast_time;       ///< Broadcast message interval
  uint32_t m_packetSize;       ///< Packet size in bytes
  Time m_time_limit;           ///< Time limit to keep neighbors in list
  bool m_sync;
};

} // namespace ns3

#endif // CAR_APPLICATION_H
