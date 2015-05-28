// Thrift java libraries 
import org.apache.thrift.server.TServer;
import org.apache.thrift.server.TServer.Args;
import org.apache.thrift.server.TSimpleServer;
import org.apache.thrift.server.TThreadPoolServer;
import org.apache.thrift.transport.TSSLTransportFactory;
import org.apache.thrift.transport.TServerSocket;
import org.apache.thrift.transport.TServerTransport;
import org.apache.thrift.transport.TSSLTransportFactory.TSSLTransportParameters;

// Generated code
import qastubs.QAService;

// Java libraries
import java.util.HashMap;

// Start the thrift qa server
public class QADaemon {
	public static QAServiceHandler handler;
	public static QAService.Processor<QAServiceHandler> processor;
	public static void main(String [] args) {
		try {
			// the handler implements the generated java interface
			// that was originally specified in the thrift file
			handler = new QAServiceHandler();
			processor = new QAService.Processor<QAServiceHandler>(handler);
			Runnable simple = new Runnable() {
				public void run() {
					simple(processor);
				}
			};
			new Thread(simple).start();
		} catch (Exception x) {
			x.printStackTrace();
		}
	}

	public static void simple(QAService.Processor processor) {
		try {
			TServerTransport serverTransport = new TServerSocket(9090);
			TServer server = new TSimpleServer(new Args(serverTransport).processor(processor));
			System.out.println("Starting the simple server...");
			server.serve();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
