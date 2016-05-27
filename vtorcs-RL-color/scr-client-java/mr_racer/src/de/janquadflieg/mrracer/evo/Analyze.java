/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.evo;

import de.janquadflieg.mrracer.Utils;

import java.io.*;
import java.util.*;

/**
 *
 * @author quad
 */
public class Analyze {

    public static final String FILE = "F:\\Quad\\Experiments\\CIG-2011\\EA-Runs\\Pareto-Run\\log.txt";
    public static final String[] TRACKS = {"Alpine-noisy", "Brondehach-noisy",
        "C-SpeedWay-noisy", "CG-Speedway-Nr1-noisy", "Street-1-noisy", "Wheel2-noisy", "Old"};
    public static final String[] TRACKS_WO_OLD = {"Alpine-noisy", "Brondehach-noisy",
        "C-SpeedWay-noisy", "CG-Speedway-Nr1-noisy", "Street-1-noisy", "Wheel2-noisy"};

    /**
     * Returns true, if i1 dominates i2
     * @param i1
     * @param i2
     * @return
     */
    public static boolean dominatesTime(Individual2011 i1, Individual2011 i2) {
        int numLessEqual = 0;
        int numLess = 0;

        for (int i = 0; i < TRACKS_WO_OLD.length; ++i) {
            double f1 = i1.fitness.get(TRACKS_WO_OLD[i]);
            double f2 = i2.fitness.get(TRACKS_WO_OLD[i]);

            if (f1 <= f2) {
                ++numLessEqual;
            }
            if (f1 < f2) {
                ++numLess;
            }
        }

        return (numLessEqual == TRACKS_WO_OLD.length && numLess > 0);
    }

    /**
     * Returns true, if i1 dominates i2
     * @param i1
     * @param i2
     * @return
     */
    public static boolean dominatesRank(Individual2011 i1, Individual2011 i2) {
        int numLessEqual = 0;
        int numLess = 0;

        for (int i = 0; i < TRACKS_WO_OLD.length; ++i) {
            double f1 = i1.ranks.get(TRACKS_WO_OLD[i]);
            double f2 = i2.ranks.get(TRACKS_WO_OLD[i]);

            if (f1 <= f2) {
                ++numLessEqual;
            }
            if (f1 < f2) {
                ++numLess;
            }
        }

        return (numLessEqual == TRACKS_WO_OLD.length && numLess > 0);
    }

    public static ArrayList<Individual2011> getNonDominatedTime(ArrayList<Individual2011> list) {
        ArrayList<Individual2011> result = new ArrayList<Individual2011>();

        for (int i = 0; i < list.size(); ++i) {
            //System.out.println("Teste ["+i+"]");

            boolean dominated = false;

            for (int k = 0; k < list.size() && !dominated; ++k) {
                if (k == i) {
                    continue;
                }
                if (dominatesTime(list.get(k), list.get(i))) {
                    dominated = true;
                }
            }

            if (!dominated) {
                //System.out.println("Is not dominated by any other point");
                result.add(list.get(i));
            }
        }

        return result;
    }

    public static ArrayList<Individual2011> getNonDominatedRank(ArrayList<Individual2011> list) {
        ArrayList<Individual2011> result = new ArrayList<Individual2011>();

        for (int i = 0; i < list.size(); ++i) {
            //System.out.println("Teste ["+i+"]");

            boolean dominated = false;

            for (int k = 0; k < list.size() && !dominated; ++k) {
                if (k == i) {
                    continue;
                }
                if (dominatesRank(list.get(k), list.get(i))) {
                    dominated = true;
                }
            }

            if (!dominated) {
                //System.out.println("Is not dominated by any other point");
                result.add(list.get(i));
            }
        }

        return result;
    }

