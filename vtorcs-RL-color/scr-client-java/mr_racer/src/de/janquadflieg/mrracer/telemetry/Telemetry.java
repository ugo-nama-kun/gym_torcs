/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.telemetry;

import de.janquadflieg.mrracer.Utils;

import java.io.*;
import java.util.*;
import java.util.zip.*;
import java.util.concurrent.ArrayBlockingQueue;

/**
 * A class to encapsulate all the telemetry data belonging to one run of the car
 * or a race.
 * @author Jan Quadflieg
 */
public class Telemetry
        implements Runnable {

    public enum Mode {

        MODE_RECORDING, /*MODE_PLAYBACK,*/ MODE_PAUSED
    }
    /** The current mode. */
    private Mode mode = Mode.MODE_RECORDING;
    /** The current lap, either for recording or for replay. */
    private int currentLap = 0;
    /** The last lap time. */
    private double lastLapTime = Utils.NO_DATA_D;
    /** The best lap time.*/
    private double bestLapTime = Utils.NO_DATA_D;
    /** Thread to asynchrounisly update the registered listeners. */
    private Thread thread;
    /** Queue to buffer log data. */
    private final ArrayBlockingQueue<DataEntry> queue = new ArrayBlockingQueue<DataEntry>(500);
    /** Initial capacity - 50 entries per second * 60 seconds per minute * 2 minutes per lap * 5 laps. */
    private static final int INITIAL_CAPACITY = 50 * 60 * 2 * 5;
    /** List of data sets. */
    private final ArrayList<DataEntry> data = new ArrayList<DataEntry>(INITIAL_CAPACITY);
    /** The list of registered listeners. */
    private final ArrayList<TelemetryListener> listeners = new ArrayList<TelemetryListener>();
    /** An object for synchronization. */
    private final Object syncObject = new Object();
    /** Flag to control the run method. */
    private boolean shutdown = false;
    /** Flag to indicate that all data should be deleted.*/
    private boolean clear = false;
    private int selectedIndex = -1;
    private double approxTrackLength = 1000.0;

    public Telemetry() {
        for(int i=0; i < INITIAL_CAPACITY; ++i){
            DataEntry entry = new DataEntry();
            data.add(entry);
        }
        data.clear();
        thread = new Thread(this);
        thread.setName("Telemetry Thread");
        thread.start();
    }

    public static Telemetry createPrecompile() {
        Telemetry result = new Telemetry();
        result.shutdown();
        try {
            InputStream fin = new Object().getClass().getResourceAsStream("/de/janquadflieg/mrracer/data/precompile.zip");
            ZipInputStream zin = new ZipInputStream(fin);
            ZipEntry entry = zin.getNextEntry();            

            result.load(zin);

            zin.close();
            fin.close();            

            return result;

        } catch (Exception e) {
            return result;
        }
    }

    public static Telemetry createPrecompile2() {
        Telemetry result = new Telemetry();
        result.shutdown();
        try {
            InputStream fin = new Object().getClass().getResourceAsStream("/de/janquadflieg/mrracer/data/precompile2.zip");
            ZipInputStream zin = new ZipInputStream(fin);
            ZipEntry entry = zin.getNextEntry();

            result.load(zin);

            zin.close();
            fin.close();

            return result;

        } catch (Exception e) {
            return result;
        }
    }

    public synchronized int getCurrentLap() {
        return this.currentLap;
    }

    public double getTrackLength(){
        return this.approxTrackLength;
    }

    public synchronized SensorData getSensorData(int i) {
        return data.get(i).s;
    }

    public synchronized Action getAction(int i) {
        return data.get(i).a;
    }

    public synchronized String getLog(int i) {
        return data.get(i).l;
    }

    public double getBestLapTime() {
        return this.bestLapTime;
    }

    public double getLastLapTime() {
        return this.lastLapTime;
    }

    public int size() {
        synchronized (data) {
            return data.size();
        }
    }

    public void clear() {
        synchronized (syncObject) {
            clear = true;
            syncObject.notifyAll();
        }
    }

    /**
     * Helper method to notify the registered listeners.
     *
     * @param e The new data point.
     */
    private void notifyListeners(DataEntry e) {
        for (TelemetryListener l : listeners) {
            l.newData(e.s, e.a, e.l, mode);
        }
    }

    private void notifyListenersCleared() {
        for (TelemetryListener l : listeners) {
            l.cleared();
        }
    }

    private void notifyListenersNewLap() {
        for (TelemetryListener l : listeners) {
            l.newLap();
        }
    }

    private void addData(DataEntry entry) {
        DataEntry lastEntry = null;

        if (data.size() > 0) {
            lastEntry = data.get(data.size() - 1);
        }
        data.add(entry);

        if (lastEntry != null) {
            // check, if the packets are in the wrong order
            if (entry.s.getTimeStamp() < lastEntry.s.getTimeStamp()) {
                System.out.println("WARNING: Packets arrived in the wrong order!");
            }


                        // check, if a new lap has begun
            if(entry.s.getDistanceFromStartLine() < lastEntry.s.getDistanceFromStartLine() &&
                    entry.s.getDistanceRaced() > lastEntry.s.getDistanceRaced()) {
                //long diff = entry.s.getTimeStamp() - lastEntry.s.getTimeStamp();
                this.approxTrackLength = lastEntry.s.getDistanceFromStartLine() +
                        ((entry.s.getDistanceRaced() - lastEntry.s.getDistanceRaced()) - entry.s.getDistanceFromStartLine());
                ++currentLap;
                if (currentLap > 1) {
                    lastLapTime = lastEntry.s.getCurrentLapTime();
                    if (bestLapTime == Utils.NO_DATA_D || lastLapTime <= bestLapTime) {
                        bestLapTime = lastLapTime;
                    }
                }
                notifyListenersNewLap();
            }
        }

        notifyListeners(entry);
    }

    public synchronized void log(SensorData data, Action a, String info) {
        if (mode != Mode.MODE_RECORDING) {
            return;
        }

        //long start = System.currentTimeMillis();
        DataEntry entry = new DataEntry();
        entry.s.setData(data);
        entry.a.setData(a);
        entry.l = info;
        synchronized (syncObject) {
            queue.add(entry);
            syncObject.notifyAll();
            if (queue.size() > 10) {
                System.err.println("Warning, queue size: " + queue.size());
            }
        }
    //System.out.println((System.currentTimeMillis()-start));
    }

    public synchronized void addListener(TelemetryListener l) {
        listeners.add(l);
    }

    public synchronized void removeListener(TelemetryListener l) {
        listeners.remove(l);
    }

    public synchronized void setMode(Mode newMode) {
    }

    public synchronized void selectEntry(int i){
        if(mode == Mode.MODE_PAUSED && i >= 0 && i < size()){
            selectedIndex = i;
            syncObject.notifyAll();
        }        
    }

    /**
     * Run method used by this object's thread to asynchronisly update any
     * registered listener.
     */
    @Override
    public void run() {
        while (!shutdown) {
            synchronized (syncObject) {
                try {
                    syncObject.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            synchronized (syncObject) {
                if (clear) {
                    data.clear();
                    queue.clear();
                    bestLapTime = Utils.NO_DATA_D;
                    lastLapTime = Utils.NO_DATA_D;
                    currentLap = 0;
                    notifyListenersCleared();
                    clear = false;

                } else if (mode == Mode.MODE_RECORDING && !queue.isEmpty()) {
                    while (!queue.isEmpty()) {
                        addData(queue.poll());
                    }
                //if(queue.size()>1){
                //    System.out.println("gepollt aber queue nicht leer! "+queue.size());
                //}

//                        long before = System.currentTimeMillis();
//                        long diff = System.currentTimeMillis() - before;
//                        System.out.println("Listeners notified, that took "+diff+"ms");
//                        System.out.println("Entries: "+data.size());
                } else if(mode == Mode.MODE_PAUSED && selectedIndex != -1){
                    notifyListeners(data.get(selectedIndex));
                    selectedIndex = -1;
                }
            }
        }
    }

    public synchronized void save(File f) {
        try {
            boolean zip = f.getName().endsWith(".zip");

            FileOutputStream fo = new FileOutputStream(f, false);
            OutputStreamWriter w;
            ZipOutputStream zo = null;

            if (zip) {
                zo = new ZipOutputStream(fo);
                w = new OutputStreamWriter(zo, "UTF-8");

                ZipEntry entry = new ZipEntry(f.getName().substring(0, f.getName().lastIndexOf(".zip")));

                zo.putNextEntry(entry);

                zo.setLevel(9);

            } else {
                w = new OutputStreamWriter(fo, "UTF-8");
            }

            this.writeCompleteData(w);
            //this.writeTrainingsData(w);

            w.flush();

            if (zip) {
                zo.closeEntry();
            }

            w.close();
            fo.close();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Writes the complete data including a human readable header.
     *
     * @param w Writer
     * @throws java.lang.Exception
     */
    private void writeCompleteData(OutputStreamWriter w)
            throws Exception {
        final String DELIMITER = " ";
        //final String DELIMITER = "\t";
        //final String DELIMITER = ";";

        final String META_DELIMITER = "§";

        SensorData.writeHeader(w, DELIMITER);
        w.write(META_DELIMITER);
        Action.writeHeader(w, DELIMITER);
        w.write(META_DELIMITER);
        w.write("Log");

        w.write("\n");

        for (DataEntry e : data) {
            e.s.write(w, DELIMITER);
            w.write(META_DELIMITER);
            e.a.write(w, DELIMITER);
            w.write(META_DELIMITER);
            w.write(e.l);

            w.write("\n");
        }
    }

    public void load(String s) {
        boolean zip = s.endsWith(".zip");

        try{
            InputStream fin = new FileInputStream(s);
            InputStream in;

            if(zip){
                ZipInputStream zin = new ZipInputStream(fin);
                ZipEntry entry = zin.getNextEntry();
                in = zin;
                
            } else {
                in = fin;
            }

            load(in);

            in.close();
            fin.close();
        } catch(Exception e){
            new RuntimeException(e);
        }        
    }

    public void load(InputStream in) {
        try{
            load(new BufferedReader(new InputStreamReader(in, "UTF-8")));
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public void load(BufferedReader reader) {
        data.clear();
        mode = Mode.MODE_PAUSED;

        try {
            final String META_DELIMITER = "§";

            String line = reader.readLine();

            // ignore the header
            if(line != null && line.startsWith("RaceD")){
                line = reader.readLine();
            }

            ModifiableSensorData last = null;            

            while (line != null) {
                StringTokenizer tok = new StringTokenizer(line, META_DELIMITER);
                DataEntry entry = new DataEntry();

                if (tok.hasMoreTokens()) {
                    ModifiableSensorData m = new ModifiableSensorData(tok.nextToken(), " ");
                    if(last != null && last.getCurrentLapTime() > m.getCurrentLapTime()){
                        m.setLastLapTime(last.getCurrentLapTime());
                        //System.out.println("New lap @"+m.getDistanceRaced());
                        //System.out.println("Lap time: "+m.getCurrentLapTimeS());
                        //System.out.println("Last lap: "+m.getLastLapTime());
                    }

                    entry.s.setData(m);

                    last = m;
                }

                if (tok.hasMoreTokens()) {
                    entry.a.setData(new Action(tok.nextToken(), " "));
                }

                if (tok.hasMoreTokens()) {
                    entry.l = tok.nextToken();
                }

                this.data.add(entry);

                line = reader.readLine();
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public synchronized void shutdown() {
        this.shutdown = true;
        synchronized (syncObject) {
            syncObject.notifyAll();
        }
    }

    private static class DataEntry {

        public ModifiableSensorData s = new ModifiableSensorData();
        public ModifiableAction a = new ModifiableAction();
        public String l = "";
    }

    public static void main(String[] args){
        Telemetry t = new Telemetry();

        t.load("/home/quad/test.zip");

        for(int i=0; i < t.size(); ++i){
            System.out.println(t.getLog(i));
        }


        t.shutdown();
    }
}