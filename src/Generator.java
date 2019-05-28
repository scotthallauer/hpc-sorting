import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Random;
import java.util.Scanner;

public class Generator {

    public static String FILENAME = "numbers.txt";
    public static void main (String[] args) throws IOException {
        Scanner input = new Scanner(System.in);
        System.out.print("Total numbers to generate: ");
        int n = input.nextInt();
        System.out.print("Minimum value: ");
        int min = input.nextInt();
        System.out.print("Maximum value: ");
        int max = input.nextInt();

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