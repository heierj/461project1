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
	
	public static void main(String[] args) {
		ByteBuffer responseA  = stageA();
		ByteBuffer responseB = stageB(responseA);
		ByteBuffer responseC = stageC(responseB);
		stageD(responseC);
	}
	
	private static void stageD(ByteBuffer responseC) {
		// TODO Auto-generated method stub
		
	}

	private static ByteBuffer stageC(ByteBuffer responseB) {
		// TODO Auto-generated method stub
		return null;
	}

	private static ByteBuffer stageB(ByteBuffer responseA) {
		int payloadStart = HEADER_SIZE;
		int num = responseA.getInt(payloadStart);
		int len = responseA.getInt(payloadStart + 4) + 4;
		int udpPort = responseA.getInt(payloadStart + 8);
		int secretA = responseA.getInt(payloadStart + 12);
		System.out.println("SecretA: " + secretA);
		
		System.out.println(num);
		
		InetAddress ipaddress;
		try {
		      ipaddress = InetAddress.getByName(HOSTNAME);
		    } catch ( UnknownHostException e ) {
		      System.out.println("Could not find IP address for: " + HOSTNAME);
		      return null;
		    }
		DatagramSocket socket;
		try {
			socket = new DatagramSocket();
		} catch (SocketException e1) {
			System.out.println("Couldn't create socket");
			return null;
		}
		
		try {
			socket.setSoTimeout(500);
		} catch (SocketException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
			System.out.println("Couldn't set timeout");
		}

		ByteBuffer packetData = startPacket(len, secretA);
		
		for(int i = 0; i < num; i++) {
			packetData.putInt(12, i);
			byte[] bytes = packetData.array();
			DatagramPacket packet = new DatagramPacket(bytes, bytes.length, ipaddress, udpPort);
			try {
				socket.send(packet);
			}  catch (IOException e) {
				System.out.println("Couldn't send packet");
				e.printStackTrace();
			}
			byte[] recPacket = new byte[16];
			DatagramPacket receive = new DatagramPacket(recPacket, recPacket.length);
			try {
				socket.receive(receive);
			} catch (SocketTimeoutException e) {
				System.out.println("No ACK received for packet " + i + ", resending...");
				i--;
				continue;
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				System.out.println("Didn't receive packet");
			}
			ByteBuffer serverResponse = ByteBuffer.wrap(receive.getData());
			System.out.println("Server response received for packet " + serverResponse.getInt(12));
			
			
		}
	
		byte[] recPacket = new byte[20];
		DatagramPacket receive = new DatagramPacket(recPacket, recPacket.length);
		try {
			socket.receive(receive);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			System.out.println("Didn't receive packet");
		}
		ByteBuffer serverResponse = ByteBuffer.wrap(recPacket);
		System.out.println("Tcp port " + serverResponse.getInt(12));
		System.out.println("SecretB " + serverResponse.getInt(16));
		socket.close();
		
		return serverResponse;
		
	}

	private static ByteBuffer stageA() {
		InetAddress ipaddress;
		try {
		      ipaddress = InetAddress.getByName(HOSTNAME);
		    } catch ( UnknownHostException e ) {
		      System.out.println("Could not find IP address for: " + HOSTNAME);
		      return null;
		    }
		DatagramSocket socket;
		try {
			socket = new DatagramSocket();
		} catch (SocketException e1) {
			System.out.println("Couldn't create socket");
			return null;
		}

		ByteBuffer packetData = startPacket(12, 0);
		packetData.put("hello world".getBytes());
		
		byte[] bytes = packetData.array();
		DatagramPacket packet = new DatagramPacket(bytes, bytes.length, ipaddress, PORT);
		try {
			socket.send(packet);
		} catch (IOException e) {
			System.out.println("Couldn't send packet");
			e.printStackTrace();
		}
		byte[] recPacket = new byte[28];
		DatagramPacket receive = new DatagramPacket(recPacket, recPacket.length);
		try {
			socket.receive(receive);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			System.out.println("Didn't receive packet");
		}
		ByteBuffer serverResponse = ByteBuffer.wrap(receive.getData());
		socket.close();
		
		return serverResponse;
	}
	
	private static ByteBuffer startPacket(int payloadLen, int pSecret) {
		int paddedLen = payloadLen + HEADER_SIZE;
		
		// Make sure packet will be 4-byte aligned 
		if(payloadLen % 4 != 0) {
			paddedLen += 4 - (payloadLen % 4);
		}
		
		ByteBuffer packetData = ByteBuffer.allocate(paddedLen);
		packetData.putInt(payloadLen);
		packetData.putInt(pSecret);
		packetData.putShort((short) 1);
		packetData.putShort((short) 980);
		
		return packetData;
				
	}
	
}
