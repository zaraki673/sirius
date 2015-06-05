// Thrift java libraries 
import org.apache.thrift.server.TServer;
import org.apache.thrift.server.TServer.Args;
import org.apache.thrift.server.TSimpleServer;
import org.apache.thrift.server.TThreadPoolServer;
import org.apache.thrift.transport.TSSLTransportFactory;
import org.apache.thrift.transport.TServerSocket;
import org.apache.thrift.transport.TServerTransport;
import org.apache.thrift.transport.TSSLTransportFactory.TSSLTransportParameters;

// Thrift client-side code for registering w/ command center
import org.apache.thrift.TException;
import org.apache.thrift.transport.TSSLTransportFactory;
import org.apache.thrift.transport.TTransport;
import org.apache.thrift.transport.TSocket;
import org.apache.thrift.transport.TSSLTransportFactory.TSSLTransportParameters;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.protocol.TProtocol;

// Generated code
import qastubs.QAService;
import cmdcenterstubs.CommandCenter;

// Java libraries
import java.util.HashMap;

// Start the thrift qa server
public class QADaemon {
	public static QAServiceHandler handler;
	public static QAService.Processor<QAServiceHandler> processor;
	public static void main(String [] args) {
		try {
			// the handler implements the generated java interface
			// that was originally specified in the thrift file.
			// When it's called, an Open Ephyra object is created.
			handler = new QAServiceHandler();
			processor = new QAService.Processor<QAServiceHandler>(handler);
			Runnable simple = new Runnable() {
				// This is the code that the thread will run
				public void run() {
					simple(processor);
				}
			};
			// Let system schedule the thread
			new Thread(simple).start();
		} catch (Exception x) {
			x.printStackTrace();
		}
	}

	public static void simple(QAService.Processor processor) {
		try {
			// Register this server with the command center
			int port = 9091;
			int cmdcenterport = 8081;
			TTransport transport = new TSocket("localhost", cmdcenterport);
			transport.open();
			TProtocol protocol = new TBinaryProtocol(transport);
			CommandCenter.Client client = new CommandCenter.Client(protocol);
			System.out.println("Registering question-answer server with command center...");

			client.registerService("localhost", port, 1);
			transport.close();

			// Start the question-answer server
			TServerTransport serverTransport = new TServerSocket(port);
			TServer server = new TSimpleServer(new Args(serverTransport).processor(processor));
			//TServer server = new TThreadPoolServer(new TThreadPoolServer.Args(serverTransport).processor(processor));
			
			System.out.println("Starting the question-answer server at port " + port + "...");
			server.serve();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