    public static void analyze()
            throws Exception {
        BufferedReader reader = new BufferedReader(new FileReader(FILE));
        ArrayList<Individual2011> list = new ArrayList<Individual2011>();

        // header
        String line = reader.readLine();
        line = reader.readLine();

        while (line != null && line.trim().length() > 0) {
            Individual2011 ind = new Individual2011(line, TRACKS);
            list.add(ind);

            String alpine = "F:\\Quad\\Experiments\\CIG-2011\\EA-Runs\\TestRun5-Alpine-Noisy\\run02\\Torcs-Noisy_0001inds.log";
            String wheel2 = "F:\\Quad\\Experiments\\CIG-2011\\EA-Runs\\TestRun5-Wheel2-Noisy\\run01\\Torcs-Noisy_0001inds.log";

            if (ind.source.contains("Wheel2")) {
                ind.properties = de.janquadflieg.mrracer.PropertiesCreator.getIndividual(wheel2, "wheel2", ind.indID,
                        new de.janquadflieg.mrracer.evo.tools.EvostarParser()).properties;

            } else if (ind.source.contains("Alpine")) {
                ind.properties = de.janquadflieg.mrracer.PropertiesCreator.getIndividual(alpine, "alpine", ind.indID,
                        new de.janquadflieg.mrracer.evo.tools.EvostarParser()).properties;
            }

            line = reader.readLine();
        }

        System.out.println("Read " + list.size() + " individuals");
        System.out.println("Building rankSums for complete list");
        initRanks(list);

        for (int i = 0; i < TRACKS_WO_OLD.length; ++i) {
            System.out.println("20 best individuals on track " + TRACKS_WO_OLD[i]);
            Collections.sort(list, new FitnessComparator(TRACKS_WO_OLD[i]));

            for (int k = 0; k < 20; ++k) {
                Individual2011 ind = list.get(k);
                System.out.println(ind.indID + " from " + ind.source + " " + Utils.dTS(ind.fitness.get(TRACKS_WO_OLD[i])));
            }

            System.out.println("");
        }

        ArrayList<Individual2011> nonDominatedTime = getNonDominatedTime(list);
        System.out.println("Found " + nonDominatedTime.size() + " non dominated according to the time");
        System.out.println("Building rankSums, for non dominated according to time");
        initRanks(nonDominatedTime);

        Collections.sort(nonDominatedTime, new RankSumComparator());

        for (int i = 0; i < nonDominatedTime.size(); ++i) {
            Individual2011 ind = nonDominatedTime.get(i);
            System.out.println("[" + i + "] " + ind.indID + " from " + ind.source + " " + ind.rankSum);
        }

        System.out.println("");
        System.out.println("list of non dominated according to the time:");
        printMike(nonDominatedTime);
        System.out.println("");
        System.out.println("Complete individuals:");
        printNonDominated(nonDominatedTime);

        System.out.println("");
        ArrayList<Individual2011> nonDominatedRank = getNonDominatedRank(list);
        System.out.println("Found " + nonDominatedRank.size() + " non dominated according to the rank");
        System.out.println("Calculating new ranks");
        initRanks(nonDominatedRank);
        System.out.println("Sanity check, now non dominated: " + getNonDominatedRank(nonDominatedRank).size());
        Collections.sort(nonDominatedRank, new RankSumComparator());
        //System.out.println("Building rankSums, for non dominated according to ");

        System.out.println("list of non dominated according to ranks");
        printMike(nonDominatedRank);



        /*System.out.println("10 best by ranksum of the non dominated:");

        for(int i=0; i < 10; ++i){
        Individual2011 ind = nonDominatedTime.get(i);
        System.out.println("["+i+"] "+ind.indID+" from "+ind.source+" "+ind.rankSum);
        for(int k=0; k < TRACKS_WO_OLD.length; ++k){
        System.out.print(ind.ranks.get(TRACKS_WO_OLD[k])+" ");
        }
        System.out.println("");
        for(int k=0; k < TRACKS_WO_OLD.length; ++k){
        System.out.print(Utils.dTS(ind.fitness.get(TRACKS_WO_OLD[k]))+" ");
        }
        System.out.println("");

        System.out.println("--------------------------------");
        }*/


        /*System.out.println("");
        System.out.println("Now with the fitness sum");
        Collections.sort(nonDominatedTime, new FitnessSumComparator());

        for(int i=0; i < nonDominatedTime.size(); ++i){
        Individual2011 ind = nonDominatedTime.get(i);
        System.out.println("["+i+"] "+ind.indID+" from "+ind.source+" "+Utils.dTS(ind.fitnessSum));
        }

        System.out.println("10 best by fitnesssum of the non dominated:");

        for(int i=0; i < 10; ++i){
        Individual2011 ind = nonDominatedTime.get(i);
        System.out.println("["+i+"] "+ind.indID+" from "+ind.source+" "+Utils.dTS(ind.fitnessSum));
        for(int k=0; k < TRACKS_WO_OLD.length; ++k){
        System.out.print(ind.ranks.get(TRACKS_WO_OLD[k])+" ");
        }
        System.out.println("");
        for(int k=0; k < TRACKS_WO_OLD.length; ++k){
        System.out.print(Utils.dTS(ind.fitness.get(TRACKS_WO_OLD[k]))+" ");
        }
        System.out.println("");

        System.out.println("--------------------------------");
        }*/
    }

