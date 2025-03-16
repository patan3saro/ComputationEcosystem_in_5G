#include "ns3/application.h"
#include <vector>
#include "ns3/udp-socket.h"
#include "packet-data-tag.h"
#include "ns3/drop-tail-queue.h"


namespace ns3
{
    /** \brief A struct to represent information about this node's neighbors. I chose MAC address and the time last message was received form that node
     * The time 'last_beacon' is used to determine whether we should remove the neighbor from the list.
     */

    class CarApplication : public ns3::Application
    {
        public: 

		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;

		CarApplication();
		~CarApplication();

		Ptr<DropTailQueue<Packet>> m_car_queue = CreateObject<DropTailQueue<Packet>> ();


		double scheduling_time = 0;
		//node id 
		uint32_t m_nodeId;
		//traces infos ctrl sinr 
		//void DlCtrlSinr(uint16_t m_cellId, uint16_t m_rnti, double m_avgSinr, uint16_t m_bwpId, uint8_t streamId);
		//traces infos data sinr
		void DlDataSinr(uint16_t m_cellId, uint16_t m_rnti, double m_avgSinr, uint16_t m_bwpId, uint8_t streamId);

		/** \brief Broadcast some information 
		*/ 
		void BeaconInformation(bool sync);

		void HandleDequeue();
		/** \brief This function is called when a net device receives a packet. 
		* I connect to the callback in StartApplication. This matches the signiture of NetDevice receive.
		*/
		void ReceivePacket (Ptr<Socket> socket);


		/** \brief Send an outgoing packet. This creates a new socket every time (not the best solution)
		*/
		void SendPacket (Ptr<Packet> packet, Ipv4Address destination, uint16_t port);

		void SetBroadcastInterval (Time interval);
		/** \brief Change the data rate used for broadcasts.
		*/

            //You can create more functions like getters, setters, and others

        private:
		void SetupReceiveSocket (Ptr<Socket> socket, uint16_t port);
		virtual void StartApplication(); 

		uint16_t m_port0;

  		Ptr<Socket> m_recv_socket1; /**< A socket to receive on a specific port */
		uint16_t m_port1; 
		uint16_t m_port_beacon;

		Ptr<Socket> m_send_socket; /**< A socket to listen on a specific port */

		Time m_broadcast_time; /**< How often do you broadcast messages */ 
		uint32_t m_packetSize; /**< Packet size in bytes */
		Time m_time_limit; /**< Time limit to keep neighbors in a list */
		bool m_sync;
             
            
            

    };
}
