/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.evo;

import de.janquadflieg.mrracer.PropertiesCreator;
import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.controller.*;
import champ2011client.Controller;

import java.io.*;
import java.util.*;

/**
 *
 * @author nostromo
 */
public class DriveTool2011 {

    public static final String[] TRACKS = {"Alpine-noisy",
        "Brondehach-noisy", "C-SpeedWay-noisy",
        "CG-Speedway-Nr1-noisy", "Street-1-noisy",
        "Wheel2-noisy"};
    public static final int[] PORTS = {3001, 3002, 3003, 3004, 3005, 3006};
    public static final int NUM = 60;
    public static final String HOST = "127.0.0.1";
    public static final int MAX_TICKS = 27000;
    public static final int MAX_LAPS = 3;

    private static void evaluate(Individual2011 ind)
            throws Exception {
        System.out.println("Evaluating indID " + ind.indID + " from " + ind.source);
        System.out.print("Needs to be evaluated on track ");

        HashMap<String, FitnessEvaluator> evals = new HashMap<String, FitnessEvaluator>();

        String oldTrack = ind.fitness.keySet().iterator().next();
        ind.fitness.put("Old", ind.fitness.get(oldTrack));
        ind.fitness.remove(oldTrack);

        for (int i = 0; i < TRACKS.length; ++i) {
            if (!ind.fitness.containsKey(TRACKS[i])) {
                System.out.print(TRACKS[i] + ", ");

                MrRacer2011 c = new MrRacer2011();
                c.setParameters(ind.properties);
                c.setStage(Controller.Stage.QUALIFYING);
                c.setTrackName(TRACKS[i]);

                FitnessEvaluator fe = new FitnessEvaluator(HOST, PORTS[i],
                        new Evaluator(c, MAX_TICKS, true), MAX_LAPS);
                evals.put(TRACKS[i], fe);
            }
        }
        System.out.println("");

        boolean allFinished = false;
        Set<String> keys = evals.keySet();

        while (!allFinished) {
            try {
                Thread.sleep(1000);
            } catch (Exception e) {
            }
            allFinished = true;
            Iterator<String> it = evals.keySet().iterator();
            while (it.hasNext()) {
                String key = it.next();
                allFinished &= evals.get(key).finished();
            }
        }

        Iterator<String> it = evals.keySet().iterator();
        while (it.hasNext()) {
            String key = it.next();
            ind.fitness.put(key, evals.get(key).getOverallTime());
        }

        for(int i=0; i < TRACKS.length; ++i){        
            String key = TRACKS[i];
            /*if(!ind.fitness.containsKey(key)){
                ind.fitness.put(key, Math.random()*420);
            }*/
            System.out.println(key+" "+Utils.dTS(ind.fitness.get(key)));
        }

        System.out.println("");
    }

    public static void main(String[] args) {
        try {
            System.out.println("DriveTool2011");

            BufferedWriter log = new BufferedWriter(new FileWriter(".\\log.txt"));
            // write header
            log.write("indID ");
            for(int i=0; i < TRACKS.length; ++i){
                log.write(TRACKS[i]+" ");
            }
            log.write("Old ");
            log.write("Source\n");
            log.flush();

            HashMap<String, String> data = new HashMap<String, String>();
            data.put("Alpine-noisy", "F:\\Quad\\Experiments\\CIG-2011\\EA-Runs\\TestRun5-Alpine-Noisy\\run02\\Torcs-Noisy_0001inds.log");
            data.put("Wheel2-noisy", "F:\\Quad\\Experiments\\CIG-2011\\EA-Runs\\TestRun5-Wheel2-Noisy\\run01\\Torcs-Noisy_0001inds.log");

            ArrayList<Individual2011> list = new ArrayList<Individual2011>();

            Iterator<String> it = data.keySet().iterator();
            while (it.hasNext()) {
                String track = it.next();
                String file = data.get(track);
                System.out.println("Loading individuals for track " + track + " from file " + file);
                ArrayList<Individual2011> inds = PropertiesCreator.getAll(file, track,
                        new de.janquadflieg.mrracer.evo.tools.EvostarParser());
                System.out.println(inds.size());
                Collections.sort(inds);
                for (int i = 0; i < NUM; ++i) {
                    Individual2011 ind = inds.get(i);
                    System.out.println("Adding indID " + ind.indID + ", fitness=" + inds.get(i).fitness.get(track) + " to list");
                    list.add(ind);
                }
            }

            for (int i = 0; i < list.size(); ++i) {
                Individual2011 ind = list.get(i);
                evaluate(ind);
                log.write(ind.indID+" ");
                for(int k=0; k < TRACKS.length; ++k){
                    log.write(ind.fitness.get(TRACKS[k])+" ");
                }
                log.write(ind.fitness.get("Old")+" ");
                log.write(ind.source+"\n");
                log.flush();                
            }

            System.out.println("");
            // header
            System.out.print("indID ");
            for(int i=0; i < TRACKS.length; ++i){
                System.out.print(TRACKS[i]+" ");
            }
            System.out.print("Old");
            System.out.println("Source");

            for (int i = 0; i < list.size(); ++i) {
                Individual2011 ind = list.get(i);
                System.out.print(ind.indID+" ");
                for(int k=0; k < TRACKS.length; ++k){
                    System.out.print(Utils.dTS(ind.fitness.get(TRACKS[k]))+" ");
                }
                System.out.print(ind.fitness.get("Old")+" ");
                System.out.println(ind.source);
            }

            log.flush();
            log.close();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
