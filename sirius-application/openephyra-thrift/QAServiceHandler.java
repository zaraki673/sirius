// This handler implements the services provided to the client.

// Open Ephyra packages
import info.ephyra.OpenEphyra;
import info.ephyra.search.Result;
import info.ephyra.io.MsgPrinter;

// Interface definition
import qastubs.QAService;
//import java.util.List;

public class QAServiceHandler implements QAService.Iface {
	public String askFactoidThrift(String question)
	{
		//String dir = "~/sirius/sirius-application/question-answer/";
		String dir = "";
		// enable msg printing to screen (logging not enabled)
		MsgPrinter.enableStatusMsgs(true);
		MsgPrinter.enableErrorMsgs(true);
		MsgPrinter.printStatusMsg("Arg:" + question);

		OpenEphyra oe  = new OpenEphyra(dir);
		Result result = oe.askFactoid(question);
		//String answer = result.getAnswer();
		String answer = "42";
		System.out.println("Java handler says: your answer is " + answer);
		return answer;
	}
}

