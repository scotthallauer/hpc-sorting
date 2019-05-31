import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Random;

public class Generator {
    public static void main (String[] args) throws IOException {
        String filename = args[0];
        int n = Integer.parseInt(args[1]);
        int min = 0;
        int max = 999999;

        BufferedWriter output = new BufferedWriter(new FileWriter(filename));
        output.write(String.valueOf(n));
        Random generator = new Random();
        for (int i = 0; i < n; i++) {
            int r = generator.nextInt((max - min) + 1) + min;
            output.append("\n" + String.valueOf(r));
        }
        output.close();
    }
}