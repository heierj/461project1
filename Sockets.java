import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

public class Sockets {
    private static final String HOSTNAME = "amlia.cs.washington.edu";
    private static final int PORT = 12235;
    private static final int HEADER_SIZE = 12;
    private static final short STUDENT_NUM = 980;
	
	public static void main(String[] args) {
		
		InetAddress ipAddress = getIpForHost();
		ByteBuffer responseA  = stageA(ipAddress);
		ByteBuffer responseB = stageB(ipAddress, responseA);
		ByteBuffer responseC = stageC(ipAddress, responseB);
		stageD(ipAddress, responseC);
	}
	
	private static InetAddress getIpForHost() {
		try {
	      return InetAddress.getByName(HOSTNAME);
	    } catch ( UnknownHostException e ) {
	      System.out.println("Could not find IP address for: " + HOSTNAME);
	      return null;
	    }
	}

	private static void stageD(InetAddress ipAddress, ByteBuffer responseC) {
		// TODO Auto-generated method stub
		
	}

	private static ByteBuffer stageC(InetAddress ipAddress, ByteBuffer responseB) {
		// TODO Auto-generated method stub
		return null;
	}

	private static ByteBuffer stageB(InetAddress ipAddress, ByteBuffer responseA) {
		
		// Parse packet received in stage A 
		int payloadStart = HEADER_SIZE;
		int num = responseA.getInt(payloadStart);
		int len = responseA.getInt(payloadStart + 4) + 4;
		int udpPort = responseA.getInt(payloadStart + 8);
		int secretA = responseA.getInt(payloadStart + 12);
				
		// Set up the packet for sending
		DatagramSocket socket = createUdpSocket(500);
		ByteBuffer packetData = startPacket(len, secretA);
		
		// Send num packets and wait for ACK each time
		for(int i = 0; i < num; i++) {
			packetData.putInt(12, i);
			byte[] bytes = packetData.array();
			DatagramPacket packet = new DatagramPacket(bytes, bytes.length, ipAddress, udpPort);
			
			// Send the packet
			try {
				socket.send(packet);
			}  catch (IOException e) {
				System.out.println("Couldn't send packet");
				e.printStackTrace();
			}
			
			// Receive the ACK
			byte[] recPacket = new byte[16];
			DatagramPacket receive = new DatagramPacket(recPacket, recPacket.length);
			try {
				socket.receive(receive);
			} catch (SocketTimeoutException e) {
				// There was a timeout... resend the packet
				System.out.println("No ACK received for packet " + i + ", resending...");
				i--;
				continue;
			} catch (IOException e) {
				System.out.println("Error receiving ACK, quitting");
				return null;
			}
			ByteBuffer serverResponse = ByteBuffer.wrap(receive.getData());	
			if(serverResponse.getInt(HEADER_SIZE) != i) {
				System.out.println("Received incorrect ACK, quitting");
				return null;
			}
			
		}
	
		// Receive server response for stage b
		byte[] recPacket = new byte[20];
		DatagramPacket receive = new DatagramPacket(recPacket, recPacket.length);
		try {
			socket.receive(receive);
		} catch (IOException e) {
			System.out.println("Didn't receive server response for stage B");
			return null;
		}
		
		ByteBuffer serverResponse = ByteBuffer.wrap(recPacket);
		System.out.println("SecretB: " + serverResponse.getInt(16));
		socket.close();	
		return serverResponse;
	}

	/**
	 * Peforms stageA of the project
	 * @param ipAddress ipAddress to send packets to
	 * @return server response for stageA
	 */
	private static ByteBuffer stageA(InetAddress ipAddress) {
		// Open UDP socket
		DatagramSocket socket = createUdpSocket(0);
		
		// Create packet to send
		ByteBuffer packetData = startPacket(12, 0);
		packetData.put("hello world".getBytes());
		byte[] bytes = packetData.array();
		DatagramPacket packet = new DatagramPacket(bytes, bytes.length, ipAddress, PORT);
		
		// Send the packet
		try {
			socket.send(packet);
		} catch (IOException e) {
			System.out.println("Couldn't send packet");
			e.printStackTrace();
		}
		
		// Receive the response
		byte[] recPacket = new byte[28];
		DatagramPacket receive = new DatagramPacket(recPacket, recPacket.length);
		try {
			socket.receive(receive);
		} catch (IOException e) {
			System.out.println("Didn't receive server response for part A");
			return null;
		}
		
		// Close socket and return server response
		ByteBuffer serverResponse = ByteBuffer.wrap(receive.getData());
		System.out.println("SecretA: " + serverResponse.getInt(HEADER_SIZE + 12));
		socket.close();		
		return serverResponse;
	}
	
	/**
	 * Creates a new UDP socket and sets a timeout if desired
	 * @param timeoutInMillis the timeout for the socket, if 0 no timeout is set
	 */
	private static DatagramSocket createUdpSocket(int timeoutInMillis) {
		DatagramSocket socket;
		try {
			socket = new DatagramSocket();
		} catch (SocketException e1) {
			System.out.println("Couldn't create socket");
			return null;
		}
		
		if(timeoutInMillis != 0) {
			try {
				socket.setSoTimeout(timeoutInMillis);
			} catch (SocketException e1) {
				System.out.println("Couldn't set timeout");
			}
		}
		return socket;
	}
	
	/**
	 * Creates a new packet that is padded and has the header filled in.
	 * @param payloadLen length of packet payload before padding
	 * @param pSecret secret from previous stage
	 */
	private static ByteBuffer startPacket(int payloadLen, int pSecret) {
		int paddedLen = payloadLen + HEADER_SIZE;
		
		// Make sure packet will be 4-byte aligned 
		if(payloadLen % 4 != 0) {
			paddedLen += 4 - (payloadLen % 4);
		}
		
		// Set the header
		ByteBuffer packetData = ByteBuffer.allocate(paddedLen);
		packetData.putInt(payloadLen);
		packetData.putInt(pSecret);
		packetData.putShort((short) 1);
		packetData.putShort(STUDENT_NUM);
		
		return packetData;
	}
}
