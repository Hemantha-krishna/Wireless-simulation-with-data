#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"

using namespace ns3;

// Custom application to send "Hello world" messages
class HelloWorldSender : public Application
{
public:
    static TypeId GetTypeId(void);
    HelloWorldSender();
    virtual ~HelloWorldSender();

    void Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t numPackets, DataRate dataRate);

protected:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

private:
    void ScheduleTx(void);
    void SendPacket(void);

    Ptr<Socket> m_socket;
    Address m_peer;
    uint32_t m_packetSize;
    uint32_t m_numPackets;
    DataRate m_dataRate;
    EventId m_sendEvent;
};

NS_OBJECT_ENSURE_REGISTERED(HelloWorldSender);

TypeId HelloWorldSender::GetTypeId(void)
{
    static TypeId tid = TypeId("HelloWorldSender")
        .SetParent<Application>()
        .SetGroupName("Applications")
        .AddConstructor<HelloWorldSender>()
        ;
    return tid;
}

HelloWorldSender::HelloWorldSender()
    : m_socket(0),
      m_peer(),
      m_packetSize(0),
      m_numPackets(0),
      m_dataRate(0),
      m_sendEvent()
{
}

HelloWorldSender::~HelloWorldSender()
{
    m_socket = 0;
}

void HelloWorldSender::Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t numPackets, DataRate dataRate)
{
    m_socket = socket;
    m_peer = address;
    m_packetSize = packetSize;
    m_numPackets = numPackets;
    m_dataRate = dataRate;

    // Correctly connect the socket to the remote address
    m_socket->Connect(m_peer);
}

void HelloWorldSender::StartApplication(void)
{
    m_socket->Bind();
    m_socket->Connect(m_peer);
    SendPacket();
}

void HelloWorldSender::StopApplication(void)
{
    Simulator::Cancel(m_sendEvent);
}

void HelloWorldSender::ScheduleTx(void)
{
    if (m_numPackets > 0)
    {
        Time tNext = Seconds(m_packetSize * 8 / static_cast<double>(m_dataRate.GetBitRate()));
        m_sendEvent = Simulator::Schedule(tNext, &HelloWorldSender::SendPacket, this);
    }
}

void HelloWorldSender::SendPacket(void)
{
    // Create a packet with a payload containing "Hello world"
    std::string message = "Hello world";
    Ptr<Packet> packet = Create<Packet>(reinterpret_cast<const uint8_t*>(message.c_str()), message.size());

    // Add UDP header to the packet
    UdpHeader udpHeader;
    udpHeader.SetSourcePort(0); // Set source port
    udpHeader.SetDestinationPort(0); // Set destination port
    packet->AddHeader(udpHeader);

    // Send the packet
    m_socket->Send(packet);

    if (--m_numPackets > 0)
    {
        ScheduleTx();
    }
}



int main(int argc, char *argv[]) {
    uint32_t numNodes = 10;
    double simulationTime = 100.0;

    NodeContainer nodes;
    nodes.Create(numNodes);

    NodeContainer monitoringNode;
    monitoringNode.Create(1);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue(Rectangle(-500, 500, -500, 500)));
    mobility.Install(nodes);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(monitoringNode);

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211n);

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel");

    YansWifiPhyHelper wifiPhy;
    wifiPhy.Set("RxGain", DoubleValue(0));
    wifiPhy.Set("TxGain", DoubleValue(0));
    wifiPhy.Set("RxNoiseFigure", DoubleValue(7));
    wifiPhy.Set("TxPowerStart", DoubleValue(16.0206));
    wifiPhy.Set("TxPowerEnd", DoubleValue(16.0206));
    wifiPhy.Set("CcaEdThreshold", DoubleValue(-62.8));
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiMacHelper wifiMac;
    NetDeviceContainer wifiDevices = wifi.Install(wifiPhy, wifiMac, nodes);
    wifiPhy.EnablePcap("wifi-capture", wifiDevices, true);

    PointToPointHelper p2pHelper;
    p2pHelper.EnablePcapAll("wireless-capture", true);

    std::vector<Ptr<PointToPointNetDevice>> monitoringDevices;

    for (uint32_t i = 0; i < numNodes; ++i) {
        Ptr<Node> node = nodes.Get(i);
        NetDeviceContainer link = p2pHelper.Install(node, monitoringNode.Get(0));
        Ptr<PointToPointNetDevice> device = DynamicCast<PointToPointNetDevice>(link.Get(1));

        if (device != nullptr) {
            monitoringDevices.push_back(device);
            p2pHelper.EnablePcap("monitoring-capture", device, true);
        }
    }

    InternetStackHelper internet;
    internet.Install(nodes);
    internet.Install(monitoringNode);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(wifiDevices);

    uint16_t port = 9;

    // Server to receive packets containing "Hello world"
    PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp = sinkHelper.Install(monitoringNode.Get(0));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(simulationTime));

    // Clients to send packets containing "Hello world"
for (uint32_t i = 0; i < numNodes; ++i) {
    for (uint32_t j = 0; j < numNodes; ++j) {
        if (i != j) {
            Ptr<Socket> socket = Socket::CreateSocket(nodes.Get(i), UdpSocketFactory::GetTypeId());
            AddressValue remoteAddress(InetSocketAddress(interfaces.GetAddress(j), port));

            Ptr<HelloWorldSender> app = CreateObject<HelloWorldSender>();
            app->Setup(socket, InetSocketAddress(interfaces.GetAddress(j), port), 1024, 100, DataRate("500kb/s"));
            nodes.Get(i)->AddApplication(app);
            app->SetStartTime(Seconds(1.0));
            app->SetStopTime(Seconds(simulationTime));
        }
    }
}


    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    AnimationInterface anim("wireless-animation.xml");
    anim.EnablePacketMetadata(true);

    PacketSinkHelper monitoringSink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 9999));
    ApplicationContainer monitoringApp = monitoringSink.Install(monitoringNode.Get(0));
    monitoringApp.Start(Seconds(0.0));
    monitoringApp.Stop(Seconds(simulationTime + 1));

    AsciiTraceHelper ascii;
    p2pHelper.EnableAsciiAll(ascii.CreateFileStream("wireless-capture.tr"));

    Simulator::Stop(Seconds(simulationTime + 1));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

