/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer;


import de.janquadflieg.mrracer.evo.Individual2011;
import de.janquadflieg.mrracer.evo.tools.*;

import java.io.*;
import java.util.*;


/**
 * Helper class to create properties objects for the parameters of the controllers
 * from EA log files.
 *
 * @author quad
 */
public class PropertiesCreator {    

     public static ArrayList<ArrayList<String>> getAll(String fileName) {
        ArrayList<ArrayList<String>> result = new ArrayList<ArrayList<String>>();
        ArrayList<String> header = new ArrayList<String>();

        return result;
    }
    

    public static ArrayList<Individual2011> getAll(String fileName, String track,
            IndividualParser parser) {
        ArrayList<Individual2011> result = new ArrayList<Individual2011>();
        ArrayList<String> header = new ArrayList<String>();

        try {
            BufferedReader reader = new BufferedReader(new FileReader(fileName));

            String line = reader.readLine();

            while (!line.startsWith("#         indno        genno fitnessValues")) {
                line = reader.readLine();
            }

            line = reader.readLine();

            while (line != null && line.trim().length() > 0) {
                Individual2011 ind = parser.parse(line, track);
                ind.source = fileName;
                result.add(ind);
                line = reader.readLine();
            }

            reader.close();

        } catch (Exception e) {
            e.printStackTrace(System.out);
        }

        //System.out.println("Read "+result.size()+" entries");

        return result;
    }

    public static Individual2011 getIndividual(String fileName, String track, int indID,
            IndividualParser parser) {
        ArrayList<Individual2011> list = getAll(fileName, track, parser);

        for (int i = 0; i < list.size(); ++i) {
            Individual2011 ind = list.get(i);
            if (ind.indID == indID) {
                return ind;
            }
        }

        return null;
    }


    public static void main(String[] args) {
        try {
            //String file = "F:\\Quad\\Experiments\\Torcs-Test4f\\Torcs-Testrun_0006inds.log";
            //String file = "F:\\Quad\\Experiments\\CIG-2011\\EA-Runs\\TestRun1-Wheel2-Noisy\\run01\\Torcs-Noisy_0001inds.log";
            //String file = "F:\\Quad\\Experiments\\CIG-2011\\EA-Runs\\TestRun2-Wheel2-Noisy\\run01\\Torcs-Noisy_0001inds.log";
            //String file = "F:\\Quad\\Experiments\\CIG-2011\\EA-Runs\\TestRun3-Wheel2-Noisy\\run01\\Torcs-Noisy_0001inds.log";
            //String file = "F:\\Quad\\Experiments\\CIG-2011\\EA-Runs\\TestRun4-Wheel2-Noisy\\run01\\Torcs-Noisy_0001inds.log";
            //String fileDrive = "F:\\Quad\\Experiments\\CIG-2011\\EA-Runs\\TestRun5-Wheel2-Noisy\\run01\\Torcs-Noisy_0001inds.log";
            //String fileDrive = "F:\\Quad\\Experiments\\SCRC2011-Gecco\\DirtOptimization\\Torcs-Dirt-Noisy_0001inds.log";
            //String fileDrive = "F:\\Quad\\Experiments\\SCRC2011-Gecco\\DirtOptimization\\run03-2011-06-30\\Torcs-Dirt-Noisy_0001inds.log";
            String fileDrive = "F:\\Quad\\Experiments\\SCRC2011-CIG\\clutch-optimization\\run11\\Torcs-CIG-with-clutch_0001inds.log";
            int idDrive = 306;
            //String trackDrive = "Dirt-6";
            String trackDrive = "Wheel2";
            
            //String fileRecovery = "F:\\Quad\\Experiments\\SCRC2011-Gecco\\RecoveryOptimization\\run01-2011-06-17\\Torcs-Noisy_0001inds.log";
            //String fileRecovery = "F:\\Quad\\Experiments\\SCRC2011-Gecco\\RecoveryOptimization\\run02-2011-06-20\\Torcs-Noisy-Recovery_0001inds.log";
            String fileRecovery = "F:\\Quad\\Experiments\\SCRC2011-Gecco\\RecoveryOptimization\\run03-2011-06-24\\Torcs-Noisy-Recovery_0001inds.log";
            int idRecovery = 591;
            String trackRecovery = "Wheel2";
            
            String saveTo = "f:\\quad\\svn\\Diplomarbeit\\Code\\projects\\MrRacer\\src\\de\\janquadflieg\\mrracer\\data\\mrracer2011cig_a";

            Individual2011 indDrive = PropertiesCreator.getIndividual(fileDrive, trackDrive, idDrive, new EvostarParser());
            // only enable this for dirt settings!
            indDrive.properties.setProperty("-MrRacer2011.Acc--DAB.dos-", String.valueOf(false));
            System.out.println("Properties of the driving individual:");
            indDrive.properties.list(System.out);

            Individual2011 indRecovery = PropertiesCreator.getIndividual(fileRecovery, trackRecovery, idRecovery, new RecoveryParser());
            System.out.println("Properties of the recovery individual:");
            indRecovery.properties.list(System.out);

            String comment = "Default parameters for MrRacer2011, driving behaviour from " + indDrive.source + " indID " + indDrive.indID
                    + ", fitness: ";
            Iterator<Map.Entry<String, Double>> it = indDrive.fitness.entrySet().iterator();

            while (it.hasNext()) {
                Map.Entry<String, Double> entry = it.next();
                comment += entry.getKey() + "=" + entry.getValue();
                if (it.hasNext()) {
                    comment += ", ";
                }
            }

            comment += ", recovery behaviour from " + indRecovery.source + " indID " + indRecovery.indID
                    + ", fitness: ";
            it = indRecovery.fitness.entrySet().iterator();

            while (it.hasNext()) {
                Map.Entry<String, Double> entry = it.next();
                comment += entry.getKey() + "=" + entry.getValue();
                if (it.hasNext()) {
                    comment += ", ";
                }
            }

            Properties p = indDrive.properties;
            p.putAll(indRecovery.properties);

            System.out.println(comment);
            p.list(System.out);

            FileOutputStream out = new FileOutputStream(new File(saveTo));
            p.store(out, comment);
            out.close();

            System.out.println("done");

        } catch (Exception e) {
            e.printStackTrace(System.out);
        }
    }
}
