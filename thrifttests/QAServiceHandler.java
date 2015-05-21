import tutorial.QAService;
import java.util.List;
public class QAServiceHandler implements QAService.Iface {
	public void AskQuestion(List<String> question) {
		System.out.println("Java server has finished processing your request...");
	}
}

