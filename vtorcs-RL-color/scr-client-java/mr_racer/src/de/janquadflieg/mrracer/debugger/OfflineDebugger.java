/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.debugger;

import champ2011client.Controller.Stage;
import de.janquadflieg.mrracer.telemetry.*;
import de.janquadflieg.mrracer.controller.*;
import de.janquadflieg.mrracer.Utils;

import java.io.File;
import javax.swing.JFileChooser;

/**
 *
 * @author quad
 */
public class OfflineDebugger {

    //private static String DEFAULT_PATH = "F:\\Quad\\Experiments\\CIG-2011\\test-wheel2";
    private static String DEFAULT_PATH = "F:\\Quad\\Experiments\\SCRC2011-CIG\\final-test";
    //private static String DEFAULT_PATH = "F:\\Quad\\Experiments\\SCRC2011\\Alpine";
    //private static String DEFAULT_PATH = "F:\\Quad\\svn\\Torcs-Paper\\TORCS\\CIG-2011\\experiments\\strecken";
    //private static String DEFAULT_PATH = "F:\\Quad\\Experiments\\SCRC2011-Gecco\\DirtOptimization";
    //private static String TRACKNAME = "ea_run_noisy_wheel2";
    //private static String DEFAULT_PATH = "X:\\Test\\china";
    //private static String TRACKNAME = "wheel2_noisy_naive";
    private static String TRACKNAME = "Wheel2-noisy";
    private static Stage STAGE = Stage.RACE;

    private static java.util.Random r = new java.util.Random(System.currentTimeMillis());

    public static void runController(File data, double startOutput, double endOutput) {
        Telemetry t = new Telemetry();
        System.out.println(data.getAbsolutePath());
        t.load(data.getAbsolutePath());
        t.shutdown();
        System.out.println("Loaded " + t.size() + " entries");

        Telemetry runTelem = new Telemetry();

        MrRacer2011 controller = new MrRacer2011(runTelem);
        //de.janquadflieg.mrracer.track.TrackModel.TEXT_DEBUG = true;
        controller.setStage(STAGE);
        controller.setTrackName(TRACKNAME);

        double time = 0.0;
        double sumTime = 0.0;
        champ2011client.SensorModel last = null;
        double maxLatSpeed = 0.0;
        double timeMaxLatSpeed = 0.0;

        long maxExecTime = Long.MIN_VALUE;
        int maxExecPacket = 0;
        long minExecTime = Long.MAX_VALUE;
        long avgExecTime = 0;

        for (int i = 0; i < t.size(); ++i) {
            if(i % 1000 == 0){
                System.out.println("Packet["+i+"]");
            }
            SensorData d = t.getSensorData(i);

            //d = makeNoisy(d);

            champ2011client.SensorModel m = d.getSensorModel();

            if (last != null && last.getCurrentLapTime() > m.getCurrentLapTime()) {
                sumTime += m.getLastLapTime();
            }

            time = sumTime + m.getCurrentLapTime();

            if (time >= startOutput && time <= endOutput) {
                System.out.println("");
                System.out.println("-----------------------------------------");
                System.out.println("Time: " + Utils.timeToExactString(time));
                System.out.println("LatSpeed: " + m.getLateralSpeed());
                System.out.println("Angle: " + Utils.dTS(Math.toDegrees(m.getAngleToTrackAxis())));
                System.out.println("Position: " + m.getTrackPosition());
                System.out.println("Log: "+t.getLog(i));
                if (Math.abs(m.getLateralSpeed()) > maxLatSpeed) {
                    maxLatSpeed = Math.abs(m.getLateralSpeed());
                    timeMaxLatSpeed = time;
                }
                //de.janquadflieg.mrracer.behaviour.CorrectingPureHeuristicSteering.TEXT_DEBUG = true;
            }

            long startExec = System.nanoTime();
            champ2011client.Action a = controller.control(m);
            long endExec = System.nanoTime();

            //try{
            //    Thread.sleep(10);
            //} catch(Exception e){}

            if(endExec-startExec > maxExecTime){
                maxExecPacket = i;
                maxExecTime = endExec-startExec;
            }
            minExecTime = Math.min(minExecTime, endExec-startExec);
            avgExecTime += endExec-startExec;
            if(endExec-startExec > 5 * 1000000){
                System.out.println("["+i+"]: "+t.getSensorData(i).getDistanceFromStartLineS()+
                        " time = "+Utils.dTS(((double) endExec-startExec) / 1000000.0)+"ms");
            }

            if (time >= startOutput && time <= endOutput) {
                System.out.println("Steering: " + Utils.dTS(t.getAction(i).getSteering() * 45.0));
            }

            if (time > endOutput) {
                //de.janquadflieg.mrracer.behaviour.CorrectingPureHeuristicSteering.TEXT_DEBUG = false;
            }
            last = m;
        }

        controller.reset();
        controller.shutdown(); 

        System.out.println("Max lateral speed: " + maxLatSpeed + " @ " + Utils.timeToExactString(timeMaxLatSpeed));

        
        System.out.println("Min exec time: "+Utils.dTS(((double) minExecTime) / 1000000.0)+"ms");
        System.out.println("Max exec time@"+maxExecPacket+": "+Utils.dTS(((double) maxExecTime) / 1000000.0)+"ms");
        System.out.println("["+maxExecPacket+"]: "+t.getSensorData(maxExecPacket).getDistanceFromStartLineS());
        System.out.println("Avg exec time: "+Utils.dTS((((double) avgExecTime) / 1000000.0) / t.size())+"ms");

        runTelem.shutdown();
        runTelem.save(new File("measure_noise.zip"));
    }

