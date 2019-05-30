import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Random;

public class Generator {

    public static String FILENAME = "numbers.txt";
    public static void main (String[] args) throws IOException {
        int n = Integer.parseInt(args[0]);
        int min = Integer.parseInt(args[1]);
        int max = Integer.parseInt(args[2]);

        BufferedWriter output = new BufferedWriter(new FileWriter(FILENAME));
        output.write(String.valueOf(n));
        Random generator = new Random();
        for (int i = 0; i < n; i++) {
            int r = generator.nextInt((max - min) + 1) + min;
            output.append("\n" + String.valueOf(r));
        }
        output.close();
    }
}