import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

// the Monitor
class BathroomMonitor {
    private int menInside = 0;
    private int womenInside = 0;
    private int menWaiting = 0;
    private int womenWaiting = 0;
    
    // Flips if bathroom is empty and both genders are waiting
    private enum Turn { MEN, WOMEN }
    private Turn turn = Turn.MEN; // Random initial value

    // Helper function
    private String getTime() {
        return new SimpleDateFormat("HH:mm:ss").format(new Date());
    }

    public synchronized void manEnter(int id) {
        menWaiting++;
        // Log for testing
        System.out.println("[" + getTime() + "] Man " + id + " wants to enter.");
        
        try {
            // Wait if:
            // Women are inside.
            // Men are inside and women are waiting
            // Room is empty, but women are waiting and it is Women's turn (flipped when the last man exited)
            while (womenInside > 0 || 
                  (menInside > 0 && womenWaiting > 0) || 
                  (menInside == 0 && womenWaiting > 0 && turn == Turn.WOMEN)) {
                wait();
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        menWaiting--;
        menInside++;
        turn = Turn.MEN; // claims turn for the men
        
        System.out.println("[" + getTime() + "] >>> Man " + id + " ENTERED. (Men Inside: " + menInside + ")");
    }

    public synchronized void manExit(int id) {
        menInside--;
        System.out.println("[" + getTime() + "] <<< Man " + id + " EXITED. (Men Inside: " + menInside + ")");
        
        // If the bathroom is now empty, and women are waiting, flip the turn to Women
        if (menInside == 0) {
            if (womenWaiting > 0) {
                turn = Turn.WOMEN;
            }
            // Notify all waiting threads to check their conditions again
            notifyAll();
        }
    }

    public synchronized void womanEnter(int id) {
        womenWaiting++;
        System.out.println("[" + getTime() + "] Woman " + id + " wants to enter.");
        
        try {
            // Wait if:
            // Men are inside.
            // Women are inside and men are waiting
            // Room is empty, but men are waiting and it is Men's turn (flipped when the last woman exited)
            while (menInside > 0 || 
                  (womenInside > 0 && menWaiting > 0) || 
                  (womenInside == 0 && menWaiting > 0 && turn == Turn.MEN)) {
                wait();
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        womenWaiting--;
        womenInside++;
        turn = Turn.WOMEN;
        
        System.out.println("[" + getTime() + "] >>> Woman " + id + " ENTERED. (Women Inside: " + womenInside + ")");
    }

    public synchronized void womanExit(int id) {
        womenInside--;
        System.out.println("[" + getTime() + "] <<< Woman " + id + " EXITED. (Women Inside: " + womenInside + ")");
        
        // If the bathroom is now empty, and women are waiting, flip the turn to Women
        if (womenInside == 0) {
            if (menWaiting > 0) {
                turn = Turn.MEN;
            }
            notifyAll();
        }
    }
}

// Thread representing a Person
class Person implements Runnable {
    private int id;
    private String gender; // "Man" or "Woman"
    private BathroomMonitor monitor;
    private Random rand;

    public Person(int id, String gender, BathroomMonitor monitor) {
        this.id = id;
        this.gender = gender;
        this.monitor = monitor;
        this.rand = new Random();
    }

    @Override
    public void run() {
        try {
            while (true) {
                // Simulate time working/outside bathroom
                Thread.sleep(rand.nextInt(15000) + 2000);

                if (gender.equals("Man")) {
                    monitor.manEnter(id);
                    // Inside bathroom
                    Thread.sleep(rand.nextInt(5000) + 1000); 
                    monitor.manExit(id);
                } else {
                    monitor.womanEnter(id);
                    // Inside bathroom
                    Thread.sleep(rand.nextInt(5000) + 1000);
                    monitor.womanExit(id);
                }
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }
}

public class UnisexBathroomMonitor {
    public static void main(String[] args) {
        BathroomMonitor monitor = new BathroomMonitor();
        int numMen = 4;
        int numWomen = 4;

        System.out.println("- Starting Simulation -");

        // Create and start Man threads
        for (int i = 1; i <= numMen; i++) {
            new Thread(new Person(i, "Man", monitor)).start();
        }

        // Create and start Woman threads
        for (int i = 1; i <= numWomen; i++) {
            new Thread(new Person(i, "Woman", monitor)).start();
        }
    }
}