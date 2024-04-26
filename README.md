# NS-3 Wireless Simulation

This is an NS-3 simulation that models a wireless network with multiple nodes and a monitoring node. The nodes move randomly within a specified area, while the monitoring node remains stationary. The simulation includes the following components:

## Components

1. **Node Creation**: The code creates `numNodes` (set to 10 by default) mobile nodes and one monitoring node.
2. **Mobility Model**: The mobile nodes use the `RandomWalk2dMobilityModel` to move randomly within a 1000m x 1000m area, while the monitoring node uses the `ConstantPositionMobilityModel` to remain stationary.
3. **Wi-Fi Configuration**: The Wi-Fi standard is set to 802.11n, with various physical layer parameters configured, such as transmission power, noise figure, and CCA threshold.
4. **Network Setup**: Each node is equipped with a Wi-Fi network interface and a point-to-point link to the monitoring node. The Internet stack is installed on all nodes, and IPv4 addresses are assigned to the Wi-Fi interfaces.
5. **Application**: A custom application called `HelloWorldSender` is created, which sends UDP packets containing the message "Hello world" from each mobile node to all other mobile nodes. The packet size is set to 1024 bytes, and each node sends 100 packets at a data rate of 500 kbps.
6. **Packet Sink**: A `PacketSink` application is installed on the monitoring node to receive the UDP packets sent by the mobile nodes.
7. **Tracing and Visualization**: Packet capture (PCAP) files are enabled for the Wi-Fi and point-to-point links. The `NetAnim` module is used to generate an XML animation file for visualizing the network topology and packet flow.
8. **Logging**: Logging is enabled for the `UdpEchoClientApplication` and `UdpEchoServerApplication` components.

## Running the Simulation

To run the simulation, compile and execute the code. The simulation will run for the specified duration (`simulationTime`, set to 100 seconds by default), during which the mobile nodes will move randomly and send UDP packets containing "Hello world" to each other. The monitoring node will receive these packets.

After the simulation completes, the following output files will be generated:

- `wifi-capture-*.pcap`: PCAP files capturing the Wi-Fi traffic.
- `wireless-capture.tr`: ASCII trace file for the point-to-point links.
- `wireless-animation.xml`: XML animation file for visualizing the network topology and packet flow using `NetAnim`.

## Customization

The simulation can be customized by modifying the following parameters:

- `numNodes`: The number of mobile nodes in the network.
- `simulationTime`: The duration of the simulation in seconds.
- `Rectangle`: The bounds for the mobility model (specified as `Rectangle(-500, 500, -500, 500)` by default).
- Wi-Fi physical layer parameters: Transmission power, noise figure, CCA threshold, etc.
- Application parameters: Packet size, number of packets, data rate, port numbers, etc.

Additionally, you can modify the network topology, mobility models, or add additional applications or protocols as needed.