    public static void validateNoiseHandling(File data) {
        Telemetry t = new Telemetry();
        System.out.println(data.getAbsolutePath());
        t.load(data.getAbsolutePath());
        t.shutdown();
        System.out.println("Loaded " + t.size() + " entries");

        Telemetry log = new Telemetry();

        MrRacer2011 controller = new MrRacer2011(log);

        controller.setStage(Stage.WARMUP);
        controller.setTrackName("ValidateTestTrack");

        double error = 0.0;

        for (int i = 0; i < t.size(); ++i) {
            //System.out.println("Packet["+i+"]");
            SensorData d = t.getSensorData(i);

            d = makeNoisy(d);

            champ2011client.SensorModel m = d.getSensorModel();

            champ2011client.Action a = controller.control(m);
        }
        controller.shutdown();
        log.shutdown();

        System.out.println("Recorded "+log.size()+" entries");

        double smallestOpponent = 300.0;

        System.out.println("CorrectMeasure NoisyMeasure");
        for (int i = 0; i < Math.min(log.size(), t.size()); ++i) {
            if (i < 100) {
                System.out.println(t.getLog(i) + " " + log.getLog(i));
            }
            double[] opponents = t.getSensorData(i).getOpponentSensors();
            for(int k=0; k < opponents.length; ++k){
                smallestOpponent = Math.min(smallestOpponent, opponents[k]);
            }
            error += (Double.parseDouble(t.getLog(i)) - Double.parseDouble(log.getLog(i)))
                    * (Double.parseDouble(t.getLog(i)) - Double.parseDouble(log.getLog(i)));
        }

        error /= Math.min(log.size(), t.size());
        error = Math.sqrt(error);
        System.out.println("RMSE: " + error);
        System.out.println("Smallest opponent: "+smallestOpponent);
    }

    public static SensorData makeNoisy(SensorData d) {
        ModifiableSensorData result = new ModifiableSensorData();
        result.setData(d);        

        double[] track = result.getTrackEdgeSensors();
        for (int l = 0; l < track.length; ++l) {
            double random = r.nextGaussian() * 0.1;
            random += 1.0;
            track[l] *= random;
        }
        result.setTrackEdgeSensors(track);

        return result;
    }

    public static void main(String[] args) {
        JFileChooser fileDialog = new JFileChooser();
        fileDialog.setCurrentDirectory(new File(DEFAULT_PATH));

        double startOutput = 168.5;
        double endOutput = 175;

        int result = fileDialog.showOpenDialog(null);

        if (result == JFileChooser.APPROVE_OPTION) {
            System.out.println("OK");
            runController(fileDialog.getSelectedFile(), startOutput, endOutput);
            //OfflineDebugger.validateNoiseHandling(fileDialog.getSelectedFile());
        }
        System.out.println("Bye");
    }
}