    public static void printNonDominated(ArrayList<Individual2011> list) {
        if (list.isEmpty()) {
            return;
        }
        ArrayList<String> keys = new ArrayList<String>();
        Iterator<Object> it = list.get(0).properties.keySet().iterator();
        while (it.hasNext()) {
            keys.add(it.next().toString());
        }

        System.out.print("track indid ");
        for (int i = 0; i < keys.size(); ++i) {
            System.out.print(keys.get(i) + " ");
        }
        System.out.println("");

        for (int k = 0; k < list.size(); ++k) {
            Individual2011 ind = list.get(k);
            System.out.print(ind.source+" "+ind.indID+" ");
            for (int i = 0; i < keys.size(); ++i) {
                System.out.print(ind.properties.getProperty(keys.get(i))+" ");
            }
            System.out.println("");
        }

    }

    public static void printMike(ArrayList<Individual2011> list) {
        System.out.print("id track ranksum ");
        for (int i = 1; i <= 6; ++i) {
            System.out.print("rt" + i + " ");
        }
        for (int i = 1; i <= 6; ++i) {
            System.out.print("ttim" + i + " ");
        }
        System.out.println("");
        for (int i = 0; i < list.size(); ++i) {
            Individual2011 ind = list.get(i);
            System.out.print(ind.indID + " " + ind.source + " " + ind.rankSum + " ");
            for (int k = 0; k < TRACKS_WO_OLD.length; ++k) {
                System.out.print(ind.ranks.get(TRACKS_WO_OLD[k]) + " ");
            }
            for (int k = 0; k < TRACKS_WO_OLD.length; ++k) {
                System.out.print(ind.fitness.get(TRACKS_WO_OLD[k]) + " ");
            }
            System.out.println("");
        }
    }

    public static void initRanks(ArrayList<Individual2011> list) {
        for (int k = 0; k < list.size(); ++k) {
            list.get(k).rankSum = 0;
            list.get(k).fitnessSum = 0.0;
        }

        for (int i = 0; i < TRACKS_WO_OLD.length; ++i) {
            Collections.sort(list, new FitnessComparator(TRACKS_WO_OLD[i]));


            for (int k = 0; k < list.size(); ++k) {
                list.get(k).rankSum += (k + 1);
                list.get(k).ranks.put(TRACKS_WO_OLD[i], (k + 1));
                list.get(k).fitnessSum += list.get(k).fitness.get(TRACKS_WO_OLD[i]);
            }
        }
    }

    public static void main(String[] args) {
        try {
            Analyze.analyze();

        } catch (Exception e) {
        }
    }
}
