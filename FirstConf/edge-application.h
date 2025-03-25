#ifndef NS3_UDP_ARQ_EDGE_APPLICATION_H
#define NS3_UDP_ARQ_EDGE_APPLICATION_H
#include "ns3/socket.h"
#include "ns3/application.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "edge-application.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ssid.h"
#include "ns3/vector.h"
using namespace ns3;

namespace ns3
{

typedef struct{
  uint32_t node_ID;
  Ipv4Address neighbor_ip;
  Time last_beacon;
}NeighborInformation;


  class EdgeApplication : public Application 
  {
    public:
    
      EdgeApplication ();
      virtual ~EdgeApplication ();

      static TypeId GetTypeId ();
      virtual TypeId GetInstanceTypeId () const;
      
      
      Ptr<DropTailQueue<Packet>> m_edge_queue = CreateObject<DropTailQueue<Packet>> ();
      
      double finalElaborationInstant;
      
      /** \brief handles incoming packets on port
       */
      void HandleReadOne (Ptr<Socket> socket);
      void HandleResponseFromVehicle (Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t interface);
      
      void HandleDequeue ();

      /** \brief Send an outgoing packet. This creates a new socket every time (not the best solution)
      */
      void SendPacket (Ptr<Packet> packet, Ipv4Address destination, uint16_t port);
      
      void HandleNeighbor (Ptr<Socket> socket);
      
      void RemoveNeighbor (Ipv4Address addr);
      
  /** \brief Update a neighbor's last contact time, or add a new neighbor
     */
      void UpdateNeighbor (Ipv4Address addr, uint32_t nodeId, Vector position);
    /** \brief Print a list of neighbors
     */
      void PrintNeighbors ();
                  /** \brief Remove neighbors you haven't heard from after some time.
             */
      void RemoveOldNeighbors ();
      
      // strategies 
      void RandomStrategy(Ptr<Packet> packet, Ipv4Address dest_ip);
      void FirstVCCStrategy(Ptr<Packet> packet, Ipv4Address dest_ip, bool vcc_active, bool edge_active, bool cloud_active);

    private:
      
      void SetupReceiveSocket (Ptr<Socket> socket, uint16_t port);
      virtual void StartApplication ();

      Ptr<Socket> m_recv_socket1; /**< A socket to receive on a specific port */
      Ptr<Socket> m_recv_socket2;
 
      uint16_t m_port1; 
      uint16_t m_port2;
  

      Ptr<Socket> m_send_socket; /**< A socket to listen on a specific port */
      
      std::vector<NeighborInformation> m_neighbors;
  };
}

#endif
