#include "ns3/antenna-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/nr-module.h"
#include "ns3/point-to-point-module.h"
#include "edge-application.h"
#include "cloud-application.h"
#include "ns3/point-to-point-module.h"
#include "pedestrians-application.h"
#include "car-application.h"
#include "globals.h"

#define PURPLE_CODE "\033[95m"
#define CYAN_CODE "\033[96m"
#define TEAL_CODE "\033[36m"
#define BLUE_CODE "\033[94m"
#define GREEN_CODE "\033[32m"
#define YELLOW_CODE "\033[33m"
#define LIGHT_YELLOW_CODE "\033[93m"
#define RED_CODE "\033[91m"
#define BOLD_CODE "\033[1m"
#define END_CODE "\033[0m"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CoE");

int
main(int argc, char* argv[])
{

        // set simulation time and mobility
        double simTime = 120;   // seconds

        // other simulation parameters default values
        uint16_t numerology = 2;

        uint16_t gNbNum = 1;

        double centralFrequency = 6e9;
        double bandwidth = 200e6;
        double txPowergNb = 20;
        double txPowerUe = 23;
        double gNBPosition_x = 1000;
        double gNBPosition_y = 1000;
        double gNBPosition_z = 30;
        double UEPedestrianDistanceFromgNB = 20;
        double UEPosition_z = 1.5;

        uint32_t udpPacketSize = 1000;
        uint8_t fixedMcs = 27;
        bool useFixedMcs = true;
    
        std::string outputDataCsv;
        std::string chosenStrategy;
        std::string traceFile;
        std::string simType;
          
        double workload = 1028; //million instructions takenfrom a paper for light tasks to offload
        uint32_t nVehicles=1;
        uint32_t nPedestrians=2;
        uint32_t vehicleQueueLength=1;
        double requestRate=1000; // milliseconds
        double cloudComputationCapacity=2356230.0;
        double edgeComputationCapacity=749070.0; 
        double carComputationCapacity=221720.0;

        CommandLine cmd(__FILE__); 
    
        cmd.AddValue ("outputDataCsv", "The date folder of the csv output", outputDataCsv);
	cmd.AddValue ("strategy", "The offloading strategy", chosenStrategy);
	cmd.AddValue ("traceFile", "Ns2 trace file", traceFile);
	cmd.AddValue ("simType", "Simulation type", simType);
	cmd.AddValue ("simTime", "Duration of the simulation in", simTime);
	cmd.AddValue ("gNBPosition_x", "x-axis position of gNB", gNBPosition_x);
	cmd.AddValue ("gNBPosition_y", "y-axis position of gNB", gNBPosition_y);
	cmd.AddValue ("gNBPosition_z", "z-axis position of gNB", gNBPosition_z);
	cmd.AddValue ("UEPedestrianDistanceFromgNB", "Distance of pedestrians from gNB", UEPedestrianDistanceFromgNB);
	cmd.AddValue ("UEPosition_z", "z-axis position of all UEs, vehicles included", UEPosition_z);
	cmd.AddValue ("packetSize", "Size of packet", udpPacketSize);
	cmd.AddValue ("workload", "Workload of packet", workload);
	cmd.AddValue ("nVehicles", "Ns2 trace file", nVehicles);
	cmd.AddValue ("nPedestrians", "Size of packet", nPedestrians);
	cmd.AddValue ("carQueueLength", "Length of the queue in a vehicle", vehicleQueueLength);
	cmd.AddValue ("requestRate", "Request rate", requestRate);
	cmd.AddValue ("edgeComputationCapacity", "Computation capacity at the Edge", edgeComputationCapacity);
	cmd.AddValue ("carComputationCapacity", "Computation capacity in cars", carComputationCapacity);
	cmd.AddValue ("cloudComputationCapacity", "Computation capacity in Cloud", cloudComputationCapacity);
	cmd.Parse (argc, argv);
	

	//Number of vehicles 
	uint32_t nCars = nVehicles;
	//Number of pedestrians
	uint32_t nPeds = nPedestrians;
	//setting global variables to use in other parts of the simulator
        packet_size = udpPacketSize;
        task_workload = workload;
        output_data_csv=outputDataCsv;
	chosen_strategy=chosenStrategy;
	request_rate=requestRate;
	cars_number=nVehicles;
	pedestrians_number=nPedestrians;
	vehicle_queue_length=vehicleQueueLength;
	cloud_computation_capacity = cloudComputationCapacity;
	edge_computation_capacity=edgeComputationCapacity;
	car_computation_capacity=carComputationCapacity;
	sim_type=simType;
	
	
        // Create all vehicles nodes and setting mobility from sumo.
	NodeContainer carNodes;
	carNodes.Create (nCars);
	// Create Ns2MobilityHelper with the specified trace log file as parameter for cars
	Ns2MobilityHelper sumoMobility = Ns2MobilityHelper (traceFile); 
	sumoMobility.Install (); // configure movements for each car, while reading trace file

        NS_ASSERT(nPedestrians > 0);

        // setup the nr simulation
        Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();

        /*
        * Spectrum division. We create one operation band with one component carrier
        * (CC) which occupies the whole operation band bandwidth. The CC contains a
        * single Bandwidth Part (BWP). This BWP occupies the whole CC band.
        * Both operational bands will use the StreetCanyon channel modeling.
        */
        CcBwpCreator ccBwpCreator;
        const uint8_t numCcPerBand = 1; // in this example, both bands have a single CC
        //BandwidthPartInfo::Scenario scenario = BandwidthPartInfo::UMi_StreetCanyon;
        BandwidthPartInfo::Scenario scenario = BandwidthPartInfo::UMa_LoS;

        // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
        // a single BWP per CC
        CcBwpCreator::SimpleOperationBandConf bandConf(centralFrequency,
                                                   bandwidth,
                                                   numCcPerBand,
                                                   scenario);

        // By using the configuration created, it is time to make the operation bands
        OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc(bandConf);

        /*
        * Initialize channel and pathloss, plus other things inside band1. If needed,
        * the band configuration can be done manually, but we leave it for more
        * sophisticated examples. For the moment, this method will take care
        * of all the spectrum initialization needs.
        */
        nrHelper->InitializeOperationBand(&band);

        BandwidthPartInfoPtrVector allBwps = CcBwpCreator::GetAllBwps({band});
	Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(320));
        /*
        * Continue setting the parameters which are common to all the nodes, like the
        * gNB transmit power or numerology.
        */
        nrHelper->SetGnbPhyAttribute("TxPower", DoubleValue(txPowergNb));
        nrHelper->SetGnbPhyAttribute("Numerology", UintegerValue(numerology));
        
        //setting tx power for UEs
        nrHelper->SetUePhyAttribute ("TxPower", DoubleValue (txPowerUe));

        // Scheduler
        nrHelper->SetSchedulerTypeId(TypeId::LookupByName("ns3::NrMacSchedulerTdmaRR"));
        nrHelper->SetSchedulerAttribute("FixedMcsDl", BooleanValue(useFixedMcs));
        nrHelper->SetSchedulerAttribute("FixedMcsUl", BooleanValue(useFixedMcs));

        if (useFixedMcs == true)
        {
        nrHelper->SetSchedulerAttribute("StartingMcsDl", UintegerValue(fixedMcs));
        nrHelper->SetSchedulerAttribute("StartingMcsUl", UintegerValue(fixedMcs));
        }

        Config::SetDefault("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));

        // Antennas for all the UEs
        nrHelper->SetUeAntennaAttribute("NumRows", UintegerValue(1));
        nrHelper->SetUeAntennaAttribute("NumColumns", UintegerValue(1));
        nrHelper->SetUeAntennaAttribute("AntennaElement",
                                    PointerValue(CreateObject<IsotropicAntennaModel>()));

        // Antennas for all the gNbs
        nrHelper->SetGnbAntennaAttribute("NumRows", UintegerValue(8));
        nrHelper->SetGnbAntennaAttribute("NumColumns", UintegerValue(8));
        nrHelper->SetGnbAntennaAttribute("AntennaElement",
                                     PointerValue(CreateObject<ThreeGppAntennaModel>()));

        // Beamforming method
        Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
        idealBeamformingHelper->SetAttribute("BeamformingMethod",
                                         TypeIdValue(DirectPathBeamforming::GetTypeId()));
        nrHelper->SetBeamformingHelper(idealBeamformingHelper);

        Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(MilliSeconds(0)));

        nrHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));

        // Error Model: UE and GNB with same spectrum error model.
        nrHelper->SetUlErrorModel("ns3::NrEesmIrT1");
        nrHelper->SetDlErrorModel("ns3::NrEesmIrT1");

        // Both DL and UL AMC will have the same model behind.
        nrHelper->SetGnbDlAmcAttribute(
        "AmcModel",
        EnumValue(NrAmc::ErrorModel)); // NrAmc::ShannonModel or NrAmc::ErrorModel
        nrHelper->SetGnbUlAmcAttribute(
        "AmcModel",
        EnumValue(NrAmc::ErrorModel)); // NrAmc::ShannonModel or NrAmc::ErrorModel

        // Create EPC helper
        Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper>();
        nrHelper->SetEpcHelper(epcHelper);
        // Core latency
        epcHelper->SetAttribute("S1uLinkDelay", TimeValue(MilliSeconds(2)));

        // gNb routing between Bearer and bandwidh part
        uint32_t bwpIdForBearer = 0;
        nrHelper->SetGnbBwpManagerAlgorithmAttribute("GBR_CONV_VOICE", UintegerValue(bwpIdForBearer));

        // Initialize nrHelper
        nrHelper->Initialize();

        /*
        *  Create the gNB and UE nodes ans UE nodes for cars according to the network topology
        */
	
	
        NodeContainer gNbNodes;
        NodeContainer pedestrianNodes;
 
        gNbNodes.Create(gNbNum);
        pedestrianNodes.Create(nPedestrians);
        
        MobilityHelper mobility;
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        Ptr<ListPositionAllocator> bsPositionAlloc = CreateObject<ListPositionAllocator>();
        bsPositionAlloc->Add (Vector (gNBPosition_x, gNBPosition_y, gNBPosition_z));
        mobility.SetPositionAllocator(bsPositionAlloc);
        mobility.Install(gNbNodes);
        
        Ptr<ListPositionAllocator> utPositionAlloc = CreateObject<ListPositionAllocator>();
        //positioning pedestrians
        for (uint32_t i=0;i<nPeds;++i){
        //every node is close & meter to be in the covered area 
        utPositionAlloc->Add (Vector (gNBPosition_x-UEPedestrianDistanceFromgNB, gNBPosition_y+0.01*i, UEPosition_z));
        }   
        mobility.SetPositionAllocator(utPositionAlloc);
        mobility.Install(pedestrianNodes);
        
        // Install nr net devices
        NetDeviceContainer gNbNetDev = nrHelper->InstallGnbDevice(gNbNodes, allBwps);

        NetDeviceContainer pedNetDev = nrHelper->InstallUeDevice(pedestrianNodes, allBwps);
        
        NetDeviceContainer carNetDev = nrHelper->InstallUeDevice(carNodes, allBwps);

        int64_t randomStream = 1;
        randomStream += nrHelper->AssignStreams(gNbNetDev, randomStream);
        randomStream += nrHelper->AssignStreams(pedNetDev, randomStream);
        randomStream += nrHelper->AssignStreams(carNetDev, randomStream);

        // When all the configuration is done, explicitly call UpdateConfig ()

        for (auto it = gNbNetDev.Begin(); it != gNbNetDev.End(); ++it)
        {
        	DynamicCast<NrGnbNetDevice>(*it)->UpdateConfig();
        }

        for (auto it = pedNetDev.Begin(); it != pedNetDev.End(); ++it)
        {
        	DynamicCast<NrUeNetDevice>(*it)->UpdateConfig();
        }
        
        for (auto it = carNetDev.Begin(); it != carNetDev.End(); ++it)
        {
        	DynamicCast<NrUeNetDevice>(*it)->UpdateConfig();
        }

        // create the internet and install the IP stack on the UEs
        // get SGW/PGW and create a single RemoteHost
        Ptr<Node> pgw = epcHelper->GetPgwNode();
        NodeContainer remoteHostContainer;
        remoteHostContainer.Create(1);
        Ptr<Node> remoteHost = remoteHostContainer.Get(0);
        
        
        InternetStackHelper internet;
        internet.Install(remoteHostContainer);

        // connect a remoteHost to pgw. Setup routing too
        PointToPointHelper p2ph;
        p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
        p2ph.SetDeviceAttribute("Mtu", UintegerValue(2500));
        // internet delay based on literature and experiments
        p2ph.SetChannelAttribute("Delay", TimeValue(MilliSeconds(35)));
        NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
        Ipv4AddressHelper ipv4h;
        ipv4h.SetBase("1.0.0.0", "255.0.0.0");
        ipv4h.Assign(internetDevices);
        Ipv4StaticRoutingHelper ipv4RoutingHelper;
        Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
        remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);
        internet.Install(pedestrianNodes);
        internet.Install(carNodes);

        Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(pedNetDev));
        Ipv4InterfaceContainer ueCarIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(carNetDev));
        Packet::EnablePrinting (); 
        
        // Set the default gateway for the UEs
        for (uint32_t j = 0; j < pedestrianNodes.GetN(); ++j)
        {
                Ptr<Ipv4StaticRouting> ueStaticRouting =
                    ipv4RoutingHelper.GetStaticRouting(pedestrianNodes.Get(j)->GetObject<Ipv4>());
                ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
        }
        
        // Set the default gateway for the UEs
        for (uint32_t j = 0; j < carNodes.GetN(); ++j)
        {
                Ptr<Ipv4StaticRouting> ueStaticRouting =
                    ipv4RoutingHelper.GetStaticRouting(carNodes.Get(j)->GetObject<Ipv4>());
                ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
        }

        // attach UEs to the closest eNB
        nrHelper->AttachToClosestEnb(pedNetDev, gNbNetDev);
        nrHelper->AttachToClosestEnb(carNetDev, gNbNetDev);


    // assign IP address to UEs, and install UDP downlink applications
    uint16_t dlPort = 7777;
    // The bearer that will carry low latency traffic
    EpsBearer bearer(EpsBearer::GBR_CONV_VOICE);

    Ptr<EpcTft> tft = Create<EpcTft>();
    EpcTft::PacketFilter dlpf;
    dlpf.localPortStart = dlPort;
    dlpf.localPortEnd = dlPort;
    tft->Add(dlpf);
    
    //to produce the mapping of the BS and an UE
   /* Ptr<NrRadioEnvironmentMapHelper> remHelper = CreateObject<NrRadioEnvironmentMapHelper> ();
    remHelper->SetMinX(-300);
    remHelper->SetMaxX(600);
    remHelper->SetMinY(-400);
    remHelper->SetMaxY(400);
    remHelper->CreateRem (gNbNetDev, pedNetDev.Get(0), 0);*/

    /*
     * Let's install the applications!
     */
	//Create our Two UDP applications
	Ptr <EdgeApplication> udpEdge = CreateObject <EdgeApplication> ();
	Ptr <CloudApplication> udpCloud = CreateObject <CloudApplication> ();

	//Set the start & stop times


	udpEdge->SetStartTime (Seconds(0));
	udpEdge->SetStopTime (Seconds (simTime));

	udpCloud->SetStartTime (Seconds(0));
	udpCloud->SetStopTime (Seconds (simTime));

	//install one application at node 0, and the other at node 1
	pgw->AddApplication (udpEdge);
	//install app to cloud
	remoteHost->AddApplication (udpCloud);
	
	        
        //setting in global variables the cloud ip 
	Ptr<Ipv4> ipv4 = remoteHost->GetObject<Ipv4> ();
	Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0); 	
        cloud_ip=iaddr.GetLocal ();

        
	//Install the Applications in cars

	for (uint32_t i=0; i<carNodes.GetN(); i++)
	{	
	
		Ptr<NetDevice> ueCarDevice = carNetDev.Get(i);
		// Activate a dedicated bearer for the traffic type
		nrHelper->ActivateDedicatedEpsBearer(ueCarDevice, bearer, tft);
		
		Ptr<CarApplication> app_i = CreateObject<CarApplication>();
		app_i->SetStartTime (Seconds (0));
		app_i->SetStopTime (Seconds (simTime));
		carNodes.Get(i)->AddApplication (app_i);
	}

	for (uint32_t i = 0; i < pedestrianNodes.GetN(); ++i)
	{
		Ptr<Node> ue = pedestrianNodes.Get(i);
		Ptr<NetDevice> ueDevice = pedNetDev.Get(i);

		// Activate a dedicated bearer for the traffic type
		nrHelper->ActivateDedicatedEpsBearer(ueDevice, bearer, tft);
		//Install the Applications in users 
		Ptr <PedApplication> udpPed_i = CreateObject <PedApplication>();
		udpPed_i->SetStartTime (Seconds(0));
		udpPed_i->SetStopTime (Seconds (simTime));
		pedestrianNodes.Get (i)->AddApplication (udpPed_i);
		Ptr<Ipv4> ipv4 = pgw->GetObject<Ipv4> ();
		Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0); 	
        	gNb_ip=iaddr.GetLocal ();
                
	}

    
        //LogComponentEnable ("EdgeApplication", LOG_LEVEL_INFO);
        //LogComponentEnable ("CloudApplication", LOG_LEVEL_INFO);
        //LogComponentEnable ("PedApplication", LOG_LEVEL_INFO);
        //LogComponentEnable ("CarApplication", LOG_LEVEL_INFO);



    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    Simulator::Destroy();

}