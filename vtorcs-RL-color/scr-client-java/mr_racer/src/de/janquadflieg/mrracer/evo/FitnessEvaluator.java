/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.evo;

import de.janquadflieg.mrracer.controller.CrashController;
import de.janquadflieg.mrracer.controller.Evaluator;
import de.janquadflieg.mrracer.network.*;

import java.io.*;
import java.util.*;

/**
 *
 * @author Jan Quadflieg
 */
public class FitnessEvaluator
        implements Runnable, ConnectionListener {

    private final Object mutex = new Object();
    private Thread thread;
    private boolean finished = false;
    private Evaluator controller;
    private String host;
    private int port;
    private EvoResults result = new EvoResults();

    public FitnessEvaluator(String host, int port, Evaluator c) {
        this(host, port, c, Evaluator.NO_MAXIMUM, true);
    }

    public FitnessEvaluator(String host, int port, Evaluator c, int maxLaps) {
        this(host, port, c, maxLaps, true);
    }

    public FitnessEvaluator(String host, int port, Evaluator c, int maxLaps, boolean checkDamage) {
        controller = c;
        controller.setMaxLaps(maxLaps);
        this.host = host;
        this.port = port;

        thread = new Thread(this);
        thread.start();
    }

    public void run() {
        try {
            controller.resetFull();

            UDPConnection connection = new UDPConnection(host, port, controller);
            connection.addConnectionListener(this);

            synchronized (mutex) {
                mutex.wait();
            }
            controller.shutdown();

            result.damage = controller.getDamage();
            result.distance = controller.getDistanceRaced();
            result.offTrack = controller.getOffTrackCtr();

        } catch (Exception e) {
            e.printStackTrace();
            result.damage = 10000;
            result.distance = 0;
            result.offTrack = 10000;
        }

        finished = true;
    }

    public boolean aborted() {
        return controller.aborted();
    }

    public boolean finished() {
        return finished;
    }

    public EvoResults getResult() {
        return result;
    }

    public double getFastestLap() {
        return controller.getFastestLap();
    }

    public double getOverallTime() {
        return controller.getOverallTime();
    }

    public boolean maxDamageReached(){
        return controller.maxDamageReached();
    }

    public void newStatistics(ConnectionStatistics data) {
    }

    public void stopped(boolean requested) {
        synchronized (mutex) {
            mutex.notifyAll();
        }
    }

    public static void main(String[] args) {
        System.setProperty("EAMode", "");

        int port = 3001;
        String host = "localhost";
        int maxSteps = 0;
        int maxLaps = 3;
        String trackName = "unknown";
        String resultFile = "results";
        boolean recovery = false;

        for (int i = 0; i < args.length; i++) {
            StringTokenizer st = new StringTokenizer(args[i], ":");
            String entity = st.nextToken();
            String value = "";
            if(st.hasMoreTokens()){
                value = st.nextToken();
            }
            if (entity.equals("recovery")) {
                recovery = true;
            }
            if (entity.equals("port")) {
                port = Integer.parseInt(value);
            }
            if (entity.equals("host")) {
                host = value;
            }
            if (entity.equals("trackName")) {
                trackName = value;
            }
            if (entity.equals("results")) {
                resultFile = value;
            }
            if (entity.equals("maxSteps")) {
                maxSteps = Integer.parseInt(value);
                if (maxSteps < 0) {
                    System.out.println(entity + ":" + value
                            + " is not a valid option");
                    System.exit(0);
                }
            }
            if (entity.equals("maxLaps")) {
                maxLaps = Integer.parseInt(value);
                if (maxLaps < 1) {
                    System.out.println(entity + ":" + value
                            + " is not a valid option");
                    System.exit(0);
                }
            }
        }

        de.janquadflieg.mrracer.controller.MrRacer2011 controller =
                new de.janquadflieg.mrracer.controller.MrRacer2011();
        controller.setStage(champ2011client.Controller.Stage.QUALIFYING);
        controller.setTrackName(trackName);

        FitnessEvaluator fe;

        if (recovery) {
            CrashController crashController = new CrashController(controller);
            crashController.setStage(champ2011client.Controller.Stage.QUALIFYING);
            crashController.setTrackName(trackName);

            fe = new FitnessEvaluator(host, port,
                    new Evaluator(crashController, maxSteps, true), maxLaps);

        } else {
            fe = new FitnessEvaluator(host, port,
                    new Evaluator(controller, maxSteps, true), maxLaps);
        }

        while (!fe.finished()) {
            try {
                Thread.sleep(1000);
            } catch (Exception e) {
            }
        }

        EvoResults r = fe.getResult();

        try {
            FileWriter writer = new FileWriter(resultFile);

            writer.write("distance=" + String.valueOf(r.distance) + "\n");
            writer.write("damage=" + String.valueOf(r.damage) + "\n");
            writer.write("offtrack=" + r.offTrack + "\n");
            writer.write("time=" + String.valueOf(fe.getOverallTime()) + "\n");
            writer.write("fastestTime=" + String.valueOf(fe.getFastestLap()) + "\n");
            writer.write("aborted=" + String.valueOf(fe.aborted()));

            writer.flush();
            writer.close();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
