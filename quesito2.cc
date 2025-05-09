#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/aodv-module.h"
#include "ns3/dsdv-module.h"
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SecondSimulationCampaign");

Ptr<PacketSink> sinkApp;
std::ofstream output;

void
CalculateThroughput (std::string name) //we select a target flow and calculate the instant throughput
{
	Time now = Simulator::Now ();                                         /* Return the simulator's virtual time. */
	double cur = (my_sink->GetTotalRx () - lastTotalRx) * (double) 8 / 1e5;     /* Convert Application RX Packets to MBits. */
	lastTotalRx = my_sink->GetTotalRx ();

	std::ofstream myfile; //open file without overwrite previous stats
	myfile.open ("instant_thr_"+name+".txt", std::ios_base::app);

	myfile << now.GetSeconds () << " \t" << cur << std::endl; //print time and Mbit/sec for stats!
	//NB: ‘\t’ is a horizontal tab . It is used for giving tab space horizontally in your output.

	myfile.close();


	Simulator::Schedule (MilliSeconds (100), &CalculateThroughput, name);

}

int main(int argc, char *argv[]) {
    bool useAodv = true;
    CommandLine cmd(__FILE__);
    cmd.AddValue("useAodv", "Usa AODV se true, DSDV se false", useAodv);
    cmd.Parse(argc, argv);

    NodeContainer nodes;
    nodes.Create(10);

    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
        "MinX", DoubleValue(0.0),
        "MinY", DoubleValue(0.0),
        "DeltaX", DoubleValue(50), 
        "DeltaY", DoubleValue(50),
        "GridWidth", UintegerValue(5),
        "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);

    YansWifiPhyHelper phy;
    

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");

    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    InternetStackHelper stack;
    if (useAodv) {
        AodvHelper aodv;
        stack.SetRoutingHelper(aodv);
        
    } else {
        DsdvHelper dsdv;
        stack.SetRoutingHelper(dsdv);
        
    }
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    //flusso campione
	uint16_t port = 9;
	PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
	ApplicationContainer apps_sink = sink.Install (nodes.Get (1));
	my_sink = StaticCast<PacketSink> (apps_sink.Get (0)); //for tracing instant throughput
	
	Ipv4Address sinkAddress = interfaces.GetAddress (1);
	static const uint32_t packetSize = 1024;
	OnOffHelper onoff ("ns3::TcpSocketFactory", InetSocketAddress (sinkAddress, port));
	onoff.SetConstantRate (DataRate ("3Mb/s"), packetSize);
	onoff.SetAttribute ("StartTime", TimeValue (Seconds (0.0)));
	onoff.SetAttribute ("StopTime", TimeValue (Seconds (60.0)));
	onoff.SetAttribute ("PacketSize", UintegerValue (packetSize));
	onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=9]"));
	onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
	//	onoff.SetAttribute ("DataRate", StringValue ("32Mb/s"));
	ApplicationContainer apps_source = onoff.Install (nodes.Get (1)); //sending pkts

	apps_source.Start(Seconds(31.0));
	apps_source.Stop(Seconds(60.0));
    

    // Flussi concorrenti (non tracciati)
    /*for (uint32_t i = 1; i <= 3; ++i) {
        uint32_t dest = (i + 5) % 9;
        uint16_t port2 = 10;
		PacketSinkHelper sink2 ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port2));
		ApplicationContainer apps_sink2 = sink2.Install (nodes.Get (dest));


		Ipv4Address sinkAddress2 = interfaces.GetAddress (dest);
		OnOffHelper onoff2 ("ns3::TcpSocketFactory", InetSocketAddress (sinkAddress2, port2));
		onoff2.SetConstantRate (DataRate ("3Mb/s"), packetSize);
		onoff2.SetAttribute ("StartTime", TimeValue (Seconds (0.0)));
		onoff2.SetAttribute ("StopTime", TimeValue (Seconds (60.0)));
		onoff2.SetAttribute ("PacketSize", UintegerValue (packetSize));
		onoff2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=5]"));
		onoff2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
		//		onoff2.SetAttribute ("DataRate", StringValue ("32Mb/s"));
		ApplicationContainer apps_source2 = onoff2.Install (nodes.Get (1)); //sending pkts

		apps_source2.Start(Seconds(31.0));
		apps_source2.Stop(Seconds(60.0));
    }*/

    //flusso concorrente 1
    uint16_t port2 = 10;
	PacketSinkHelper sink2 ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port2));
	ApplicationContainer apps_sink2 = sink2.Install (nodes.Get (4));


	Ipv4Address sinkAddress2 = interfaces.GetAddress (4);
	OnOffHelper onoff2 ("ns3::TcpSocketFactory", InetSocketAddress (sinkAddress2, port2));
	onoff2.SetConstantRate (DataRate ("3Mb/s"), packetSize);
	onoff2.SetAttribute ("StartTime", TimeValue (Seconds (0.0)));
	onoff2.SetAttribute ("StopTime", TimeValue (Seconds (60.0)));
	onoff2.SetAttribute ("PacketSize", UintegerValue (packetSize));
	onoff2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=7]"));
	onoff2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
	//		onoff2.SetAttribute ("DataRate", StringValue ("32Mb/s"));
	ApplicationContainer apps_source2 = onoff2.Install (nodes.Get (1)); //sending pkts

	apps_source2.Start(Seconds(31.0));
	apps_source2.Stop(Seconds(60.0));

    //flusso concorrente 2
    uint16_t port3 = 11;
	PacketSinkHelper sink3 ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port3));
	ApplicationContainer apps_sink3 = sink3.Install (nodes.Get (5));


	Ipv4Address sinkAddress3 = interfaces.GetAddress (5);
	OnOffHelper onoff3 ("ns3::TcpSocketFactory", InetSocketAddress (sinkAddress3, port3));
	onoff3.SetConstantRate (DataRate ("3Mb/s"), packetSize);
	onoff3.SetAttribute ("StartTime", TimeValue (Seconds (0.0)));
	onoff3.SetAttribute ("StopTime", TimeValue (Seconds (60.0)));
	onoff3.SetAttribute ("PacketSize", UintegerValue (packetSize));
	onoff3.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=5]"));
	onoff3.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
	//		onoff2.SetAttribute ("DataRate", StringValue ("32Mb/s"));
	ApplicationContainer apps_source3 = onoff3.Install (nodes.Get (1)); //sending pkts

	apps_source3.Start(Seconds(31.0));
	apps_source3.Stop(Seconds(60.0));

    //flusso concorrente 3
    uint16_t port4 = 12;
	PacketSinkHelper sink4 ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port4));
	ApplicationContainer apps_sink4 = sink4.Install (nodes.Get (6));


	Ipv4Address sinkAddress4 = interfaces.GetAddress (6);
	OnOffHelper onoff4 ("ns3::TcpSocketFactory", InetSocketAddress (sinkAddress4, port4));
	onoff4.SetConstantRate (DataRate ("3Mb/s"), packetSize);
	onoff4.SetAttribute ("StartTime", TimeValue (Seconds (0.0)));
	onoff4.SetAttribute ("StopTime", TimeValue (Seconds (60.0)));
	onoff4.SetAttribute ("PacketSize", UintegerValue (packetSize));
	onoff4.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=3]"));
	onoff4.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
	//		onoff2.SetAttribute ("DataRate", StringValue ("32Mb/s"));
	ApplicationContainer apps_source4 = onoff4.Install (nodes.Get (1)); //sending pkts

	apps_source4.Start(Seconds(31.0));
	apps_source4.Stop(Seconds(60.0));






    std::string fileName = useAodv ? "AODV" : "DSDV";
    Simulator::Schedule (Seconds (31.1), &CalculateThroughput, fileName);

    
    Simulator::Stop(Seconds(60.0));
    Simulator::Run();
    
    Simulator::Destroy();

    return 0;
}

