#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Traccia17"); // @suppress("Symbol is not resolved")

int main (int argc, char *argv[])
{
  bool useAodv = true;
  uint32_t flowsNumber = 9; // @suppress("Type cannot be resolved")

  uint32_t runNumber = 1; // Default run number // @suppress("Type cannot be resolved")

  CommandLine cmd;
  cmd.AddValue ("useAodv", "Use AODV if true, DSDV if false", useAodv); // @suppress("Invalid arguments")
  cmd.AddValue ("flowsNumber", "Number of active flows", flowsNumber); // @suppress("Invalid arguments")
  cmd.AddValue ("runNumber", "Run number to set different random seeds", runNumber); // @suppress("Invalid arguments")
  cmd.Parse (argc, argv);

  SeedManager::SetRun (runNumber); // @suppress("Invalid arguments")

  NodeContainer nodes;
  nodes.Create (10); // @suppress("Invalid arguments")

  WifiHelper wifi;
  wifi.SetStandard (WIFI_STANDARD_80211g);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue ("ErpOfdmRate18Mbps"), // @suppress("Symbol is not resolved") // @suppress("Abstract class cannot be instantiated")
                                "ControlMode", StringValue ("ErpOfdmRate18Mbps")); // @suppress("Abstract class cannot be instantiated") // @suppress("Symbol is not resolved")


  YansWifiPhyHelper phy; // @suppress("Abstract class cannot be instantiated")
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  phy.SetChannel (channel.Create ());
  phy.Set ("TxPowerStart", DoubleValue (16.0206)); // @suppress("Abstract class cannot be instantiated") // @suppress("Invalid arguments")
  phy.Set ("TxPowerEnd", DoubleValue (16.0206)); // @suppress("Abstract class cannot be instantiated") // @suppress("Invalid arguments")
  WifiMacHelper mac;
  mac.SetType ("ns3::AdhocWifiMac");

  NetDeviceContainer devices = wifi.Install (phy, mac, nodes);

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator", 
                                 "MinX", DoubleValue (0.0), 
                                 "MinY", DoubleValue (0.0), 
                                 "DeltaX", DoubleValue (50), // @suppress("Abstract class cannot be instantiated")
                                 "DeltaY", DoubleValue (50), // @suppress("Abstract class cannot be instantiated")
                                 "GridWidth", UintegerValue (5), // @suppress("Abstract class cannot be instantiated") // @suppress("Symbol is not resolved")
                                 "LayoutType", StringValue ("RowFirst")); // @suppress("Symbol is not resolved") // @suppress("Abstract class cannot be instantiated")
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  InternetStackHelper internet; // @suppress("Abstract class cannot be instantiated")
  if (useAodv)
    {
      AodvHelper aodv;
      internet.SetRoutingHelper (aodv);
    }
  else
    {
      DsdvHelper dsdv;
      internet.SetRoutingHelper (dsdv);
    }
  internet.Install (nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

  uint16_t port = 5000; // @suppress("Type cannot be resolved")
  ApplicationContainer sinkApps;
  ApplicationContainer clientApps;

  for (uint32_t i = 1; i <= flowsNumber; ++i) // @suppress("Type cannot be resolved")
    {
      PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", // @suppress("Invalid arguments")
                                   InetSocketAddress (Ipv4Address::GetAny (), port)); // @suppress("Symbol is not resolved")
      ApplicationContainer sink = sinkHelper.Install (nodes.Get (i)); // @suppress("Invalid arguments")
      sink.Start (Seconds (30.0));
      sink.Stop (Seconds (60.0));
      sinkApps.Add (sink);

      OnOffHelper client ("ns3::TcpSocketFactory", // @suppress("Invalid arguments")
                          Address (InetSocketAddress (interfaces.GetAddress (i), port))); // @suppress("Invalid arguments") // @suppress("Symbol is not resolved")
      client.SetAttribute ("DataRate", StringValue ("3Mbps")); // @suppress("Abstract class cannot be instantiated") // @suppress("Invalid arguments") // @suppress("Symbol is not resolved")
      client.SetAttribute ("PacketSize", UintegerValue (1024)); // @suppress("Invalid arguments") // @suppress("Symbol is not resolved") // @suppress("Abstract class cannot be instantiated")
      client.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]")); // @suppress("Symbol is not resolved") // @suppress("Invalid arguments") // @suppress("Abstract class cannot be instantiated")
      client.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]")); // @suppress("Invalid arguments") // @suppress("Symbol is not resolved") // @suppress("Abstract class cannot be instantiated")

      ApplicationContainer clientApp = client.Install (nodes.Get (0)); // @suppress("Invalid arguments")
      clientApp.Start (Seconds (31.0));
      clientApp.Stop (Seconds (60.0));
      clientApps.Add (clientApp);
    }

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  Simulator::Stop (Seconds (60.0));
  Simulator::Run ();

  monitor->CheckForLostPackets ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats (); // @suppress("Type cannot be resolved") // @suppress("Symbol is not resolved")

  double totalThroughput = 0;
  uint32_t totalTxPackets = 0; // @suppress("Type cannot be resolved")
  uint32_t totalRxPackets = 0; // @suppress("Type cannot be resolved")
  uint32_t totalLostPackets = 0; // @suppress("Type cannot be resolved")

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter) // @suppress("Type cannot be resolved") // @suppress("Method cannot be resolved") // @suppress("Symbol is not resolved")
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first); // @suppress("Invalid arguments") // @suppress("Field cannot be resolved")
      if (t.sourceAddress == interfaces.GetAddress (0)) // @suppress("Invalid arguments")
        {
          double throughput = (iter->second.rxBytes * 8.0) / (29.0 * 1000000.0); // @suppress("Field cannot be resolved")
          totalThroughput += throughput;
          totalTxPackets += iter->second.txPackets; // @suppress("Field cannot be resolved")
          totalRxPackets += iter->second.rxPackets; // @suppress("Field cannot be resolved")
          totalLostPackets += iter->second.lostPackets; // @suppress("Field cannot be resolved")
        }
    }

  std::cout << (useAodv ? "AODV" : "DSDV") << " Results:" << std::endl; // @suppress("Symbol is not resolved")
  std::cout << "Total Throughput (Mbps): " << totalThroughput << std::endl; // @suppress("Symbol is not resolved")
  std::cout << "Total Tx Packets: " << totalTxPackets << std::endl; // @suppress("Symbol is not resolved")
  std::cout << "Total Rx Packets: " << totalRxPackets << std::endl; // @suppress("Symbol is not resolved")
  std::cout << "Total Lost Packets: " << totalLostPackets << std::endl; // @suppress("Symbol is not resolved")

  Simulator::Destroy ();
  return 0;
}
