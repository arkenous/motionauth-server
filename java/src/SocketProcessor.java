import java.io.*;
import java.net.Socket;

public class SocketProcessor implements Runnable {
  static {
    System.loadLibrary("authserver");
  }

  public native String[] learn(double[][] x);
  public native double out(String[] neuronParams, double[] x);

  private Socket socket = null;
  private DataOutputStream outputStream = null;
  private BufferedReader bufferedReader = null;
  private DataInputStream inputStream = null;
  private String line = "";

  SocketProcessor(Socket socket) {
    this.socket = socket;
    try {
      System.out.print("Initializing socket stream...");
      this.outputStream = new DataOutputStream(socket.getOutputStream());
      this.bufferedReader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
      this.inputStream = new DataInputStream(socket.getInputStream());
      System.out.println("   Finished");
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  @Override
  public void run() {
    if (outputStream == null || bufferedReader == null) {
      System.err.println("outputStream or/and bufferedReader == null");
      return;
    }

    int mode = 0;
    String username = "";
    int input_time = 0;
    int dimen_data = 0;

    try {
      //TODO Socket切断などのエラー処理を整える
      //ユーザ名や動作モード，モーションデータをSocket経由で取得する

      // Receive mode value
      System.out.print("Waiting for receiving mode value...");
      mode = inputStream.readInt();
      outputStream.writeBoolean(true);
      outputStream.flush();
      System.out.println("   Received: "+mode);

      // Receive user name
      System.out.print("Waiting for receiving user name...");
      if ((line = bufferedReader.readLine()) != null) username = line;
      outputStream.writeBoolean(true);
      outputStream.flush();
      System.out.println("   Received: "+username);

      // Receive number of input time
      System.out.print("Waiting for receiving number of input time...");
      input_time = inputStream.readInt();
      outputStream.writeBoolean(true);
      outputStream.flush();
      System.out.println("   Received: "+input_time);

      // Receive number of data dimension
      System.out.print("Waiting for receiving number of data dimension...");
      dimen_data = inputStream.readInt();
      outputStream.writeBoolean(true);
      outputStream.flush();
      System.out.println("   Received: "+dimen_data);

      // Receive data
      double[][] x = new double[input_time][dimen_data];
      System.out.print("Waiting for receiving data...");
      for (int time = 0, t_size = x.length; time < t_size; ++time) {
        for (int dimen = 0, d_size = x[time].length; dimen < d_size; ++dimen) {
          x[time][dimen] = inputStream.readDouble();
        }
      }
      outputStream.writeBoolean(true);
      outputStream.flush();
      System.out.println("   Received");

      //JniMainにデータを渡して処理させる
      if (mode == 0) {
        System.out.println("Registration mode");
        // Registration mode
        String[] learnResult = learn(x);

        //処理結果を返す
        //Send neuron size (learnResult size)
        System.out.println("learnResult.length: "+learnResult.length);
        outputStream.writeInt(learnResult.length);
        outputStream.flush();

        // Send learnResult
        for (int neuron = 0, size = learnResult.length; neuron < size; ++neuron) {
          outputStream.writeBytes(learnResult[neuron]);
          outputStream.flush();
        }
      } else if (mode == 1) {
        // Authentication mode
        System.out.println("Authentication mode");
        // Receive SdA parameter length
        System.out.print("Waiting for receiving parameter_length...");
        int parameter_length = inputStream.readInt();
        outputStream.writeBoolean(true);
        outputStream.flush();
        System.out.println("   Received: "+parameter_length);

        // Receive SdA parameter
        String[] SdA_parameter = new String[parameter_length];
        System.out.print("Waiting for receiving SdA parameter...");
        for (int i = 0; i < parameter_length; ++i) {
          if ((line = bufferedReader.readLine()) != null) SdA_parameter[i] = line;
        }
        outputStream.writeBoolean(true);
        outputStream.flush();
        System.out.println("   Received");

        double SdA_result = out(SdA_parameter, x[0]);
        System.out.println("SdA result: "+SdA_result);
        outputStream.writeDouble(SdA_result);
        outputStream.flush();
      }

      outputStream.close();
      bufferedReader.close();
      socket.close();
    } catch (IOException e) {
      e.printStackTrace();
    }
  }
}
