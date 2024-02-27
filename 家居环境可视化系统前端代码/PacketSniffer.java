import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.nio.ByteBuffer;

public class PacketSniffer {

    private static volatile boolean stopSniffingFlag = false;

    public static void main(String[] args) {
        startSniffing("127.0.0.1", 8000); // 用你想要的IP地址和端口替换
    }

    public static void startSniffing(String ipAddress, int port) {
        stopSniffingFlag = false;

        try (DatagramSocket socket = new DatagramSocket(port, InetAddress.getByName(ipAddress))) {
            socket.setReceiveBufferSize(65535);
            socket.setSoTimeout(1000);

            new Thread(() -> sniffThread(socket)).start();
        } catch (SocketException | java.net.UnknownHostException e) {
            // 处理异常
            e.printStackTrace();
        }
    }

    public static void stopSniffing() {
        stopSniffingFlag = true;
        System.out.println("Packet sniffing stopped.");
    }

    private static void sniffThread(DatagramSocket socket) {
        byte[] buffer = new byte[65535];

        while (!stopSniffingFlag) {
            try {
                DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                socket.receive(packet);

                ByteBuffer byteBuffer = ByteBuffer.wrap(packet.getData());

                // Extract IP header information
                byte verIhl = byteBuffer.get();
                byte tos = byteBuffer.get();
                short totalLen = byteBuffer.getShort();
                short id = byteBuffer.getShort();
                short offset = byteBuffer.getShort();
                byte ttl = byteBuffer.get();
                byte proto = byteBuffer.get();
                short checksum = byteBuffer.getShort();
                int sourceIP = byteBuffer.getInt();
                int destIP = byteBuffer.getInt();

                // Extract additional information based on the protocol (ICMP, UDP, TCP)
                String sourceIpStr = InetAddress.getByAddress(ByteBuffer.allocate(4).putInt(sourceIP).array()).getHostAddress();
                String destIpStr = InetAddress.getByAddress(ByteBuffer.allocate(4).putInt(destIP).array()).getHostAddress();
                String protocolName = getProtocolName(proto);

                // Print packet details
                System.out.println("Source IP Address: " + sourceIpStr + ", Destination IP Address: " + destIpStr + ", Protocol: " + protocolName);

                // Print raw packet data (hex format)
                System.out.print("Raw Packet Content: ");
                for (byte b : packet.getData()) {
                    System.out.print(String.format("%02x ", b));
                }
                System.out.println();

                // Extract additional information based on the protocol
                if (proto == 1) { // ICMP
                    byte type = byteBuffer.get();
                    byte code = byteBuffer.get();
                    short icmpChecksum = byteBuffer.getShort();
                    short unused = byteBuffer.getShort();

                    System.out.println("ICMP Type: " + type + ", Code: " + code);
                } else if (proto == 17) { // UDP
                    short sourcePort = byteBuffer.getShort();
                    short destPort = byteBuffer.getShort();
                    short length = byteBuffer.getShort();
                    short udpChecksum = byteBuffer.getShort();

                    System.out.println("UDP Source Port: " + sourcePort + ", Destination Port: " + destPort);
                } else if (proto == 6) { // TCP
                    short sourcePort = byteBuffer.getShort();
                    short destPort = byteBuffer.getShort();
                    int seqNum = byteBuffer.getInt();
                    int ackNum = byteBuffer.getInt();
                    byte offsetReserved = byteBuffer.get();
                    byte flags = byteBuffer.get();
                    short windowSize = byteBuffer.getShort();
                    short tcpChecksum = byteBuffer.getShort();
                    short urgentPointer = byteBuffer.getShort();

                    System.out.println("TCP Source Port: " + sourcePort + ", Destination Port: " + destPort);
                }

                // Implement the rest of the logic for updating GUI or other actions

            } catch (Exception e) {
                // Handle exceptions
                e.printStackTrace();
            }
        }
    }

    private static String getProtocolName(byte proto) {
        // Get protocol name based on protocol number
        switch (proto) {
            case 1:
                return "ICMP";
            case 6:
                return "TCP";
            case 17:
                return "UDP";
            default:
                return "UnknownProtocol";
        }
    }
}
