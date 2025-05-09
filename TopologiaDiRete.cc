#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/aodv-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/log.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TesinaAdHoc"); 

int main(int argc, char *argv[]) {
	std::string protocol = "AODV"; 
	CommandLine cmd;
	cmd.AddValue("protocol", "Routing protocol to use: AODV or DSDV", protocol); 
	cmd.Parse(argc, argv);

	NodeContainer nodes;
	nodes.Create(10); 
	MobilityHelper mobility;
	mobility.SetPositionAllocator("ns3::GridPositionAllocator",
			"MinX", DoubleValue(0.0), 
			"MinY", DoubleValue(0.0), 
			"DeltaX", DoubleValue(150), 
			"DeltaY", DoubleValue(150), 
			"GridWidth", UintegerValue(5),
			"LayoutType", StringValue("RowFirst")); 
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility.Install(nodes);

	WifiHelper wifi;
	wifi.SetStandard(WIFI_STANDARD_80211g);

	WifiMacHelper mac;
	mac.SetType("ns3::AdhocWifiMac");

	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
			StringValue("ErpOfdmRate18Mbps"), // @suppress("Abstract class cannot be instantiated") // @suppress("Symbol is not resolved")
			"ControlMode", StringValue("ErpOfdmRate18Mbps")); // @suppress("Abstract class cannot be instantiated") // @suppress("Symbol is not resolved")

	YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
	YansWifiPhyHelper phy; // @suppress("Abstract class cannot be instantiated")
	phy.SetChannel(channel.Create());
	phy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

	NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

	InternetStackHelper stack; // @suppress("Abstract class cannot be instantiated")
	if (protocol == "AODV") {
		AodvHelper aodv;
		stack.SetRoutingHelper(aodv);
	} else {
		DsdvHelper dsdv;
		stack.SetRoutingHelper(dsdv);
	}
	stack.Install(nodes);

	Ipv4AddressHelper address;
	address.SetBase("10.0.0.0", "255.255.255.0");
	Ipv4InterfaceContainer interfaces = address.Assign(devices);

	uint16_t port = 9; // @suppress("Type cannot be resolved")
	for (uint32_t i = 1; i < nodes.GetN(); ++i) { // @suppress("Type cannot be resolved")
		OnOffHelper onoff("ns3::TcpSocketFactory", // @suppress("Invalid arguments")
				Address(InetSocketAddress(interfaces.GetAddress(i), port))); // @suppress("Symbol is not resolved") // @suppress("Invalid arguments")
		onoff.SetAttribute("DataRate", StringValue("3Mbps")); // @suppress("Invalid arguments") // @suppress("Abstract class cannot be instantiated") // @suppress("Symbol is not resolved")
		onoff.SetAttribute("PacketSize", UintegerValue(1024)); // @suppress("Invalid arguments") // @suppress("Abstract class cannot be instantiated") // @suppress("Symbol is not resolved")
		onoff.SetAttribute("OnTime", // @suppress("Invalid arguments")
				StringValue("ns3::ConstantRandomVariable[Constant=60]")); // @suppress("Invalid arguments") // @suppress("Abstract class cannot be instantiated") // @suppress("Symbol is not resolved")
		onoff.SetAttribute("OffTime", // @suppress("Invalid arguments")
				StringValue("ns3::ConstantRandomVariable[Constant=0]")); // @suppress("Invalid arguments") // @suppress("Abstract class cannot be instantiated") // @suppress("Symbol is not resolved")
		ApplicationContainer app = onoff.Install(nodes.Get(0)); // @suppress("Invalid arguments")
		app.Start(Seconds(31.0));
		app.Stop(Seconds(60.0));

	}

	for (uint32_t i = 1; i < nodes.GetN(); ++i) { // @suppress("Type cannot be resolved")
		PacketSinkHelper sink("ns3::TcpSocketFactory", // @suppress("Invalid arguments")
				Address(InetSocketAddress(Ipv4Address::GetAny(), port))); // @suppress("Symbol is not resolved")
		ApplicationContainer sinkApp = sink.Install(nodes.Get(i)); // @suppress("Invalid arguments")
		sinkApp.Start(Seconds(30.0));  // un po’ prima
		sinkApp.Stop(Seconds(61.0));   // margine finale

	}

	// Attiva log per debug (opzionale)
	LogComponentEnable("PacketSink", LOG_LEVEL_INFO); // @suppress("Invalid arguments")
	LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO); // @suppress("Invalid arguments")

	Simulator::Stop(Seconds(65.0));

	AnimationInterface anim("tesina-topologia.xml"); // crea il file XML per NetAnim // @suppress("Invalid arguments")

	// Opzionale: imposta dimensioni e label
	for (uint32_t i = 0; i < nodes.GetN(); ++i) { // @suppress("Type cannot be resolved")
		anim.UpdateNodeDescription(nodes.Get(i), "Node " + std::to_string(i)); // etichetta // @suppress("Function cannot be resolved") // @suppress("Invalid arguments")
		anim.UpdateNodeColor(nodes.Get(i), 0, 255, 0); // verde // @suppress("Invalid arguments")
	}

	Ptr<FlowMonitor> flowMonitor;
	FlowMonitorHelper flowHelper;
	flowMonitor = flowHelper.InstallAll();

	Simulator::Run();
	flowMonitor->CheckForLostPackets();
	flowMonitor->SerializeToXmlFile("flowmonitor-output.xml", true, true); // @suppress("Invalid arguments")

	Simulator::Destroy();

	return 0;
}
