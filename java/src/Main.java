import java.io.IOException;
import java.net.ServerSocket;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;


public class Main {
  public static void main(String[] args) {
    int port = 0;
    ServerSocket server = null;
    ExecutorService executorService = Executors.newCachedThreadPool();

    try {
      server = new ServerSocket(port);
    } catch (IOException e) {
      e.printStackTrace();
    }

    try {
      if (server != null) {
        while (true) {
          System.out.print("Waiting for socket connection...");
          executorService.submit(new SocketProcessor(server.accept()));
          System.out.println("   Connected");
        }
      }
    } catch (IOException e) {
      e.printStackTrace();
    } finally {
      executorService.shutdown();
    }
  }
}
