/**
 * Cloud and Edge Computing Simulation with 5G NR Technology
 * 
 * This simulation models a 5G NR network with vehicles, pedestrians, edge nodes,
 * and cloud computing capabilities to evaluate different offloading strategies.
 * 
 * The simulation includes:
 * - A configurable number of vehicles with mobility patterns from SUMO
 * - A configurable number of pedestrian UEs in fixed positions
 * - A 5G gNB (base station) with configurable parameters
 * - Edge and cloud computing nodes with different processing capabilities
 */

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
#include "pedestrians-application.h"
#include "car-application.h"
#include "globals.h"

// Terminal color codes for log formatting
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

NS_LOG_COMPONENT_DEFINE("CloudEdgeOffloading");

int main(int argc, char* argv[])
{
    //-------------------------------------------------------------------------
    // Default simulation parameters
    //-------------------------------------------------------------------------
    
    // Simulation time
    double simTime = 120.0;   // seconds
    
    // 5G NR parameters
    uint16_t numerology = 2;
    uint16_t gNbNum = 1;
    double centralFrequency = 6e9;      // 6 GHz
    double bandwidth = 200e6;           // 200 MHz
    double txPowergNb = 20;             // dBm
    double txPowerUe = 23;              // dBm
    uint8_t fixedMcs = 27;              // Modulation and coding scheme
    bool useFixedMcs = true;
    
    // Node positions
    double gNBPosition_x = 1000;
    double gNBPosition_y = 1000;
    double gNBPosition_z = 30;          // Height of gNB
    double UEPedestrianDistanceFromgNB = 20;
    double UEPosition_z = 1.5;          // Height of UEs
    
    // Application parameters
    uint32_t udpPacketSize = 1000;      // bytes
    double workload = 1028;             // Million instructions (from literature for light tasks)
    uint32_t nVehicles = 1;
    uint32_t nPedestrians = 2;
    uint32_t vehicleQueueLength = 1;
    double requestRate = 1000;          // Milliseconds
    
    // Computation capacities (MIPS)
    double cloudComputationCapacity = 2356230.0;
    double edgeComputationCapacity = 749070.0;
    double carComputationCapacity = 221720.0;
    
    // Output parameters
    std::string outputDataCsv;
    std::string chosenStrategy;
    std::string traceFile;
    std::string simType;
    
    //-------------------------------------------------------------------------
    // Command line parameter parsing
    //-------------------------------------------------------------------------
    CommandLine cmd(__FILE__);
    
    // Output parameters
    cmd.AddValue("outputDataCsv", "CSV output data folder", outputDataCsv);
    cmd.AddValue("strategy", "Offloading strategy to use", chosenStrategy);
    cmd.AddValue("traceFile", "NS2 mobility trace file", traceFile);
    cmd.AddValue("simType", "Simulation type", simType);
    
    // Simulation parameters
    cmd.AddValue("simTime", "Duration of the simulation (s)", simTime);
    
    // Node position parameters
    cmd.AddValue("gNBPosition_x", "x-position of gNB", gNBPosition_x);
    cmd.AddValue("gNBPosition_y", "y-position of gNB", gNBPosition_y);
    cmd.AddValue("gNBPosition_z", "z-position of gNB", gNBPosition_z);
    cmd.AddValue("UEPedestrianDistanceFromgNB", "Distance of pedestrians from gNB", UEPedestrianDistanceFromgNB);
    cmd.AddValue("UEPosition_z", "z-position of all UEs", UEPosition_z);
    
    // Application parameters
    cmd.AddValue("packetSize", "Size of packet (bytes)", udpPacketSize);
    cmd.AddValue("workload", "Workload per task (MI)", workload);
    cmd.AddValue("nVehicles", "Number of vehicles", nVehicles);
    cmd.AddValue("nPedestrians", "Number of pedestrians", nPedestrians);
    cmd.AddValue("carQueueLength", "Queue length in vehicles", vehicleQueueLength);
    cmd.AddValue("requestRate", "Request rate (ms)", requestRate);
    
    // Computation capacity parameters
    cmd.AddValue("edgeComputationCapacity", "Edge computation capacity (MIPS)", edgeComputationCapacity);
    cmd.AddValue("carComputationCapacity", "Vehicle computation capacity (MIPS)", carComputationCapacity);
    cmd.AddValue("cloudComputationCapacity", "Cloud computation capacity (MIPS)", cloudComputationCapacity);
    
    cmd.Parse(argc, argv);
    
    //-------------------------------------------------------------------------
    // Set global variables for access from other simulator components
    //-------------------------------------------------------------------------
    packet_size = udpPacketSize;
    task_workload = workload;
    output_data_csv = outputDataCsv;
    chosen_strategy = chosenStrategy;
    request_rate = requestRate;
    cars_number = nVehicles;
    pedestrians_number = nPedestrians;
    vehicle_queue_length = vehicleQueueLength;
    cloud_computation_capacity = cloudComputationCapacity;
    edge_computation_capacity = edgeComputationCapacity;
    car_computation_capacity = carComputationCapacity;
    sim_type = simType;
    
    //-------------------------------------------------------------------------
    // Create nodes and set up mobility
    //-------------------------------------------------------------------------
    
    // Create vehicle nodes and set up SUMO mobility
    NodeContainer carNodes;
    carNodes.Create(nVehicles);
    Ns2MobilityHelper sumoMobility = Ns2MobilityHelper(traceFile);
    sumoMobility.Install(); // Configure movements for each vehicle from trace file
    
    // Ensure we have at least one pedestrian
    NS_ASSERT(nPedestrians > 0);
    
    //-------------------------------------------------------------------------
    // Set up the 5G NR network
    //-------------------------------------------------------------------------
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
    
    // Spectrum configuration: create one operation band with one component carrier
    // The CC contains a single Bandwidth Part (BWP) that occupies the entire bandwidth
    CcBwpCreator ccBwpCreator;
    const uint8_t numCcPerBand = 1;
    BandwidthPartInfo::Scenario scenario = BandwidthPartInfo::UMa_LoS;
    
    // Create a configuration for the CcBwpHelper
    CcBwpCreator::SimpleOperationBandConf bandConf(centralFrequency,
                                                  bandwidth,
                                                  numCcPerBand,
                                                  scenario);
    
    // Create the operation band with the configuration
    OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc(bandConf);
    
    // Initialize channel, pathloss, and other spectrum parameters
    nrHelper->InitializeOperationBand(&band);
    BandwidthPartInfoPtrVector allBwps = CcBwpCreator::GetAllBwps({band});
    
    // Set SRS periodicity
    Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(320));
    
    //-------------------------------------------------------------------------
    // Configure 5G NR parameters for gNB and UEs
    //-------------------------------------------------------------------------
    
    // gNB configuration
    nrHelper->SetGnbPhyAttribute("TxPower", DoubleValue(txPowergNb));
    nrHelper->SetGnbPhyAttribute("Numerology", UintegerValue(numerology));
    
    // UE configuration
    nrHelper->SetUePhyAttribute("TxPower", DoubleValue(txPowerUe));
    
    // Scheduler configuration
    nrHelper->SetSchedulerTypeId(TypeId::LookupByName("ns3::NrMacSchedulerTdmaRR"));
    nrHelper->SetSchedulerAttribute("FixedMcsDl", BooleanValue(useFixedMcs));
    nrHelper->SetSchedulerAttribute("FixedMcsUl", BooleanValue(useFixedMcs));
    
    if (useFixedMcs) {
        nrHelper->SetSchedulerAttribute("StartingMcsDl", UintegerValue(fixedMcs));
        nrHelper->SetSchedulerAttribute("StartingMcsUl", UintegerValue(fixedMcs));
    }
    
    // Set RLC buffer size
    Config::SetDefault("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));
    
    // Configure UE antennas (1x1 SISO)
    nrHelper->SetUeAntennaAttribute("NumRows", UintegerValue(1));
    nrHelper->SetUeAntennaAttribute("NumColumns", UintegerValue(1));
    nrHelper->SetUeAntennaAttribute("AntennaElement",
                                  PointerValue(CreateObject<IsotropicAntennaModel>()));
    
    // Configure gNB antennas (8x8 MIMO)
    nrHelper->SetGnbAntennaAttribute("NumRows", UintegerValue(8));
    nrHelper->SetGnbAntennaAttribute("NumColumns", UintegerValue(8));
    nrHelper->SetGnbAntennaAttribute("AntennaElement",
                                   PointerValue(CreateObject<ThreeGppAntennaModel>()));
    
    // Configure beamforming
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    idealBeamformingHelper->SetAttribute("BeamformingMethod",
                                       TypeIdValue(DirectPathBeamforming::GetTypeId()));
    nrHelper->SetBeamformingHelper(idealBeamformingHelper);
    
    // Configure channel update period
    Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(MilliSeconds(0)));
    
    // Disable shadowing for deterministic results
    nrHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));
    
    // Configure error models
    nrHelper->SetUlErrorModel("ns3::NrEesmIrT1");
    nrHelper->SetDlErrorModel("ns3::NrEesmIrT1");
    
    // Configure AMC models
    nrHelper->SetGnbDlAmcAttribute("AmcModel", EnumValue(NrAmc::ErrorModel));
    nrHelper->SetGnbUlAmcAttribute("AmcModel", EnumValue(NrAmc::ErrorModel));
    
    //-------------------------------------------------------------------------
    // Set up the EPC (core network)
    //-------------------------------------------------------------------------
    Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper>();
    nrHelper->SetEpcHelper(epcHelper);
    
    // Set S1-U link delay (core network latency)
    epcHelper->SetAttribute("S1uLinkDelay", TimeValue(MilliSeconds(2)));
    
    // Configure bearer-to-BWP mapping
    uint32_t bwpIdForBearer = 0;
    nrHelper->SetGnbBwpManagerAlgorithmAttribute("GBR_CONV_VOICE", UintegerValue(bwpIdForBearer));
    
    // Initialize the NR helper
    nrHelper->Initialize();
    
    //-------------------------------------------------------------------------
    // Create and position nodes
    //-------------------------------------------------------------------------
    
    // Create gNB and pedestrian nodes
    NodeContainer gNbNodes;
    NodeContainer pedestrianNodes;
    
    gNbNodes.Create(gNbNum);
    pedestrianNodes.Create(nPedestrians);
    
    // Position the gNB
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    Ptr<ListPositionAllocator> bsPositionAlloc = CreateObject<ListPositionAllocator>();
    bsPositionAlloc->Add(Vector(gNBPosition_x, gNBPosition_y, gNBPosition_z));
    mobility.SetPositionAllocator(bsPositionAlloc);
    mobility.Install(gNbNodes);
    
    // Position pedestrians
    Ptr<ListPositionAllocator> utPositionAlloc = CreateObject<ListPositionAllocator>();
    for (uint32_t i = 0; i < nPedestrians; ++i) {
        // Position pedestrians near the gNB with small offsets to avoid overlap
        utPositionAlloc->Add(Vector(gNBPosition_x - UEPedestrianDistanceFromgNB, 
                                   gNBPosition_y + 0.01 * i, 
                                   UEPosition_z));
    }
    mobility.SetPositionAllocator(utPositionAlloc);
    mobility.Install(pedestrianNodes);
    
    //-------------------------------------------------------------------------
    // Install network devices
    //-------------------------------------------------------------------------
    
    // Install gNB devices
    NetDeviceContainer gNbNetDev = nrHelper->InstallGnbDevice(gNbNodes, allBwps);
    
    // Install UE devices for pedestrians and vehicles
    NetDeviceContainer pedNetDev = nrHelper->InstallUeDevice(pedestrianNodes, allBwps);
    NetDeviceContainer carNetDev = nrHelper->InstallUeDevice(carNodes, allBwps);
    
    // Assign random number streams for reproducibility
    int64_t randomStream = 1;
    randomStream += nrHelper->AssignStreams(gNbNetDev, randomStream);
    randomStream += nrHelper->AssignStreams(pedNetDev, randomStream);
    randomStream += nrHelper->AssignStreams(carNetDev, randomStream);
    
    // Update configuration for all devices
    for (auto it = gNbNetDev.Begin(); it != gNbNetDev.End(); ++it) {
        DynamicCast<NrGnbNetDevice>(*it)->UpdateConfig();
    }
    
    for (auto it = pedNetDev.Begin(); it != pedNetDev.End(); ++it) {
        DynamicCast<NrUeNetDevice>(*it)->UpdateConfig();
    }
    
    for (auto it = carNetDev.Begin(); it != carNetDev.End(); ++it) {
        DynamicCast<NrUeNetDevice>(*it)->UpdateConfig();
    }
    
    //-------------------------------------------------------------------------
    // Set up the internet and IP connectivity
    //-------------------------------------------------------------------------
    
    // Get SGW/PGW and create a remote host (cloud)
    Ptr<Node> pgw = epcHelper->GetPgwNode();
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create(1);
    Ptr<Node> remoteHost = remoteHostContainer.Get(0);
    
    // Install internet stack on remote host
    InternetStackHelper internet;
    internet.Install(remoteHostContainer);
    
    // Connect remote host to PGW with a point-to-point link
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
    p2ph.SetDeviceAttribute("Mtu", UintegerValue(2500));
    p2ph.SetChannelAttribute("Delay", TimeValue(MilliSeconds(35))); // Internet delay based on literature
    
    NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
    
    // Assign IP addresses to the internet devices
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase("1.0.0.0", "255.0.0.0");
    ipv4h.Assign(internetDevices);
    
    // Set up routing to UE network
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
    remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);
    
    // Install internet stack on UEs
    internet.Install(pedestrianNodes);
    internet.Install(carNodes);
    
    // Assign IP addresses to UEs
    Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(pedNetDev));
    Ipv4InterfaceContainer ueCarIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(carNetDev));
    
    // Enable packet printing
    Packet::EnablePrinting();
    
    // Set default gateway for pedestrian UEs
    for (uint32_t j = 0; j < pedestrianNodes.GetN(); ++j) {
        Ptr<Ipv4StaticRouting> ueStaticRouting =
            ipv4RoutingHelper.GetStaticRouting(pedestrianNodes.Get(j)->GetObject<Ipv4>());
        ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
    }
    
    // Set default gateway for vehicle UEs
    for (uint32_t j = 0; j < carNodes.GetN(); ++j) {
        Ptr<Ipv4StaticRouting> ueStaticRouting =
            ipv4RoutingHelper.GetStaticRouting(carNodes.Get(j)->GetObject<Ipv4>());
        ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
    }
    
    // Attach UEs to the closest gNB
    nrHelper->AttachToClosestEnb(pedNetDev, gNbNetDev);
    nrHelper->AttachToClosestEnb(carNetDev, gNbNetDev);
    
    //-------------------------------------------------------------------------
    // Set up bearers and traffic flow templates
    //-------------------------------------------------------------------------
    
    // UDP port for applications
    uint16_t dlPort = 7777;
    
    // Set up bearer for low-latency traffic
    EpsBearer bearer(EpsBearer::GBR_CONV_VOICE);
    
    // Create traffic flow template
    Ptr<EpcTft> tft = Create<EpcTft>();
    EpcTft::PacketFilter dlpf;
    dlpf.localPortStart = dlPort;
    dlpf.localPortEnd = dlPort;
    tft->Add(dlpf);
    
    //-------------------------------------------------------------------------
    // Install applications
    //-------------------------------------------------------------------------
    
    // Create and install edge and cloud applications
    Ptr<EdgeApplication> udpEdge = CreateObject<EdgeApplication>();
    Ptr<CloudApplication> udpCloud = CreateObject<CloudApplication>();
    
    // Set application start and stop times
    udpEdge->SetStartTime(Seconds(0));
    udpEdge->SetStopTime(Seconds(simTime));
    
    udpCloud->SetStartTime(Seconds(0));
    udpCloud->SetStopTime(Seconds(simTime));
    
    // Install applications on nodes
    pgw->AddApplication(udpEdge);
    remoteHost->AddApplication(udpCloud);
    
    // Store cloud IP in global variable for use by other applications
    Ptr<Ipv4> cloudIpv4 = remoteHost->GetObject<Ipv4>();
    Ipv4InterfaceAddress cloudIaddr = cloudIpv4->GetAddress(1, 0);
    cloud_ip = cloudIaddr.GetLocal();
    
    // Install applications on vehicles
    for (uint32_t i = 0; i < carNodes.GetN(); i++) {
        // Activate dedicated bearer for the vehicle
        Ptr<NetDevice> ueCarDevice = carNetDev.Get(i);
        nrHelper->ActivateDedicatedEpsBearer(ueCarDevice, bearer, tft);
        
        // Create and install car application
        Ptr<CarApplication> app_i = CreateObject<CarApplication>();
        app_i->SetStartTime(Seconds(0));
        app_i->SetStopTime(Seconds(simTime));
        carNodes.Get(i)->AddApplication(app_i);
    }
    
    // Install applications on pedestrians
    for (uint32_t i = 0; i < pedestrianNodes.GetN(); ++i) {
        // Activate dedicated bearer for the pedestrian
        Ptr<NetDevice> ueDevice = pedNetDev.Get(i);
        nrHelper->ActivateDedicatedEpsBearer(ueDevice, bearer, tft);
        
        // Create and install pedestrian application
        Ptr<PedApplication> udpPed_i = CreateObject<PedApplication>();
        udpPed_i->SetStartTime(Seconds(0));
        udpPed_i->SetStopTime(Seconds(simTime));
        pedestrianNodes.Get(i)->AddApplication(udpPed_i);
        
        // Store gNB IP in global variable
        Ptr<Ipv4> gNbIpv4 = pgw->GetObject<Ipv4>();
        Ipv4InterfaceAddress gNbIaddr = gNbIpv4->GetAddress(1, 0);
        gNb_ip = gNbIaddr.GetLocal();
    }
    
    // Uncomment to enable application-level logging
    // LogComponentEnable("EdgeApplication", LOG_LEVEL_INFO);
    // LogComponentEnable("CloudApplication", LOG_LEVEL_INFO);
    // LogComponentEnable("PedApplication", LOG_LEVEL_INFO);
    // LogComponentEnable("CarApplication", LOG_LEVEL_INFO);
    
    //-------------------------------------------------------------------------
    // Run the simulation
    //-------------------------------------------------------------------------
    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    Simulator::Destroy();
    
    return 0;
}
