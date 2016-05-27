/**
 * 
 */
package scr;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;

/**
 * @author Daniele Loiacono
 * 
 */
public class SocketHandler {

	private InetAddress address;
	private int port;
	private DatagramSocket socket;
	private boolean verbose;

	public SocketHandler(String host, int port, boolean verbose) {

		// set remote address
		try {
			this.address = InetAddress.getByName(host);
		} catch (UnknownHostException e) {
			e.printStackTrace();
		}
		this.port = port;
		// init the socket
		try {
			socket = new DatagramSocket();
		} catch (SocketException e) {
			e.printStackTrace();
		}
		this.verbose = verbose;
	}

	public void send(String msg) {

		if (verbose)
			System.out.println("Sending: " + msg);
		try {
			byte[] buffer = msg.getBytes();
			socket
					.send(new DatagramPacket(buffer, buffer.length, address,
							port));
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public String receive() {
		try {
			byte[] buffer = new byte[1024];
			DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
			socket.receive(packet);
			String received = new String(packet.getData(), 0, packet
					.getLength());
			if (verbose)
				System.out.println("Received: " + received);
			return received;
		} catch(SocketTimeoutException se){			
			if (verbose)
				System.out.println("Socket Timeout!");
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
	}

	public String receive(int timeout) {
		try {
			socket.setSoTimeout(timeout);
			String received = receive();
			socket.setSoTimeout(0);
			return received;
		} catch (SocketException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}		
		return null;
	}
	
	public void close() {
		socket.close();
	}
	
	
	

}
