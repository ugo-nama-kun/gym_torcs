/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.plan;

import de.janquadflieg.mrracer.controller.BaseController;
import de.janquadflieg.mrracer.Utils;
import static de.janquadflieg.mrracer.data.CarConstants.*;
import de.janquadflieg.mrracer.telemetry.SensorData;
import de.janquadflieg.mrracer.track.*;

import java.io.File;
import java.io.FileOutputStream;
import java.util.ArrayList;
import java.util.Properties;

/**
 *
 * @author quad
 */
public class OnlineLearning2011 {
    private Plan2011 plan;
    private ArrayList<DataSet> meta = new ArrayList<DataSet>();
    private double start = 0.0;
    private double nextLap = 0.0;
    private int lapsOK = 0;
    private SensorData lastData;
    private TrackModel trackModel;
    private long[] times = new long[6];
    private int triesToGoUp = 0;
    private int lapCtr = 1;
    private boolean everTrouble = false;
    //private int firstLap
    private static final boolean TEXT_DEBUG = false;
    private int paramSet = 0;
    private BaseController controller;
    private int testingNextLap;
    private ArrayList<Double> testingTimes = new ArrayList<Double>();

    enum LearningPhase {

        Waiting, Testing, GoingDown, GoingUp, GoingFaster
    };
    private LearningPhase phase = LearningPhase.Waiting;

    public void update(SensorData data) {

        if (data.getDistanceRaced() > lastData.getDistanceRaced()
                && lastData.getDistanceFromStartLine() > data.getDistanceFromStartLine()) {
            if (TEXT_DEBUG) {
                System.out.println(lapCtr + " " + Utils.timeToExactString(data.getLastLapTime()));
            }
            ++lapCtr;
        }


        if (data.getDistanceRaced() > nextLap) {
            if (TEXT_DEBUG) {
                System.out.println("");
                System.out.println("-------------------------------------------------------");
                System.out.println("[" + phase.toString() + "] Evaluating last lap...");
            }
            if (phase == LearningPhase.Waiting) {
                if (TEXT_DEBUG) {
                    System.out.println("Done with waiting, testing parameter sets");
                }
                if (controller.getParameterSetCount() > 1) {
                    phase = LearningPhase.Testing;
                    times[1] = System.currentTimeMillis();
                    updateTesting(true, data);

                } else {
                    if (TEXT_DEBUG) {
                        System.out.println("Controller has only one parameter set, no testing needed");
                    }
                    phase = LearningPhase.GoingDown;
                    times[1] = System.currentTimeMillis();
                    times[2] = System.currentTimeMillis();
                }

            } else if (phase == LearningPhase.Testing) {
                updateTesting(false, data);

            } else if (phase == LearningPhase.GoingDown) {
                updateGoingDown();

            } else if (phase == LearningPhase.GoingUp) {
                updateGoingUp(false);

            } else if (phase == LearningPhase.GoingFaster) {
                updateGoingFaster(false);
            }

            nextLap = (data.getDistanceRaced() - data.getDistanceFromStartLine()) + trackModel.getLength() + 0.5;

            if(TEXT_DEBUG){
                System.out.println("-----------------------------------------");
                System.out.println("");
            }

        } else {
            updateDriving(data);
        }


        lastData = data;
    }

    private void updateDriving(SensorData data) {
        //boolean realOffTrack = Math.abs(data.getTrackPosition()) > 1.05;

        //Math.abs(data.getTrackPosition()*(trackModel.getWidth()/2.0))
        //    > ((trackModel.getWidth()/2.0)-0.3);

        if (phase == LearningPhase.Waiting) {
            return;
        }

        if (phase == LearningPhase.Testing) {
            return;
        }

        boolean gotDamage = data.getDamage() > lastData.getDamage() + 20.0;
        if (gotDamage) {
            if (TEXT_DEBUG) {
                System.out.println("I've got damage");
            }
        }

        if (data.getDistanceRaced() > lastData.getDistanceRaced()) {
            int index = trackModel.getIndex(data.getDistanceFromStartLine());
            meta.get(index).latSpeedInt += Math.abs(data.getLateralSpeed())
                    * (data.getDistanceRaced() - lastData.getDistanceRaced());
            meta.get(index).maxLatSpeed = Math.max(meta.get(index).maxLatSpeed,
                    Math.abs(data.getLateralSpeed()));
        }

        if (data.onTrack()) {
            double absPosition = SensorData.calcAbsoluteTrackPosition(data.getTrackPosition(),
                    trackModel.getWidth());
            int index = trackModel.getIndex(data.getDistanceFromStartLine());
            DataSet ds = meta.get(index);
            TrackSegment seg = trackModel.getSegment(index);
            int prevIndex = trackModel.decIdx(index);
            TrackSegment prevSeg = trackModel.getSegment(prevIndex);
            DataSet prevDs = meta.get(prevIndex);

            if (absPosition < CAR_WIDTH * 0.5) {
                if (seg.isRight() && !ds.onLimit) {
                    ds.onLimit = true;
                    if (TEXT_DEBUG) {
                        System.out.println("On limit in corner " + index);
                    }

                } else if (prevSeg.isCorner() && !prevDs.onLimit
                        && data.getDistanceRaced() - seg.getStart() < 3 * trackModel.getWidth()) {
                    prevDs.onLimit = true;
                    if (TEXT_DEBUG) {
                        System.out.println("On limit inside in corner " + index + ","
                                + "blaming the previous corner");
                    }
                }


            } else if (absPosition > trackModel.getWidth() - CAR_WIDTH * 0.5) {
                if (seg.isLeft() && !ds.onLimit) {
                    ds.onLimit = true;
                    if (TEXT_DEBUG) {
                        System.out.println("On limit in corner " + index);
                    }

                } else if (prevSeg.isCorner() && !prevDs.onLimit
                        && data.getDistanceRaced() - seg.getStart() < 3 * trackModel.getWidth()) {
                    prevDs.onLimit = true;
                    if (TEXT_DEBUG) {
                        System.out.println("On limit inside in corner " + index + ","
                                + "blaming the previous corner");
                    }
                }

            }
        }

        if ((!data.onTrack() && lastData.onTrack()) || gotDamage) {
            int index = trackModel.getIndex(data.getDistanceFromStartLine());
            DataSet ds = meta.get(index);
            TrackSegment seg = trackModel.getSegment(index);


            if (ds.corner && !ds.offTrack) {
                if (TEXT_DEBUG) {
                    if (!data.onTrack()) {
                        System.out.print("Left the track ");
                    }
                    if (gotDamage) {
                        System.out.print("Got damage ");
                    }
                    System.out.println("in corner " + index + ", " + "remembering that!");
                }

                ds.offTrack = true;
                TrackSegment.Apex[] apexes = seg.getApexes();
                int prevIndex = trackModel.decIdx(index);
                int nextIndex = trackModel.incIdx(index);
                TrackSegment prevSeg = trackModel.getSegment(prevIndex);
                TrackSegment nextSeg = trackModel.getSegment(nextIndex);
                DataSet prevDs = meta.get(prevIndex);
                DataSet nextDs = meta.get(prevIndex);

                if (data.getDistanceFromStartLine() < apexes[apexes.length - 1].position) {
                    if (TEXT_DEBUG) {
                        System.out.println("Happened before the last apex");
                    }
                    if (prevSeg.isCorner()) {
                        TrackSegment.Apex[] prevApexes = prevSeg.getApexes();
                        double distance = data.getDistanceFromStartLine() - prevApexes[prevApexes.length - 1].position;
                        if (TEXT_DEBUG) {
                            System.out.println("Previous segment was a corner too");
                            System.out.println("Blaming that one too :-)");
                        }
                        prevDs.offTrack = true;
                        if (distance > 100.0) {
                            if (TEXT_DEBUG) {
                                System.out.println("Nope, distance bigger than 100m, don't blame previous corner");
                            }
                            prevDs.offTrack = false;
                        }
                    }

                } else {
                    double distance = data.getDistanceFromStartLine() - apexes[apexes.length - 1].position;
                    if (TEXT_DEBUG) {
                        System.out.println("Happened " + Utils.dTS(distance) + "m after the last apex");
                    }
                    if (distance > 100.0) {
                        if (TEXT_DEBUG) {
                            System.out.println("Distance bigger than 100m, don't blame this corner");
                        }
                        ds.offTrack = false;

                    }
                    if (nextSeg.isCorner()) {
                        if (TEXT_DEBUG) {
                            System.out.println("Next segment is a corner");
                            System.out.println("Blaming that one too :-)");
                        }
                        nextDs.offTrack = true;
                    }
                }



            } else if (seg.isStraight() && (data.getDistanceFromStartLine() - seg.getStart()) < 30.0) {
                int prevIndex = trackModel.decIdx(index);
                TrackSegment prevSeg = trackModel.getSegment(prevIndex);
                DataSet prevDs = meta.get(prevIndex);
                if (prevSeg.isCorner() && !prevDs.offTrack) {
                    prevDs.offTrack = true;
                    if (TEXT_DEBUG) {
                        System.out.println("I left the track on a straight [" + index + "] " + Utils.dTS(data.getDistanceFromStartLine() - seg.getStart()) + "m away from corner " + prevIndex);
                        System.out.println("Blaming the corner for that");
                    }
                }
            }
        }
    }

    private void updateTesting(boolean justSwitched, SensorData data) {
        if (justSwitched) {
            testingNextLap = lapCtr + 2;
            return;
        }

        if (lapCtr == testingNextLap) {
            testingTimes.add(data.getLastLapTime());
            if (TEXT_DEBUG) {
                System.out.println("Lap time for parameter set " + paramSet
                        + "/" + (controller.getParameterSetCount() - 1) + ": " + Utils.timeToExactString(data.getLastLapTime()));
            }
            ++paramSet;

            if (paramSet < controller.getParameterSetCount()) {
                controller.selectParameterSet(paramSet);
                reinitTargetSpeeds();
                testingNextLap = lapCtr + 2;

            } else {
                if (TEXT_DEBUG) {
                    System.out.println("Done testing all parameter sets: ");
                }
                int best = 0;
                double bestTime = testingTimes.get(0);
                for (int i = 0; i < testingTimes.size(); ++i) {
                    if (TEXT_DEBUG) {
                        System.out.println(i + "/" + (controller.getParameterSetCount() - 1) + ": " + Utils.timeToExactString(testingTimes.get(i)));
                    }
                    if (testingTimes.get(i) < bestTime) {
                        bestTime = testingTimes.get(i);
                        best = i;
                    }
                }
                if (TEXT_DEBUG) {
                    System.out.println("Best: " + best);
                }
                controller.selectParameterSet(best);
                reinitTargetSpeeds();

                // switch to going down
                phase = LearningPhase.GoingDown;
                times[2] = System.currentTimeMillis();
            }
        } else {
            if(TEXT_DEBUG){
                System.out.println("One more lap to go: "+testingNextLap+"/"+lapCtr);
            }
        }
    }

    private void updateGoingDown() {
        boolean allOK = true;
        for (int i = 0; i < meta.size(); ++i) {
            DataSet ds = meta.get(i);
            allOK &= allOK && !ds.offTrack;

            if (!ds.corner) {
                continue;
            }

            if (TEXT_DEBUG) {
                TrackSegment seg = trackModel.getSegment(i);
                System.out.println("[" + i + "] latSpeedInt: " + Utils.dTS(ds.latSpeedInt / seg.getLength())
                        + ", max: " + Utils.dTS(ds.maxLatSpeed));
            }

            if (ds.offTrack) {
                everTrouble = true;
                TrackSegment seg = trackModel.getSegment(i);
                TrackSegment.Apex[] apexes = seg.getApexes();
                if (TEXT_DEBUG) {
                    System.out.println("I was too fast in segment " + i);
                }
                ds.problematic = true;

                // going slower
                for (int k = 0; k < apexes.length; ++k) {
                    // try to make this faster again later, by setting plus > 0
                    ds.plusCorrection = 0.5 * ds.minusCorrection;
                    double newSpeed = apexes[k].targetSpeed * (1.0 - ds.minusCorrection);
                    if (TEXT_DEBUG) {
                        System.out.println("Setting speed from " + Utils.dTS(apexes[k].targetSpeed) + " to " + Utils.dTS(newSpeed));
                    }
                    seg.setTargetSpeed(k, newSpeed);
                }
            }
        }

        // allways save the current speeds as save speeds
        saveTargetSpeedsAsSave();

        if (TEXT_DEBUG) {
            System.out.println("Resetting...");
        }

        for (DataSet ds : meta) {
            ds.offTrack = false;
            ds.latSpeedInt = 0.0;
            ds.maxLatSpeed = 0.0;
        }

        if (allOK) {
            ++lapsOK;
        } else {
            lapsOK = 0;
        }

        if (lapsOK == 2) {
            if (TEXT_DEBUG) {
                System.out.println("I completed two laps without problems");
                System.out.println("Switching to going up...");
            }
            phase = LearningPhase.GoingUp;
            times[3] = System.currentTimeMillis();
            lapsOK = 0;

            if (everTrouble == false) {
                if (TEXT_DEBUG) {
                    System.out.println("I never had any trouble");
                    System.out.println("Directly switching to going faster");
                }
                phase = LearningPhase.GoingFaster;
                times[4] = System.currentTimeMillis();
                updateGoingFaster(true);
            } else {
                updateGoingUp(true);
            }
        }
    }

    private void updateGoingUp(boolean justSwitched) {
        boolean allOK = true;
        for (int i = 0; i < meta.size(); ++i) {
            DataSet ds = meta.get(i);
            allOK &= allOK && !ds.offTrack;

            if (!ds.corner) {
                continue;
            }

            if (TEXT_DEBUG) {
                TrackSegment seg = trackModel.getSegment(i);
                System.out.println("[" + i + "] latSpeedInt: " + Utils.dTS(ds.latSpeedInt / seg.getLength())
                        + ", max: " + Utils.dTS(ds.maxLatSpeed));
            }

            if (ds.offTrack) {
                TrackSegment seg = trackModel.getSegment(i);
                TrackSegment.Apex[] apexes = seg.getApexes();
                if (TEXT_DEBUG) {
                    System.out.println("I was too fast in segment " + i);
                }

                ds.plusCorrection = 0.5 * ds.plusCorrection;
                for (int k = 0; k < apexes.length; ++k) {
                    double newSpeed = ds.saveSpeeds[k] * (1.0 + ds.plusCorrection);
                    if (TEXT_DEBUG) {
                        System.out.println("Setting speed from " + Utils.dTS(apexes[k].targetSpeed) + " to " + Utils.dTS(newSpeed));
                    }
                    seg.setTargetSpeed(k, newSpeed);
                }
            }
        }

        if (TEXT_DEBUG) {
            System.out.println("Resetting...");
        }

        for (DataSet ds : meta) {
            ds.offTrack = false;
            ds.latSpeedInt = 0.0;
            ds.maxLatSpeed = 0.0;
        }

        if (allOK) {
            ++lapsOK;

        } else {
            lapsOK = 0;
            ++triesToGoUp;
        }

        if (justSwitched) {
            if (TEXT_DEBUG) {
                System.out.println("Just switched, adjusting target speeds up");
            }
            for (int i = 0; i < meta.size(); ++i) {
                DataSet ds = meta.get(i);

                if (!ds.corner || ds.plusCorrection == 0.0) {
                    continue;
                }

                TrackSegment seg = trackModel.getSegment(i);
                TrackSegment.Apex[] apexes = seg.getApexes();

                for (int k = 0; k < apexes.length; ++k) {
                    double newSpeed = apexes[k].targetSpeed * (1.0 + ds.plusCorrection);
                    if (TEXT_DEBUG) {
                        System.out.println("Setting speed from " + Utils.dTS(apexes[k].targetSpeed) + " to " + Utils.dTS(newSpeed));
                    }
                    seg.setTargetSpeed(k, newSpeed);
                }
            }
            lapsOK = 0;

        } else if (triesToGoUp == 2) {
            if (TEXT_DEBUG) {
                System.out.println("I tried faster speeds two times without success");
                System.out.println("Switching to going faster (trying to get the other corners faster");
            }
            writeBackSaveSpeeds();
            phase = LearningPhase.GoingFaster;
            times[4] = System.currentTimeMillis();
            updateGoingFaster(true);

        } else if (lapsOK == 2) {
            if (TEXT_DEBUG) {
                System.out.println("I completed two laps without problems");
                System.out.println("Switching to going faster...");
            }
            saveTargetSpeedsAsSave();
            phase = LearningPhase.GoingFaster;
            times[4] = System.currentTimeMillis();
            updateGoingFaster(true);
        }
    }

    private void updateGoingFaster(boolean justSwitched) {
        if (justSwitched) {
            if (TEXT_DEBUG) {
                System.out.println("Just switched, setting increment to one percent");
            }
            lapsOK = 0;

            for (int i = 0; i < meta.size(); ++i) {
                DataSet ds = meta.get(i);

                if (ds.corner && !ds.problematic && !ds.onLimit) {
                    ds.plusCorrection = 0.01;

                } else {
                    ds.plusCorrection = 0.0;
                }
            }
        }

        boolean allOK = true;
        boolean offLastTwoSegments = false;
        for (int i = 0; i < meta.size(); ++i) {
            DataSet ds = meta.get(i);
            allOK &= allOK && !ds.offTrack;

            if (i >= meta.size() - 2 && ds.offTrack) {
                offLastTwoSegments = true;
            }

            if (!ds.corner) {
                continue;
            }

            if (TEXT_DEBUG) {
                TrackSegment seg = trackModel.getSegment(i);
                System.out.print("[" + i + "] latSpeedInt: " + Utils.dTS(ds.latSpeedInt / seg.getLength())
                        + ", max: " + Utils.dTS(ds.maxLatSpeed) + " ");
            }

            if (ds.offTrack || (ds.onLimit && ds.plusCorrection > 0.0)) {
                TrackSegment seg = trackModel.getSegment(i);
                TrackSegment.Apex[] apexes = seg.getApexes();
                if (TEXT_DEBUG) {
                    System.out.print("too fast, reset to save, inc=0 ");
                }

                ds.plusCorrection = 0.0;
                ds.onLimit = true;
                for (int k = 0; k < apexes.length; ++k) {
                    double newSpeed = ds.saveSpeeds[k];
                    if (TEXT_DEBUG) {
                        System.out.print("[" + Utils.dTS(apexes[k].targetSpeed) + "->" + Utils.dTS(newSpeed) + "] ");
                    }
                    seg.setTargetSpeed(k, newSpeed);
                }
            }
            if (TEXT_DEBUG) {
                System.out.println("");
            }
        }

        if (TEXT_DEBUG) {
            System.out.println("Resetting...");
        }
        for (DataSet ds : meta) {
            ds.offTrack = false;
            ds.latSpeedInt = 0.0;
            ds.maxLatSpeed = 0.0;
        }

        if (allOK) {
            ++lapsOK;

        } else {
            if (offLastTwoSegments) {
                lapsOK = 0;

            } else {
                if (TEXT_DEBUG) {
                    System.out.println("On track in last two segments, pretending that I completed on lap without problems");
                }
                lapsOK = 1;
            }
        }

        if (justSwitched) {
            lapsOK = 0;
            if (TEXT_DEBUG) {
                System.out.println("Adjusting target speeds");
            }
            for (int i = 0; i < meta.size(); ++i) {
                DataSet ds = meta.get(i);

                if (!ds.corner || ds.plusCorrection == 0.0) {
                    continue;
                }

                TrackSegment seg = trackModel.getSegment(i);
                TrackSegment.Apex[] apexes = seg.getApexes();
                if (TEXT_DEBUG) {
                    System.out.print("Maybe can go faster in " + i + " ");
                }

                for (int k = 0; k < apexes.length; ++k) {
                    double newSpeed = apexes[k].targetSpeed * (1.0 + ds.plusCorrection);
                    if (TEXT_DEBUG) {
                        System.out.print("[" + Utils.dTS(apexes[k].targetSpeed) + "->" + Utils.dTS(newSpeed) + "] ");
                    }
                    seg.setTargetSpeed(k, newSpeed);
                }
                if(TEXT_DEBUG){
                    System.out.println("");
                }
            }
        }

        if (lapsOK == 2) {
            if (TEXT_DEBUG) {
                System.out.println("2 laps without problems, saving speeds");
            }
            saveTargetSpeedsAsSave();

            lapsOK = 1;

            if (TEXT_DEBUG) {
                System.out.println("Adjusting target speeds");
            }
            for (int i = 0; i < meta.size(); ++i) {
                DataSet ds = meta.get(i);

                if (!ds.corner || ds.plusCorrection == 0.0) {
                    continue;
                }

                TrackSegment seg = trackModel.getSegment(i);
                TrackSegment.Apex[] apexes = seg.getApexes();

                if (TEXT_DEBUG) {
                    System.out.print("Maybe can go faster in " + i + " ");
                }

                for (int k = 0; k < apexes.length; ++k) {
                    double newSpeed = apexes[k].targetSpeed * (1.0 + ds.plusCorrection);
                    if (TEXT_DEBUG) {
                        System.out.print("[" + Utils.dTS(apexes[k].targetSpeed) + "->" + Utils.dTS(newSpeed) + "] ");
                    }
                    seg.setTargetSpeed(k, newSpeed);
                }
                if(TEXT_DEBUG){
                    System.out.println("");
                }
            }
        }
    }

    private void saveTargetSpeedsAsSave() {
        for (int i = 0; i < trackModel.size(); ++i) {
            TrackSegment seg = trackModel.getSegment(i);

            if (seg.isCorner()) {
                TrackSegment.Apex[] apexes = seg.getApexes();
                for (int k = 0; k < apexes.length; ++k) {
                    meta.get(i).saveSpeeds[k] = apexes[k].targetSpeed;
                }
            }
        }
    }

    private void writeBackSaveSpeeds() {
        for (int i = 0; i < trackModel.size(); ++i) {
            TrackSegment seg = trackModel.getSegment(i);

            if (seg.isCorner()) {
                TrackSegment.Apex[] apexes = seg.getApexes();
                for (int k = 0; k < apexes.length; ++k) {
                    seg.setTargetSpeed(k, meta.get(i).saveSpeeds[k]);
                }
            }
        }
    }

    public void saveData() {
        System.out.println("Done with online learning, saving data");
        System.out.println("Last lap was " + Utils.timeToExactString(lastData.getLastLapTime()));
        times[5] = System.currentTimeMillis();
        for (int i = 0; i < meta.size(); ++i) {
            TrackSegment seg = trackModel.getSegment(i);
            DataSet ds = meta.get(i);

            if (seg.isCorner()) {
                TrackSegment.Apex[] apexes = seg.getApexes();
                for (int k = 0; k < apexes.length; ++k) {
                    seg.setTargetSpeed(k, ds.saveSpeeds[k]);
                }
            }
        }
        if (times[0] != Long.MAX_VALUE && times[1] != Long.MAX_VALUE) {
            double seconds = (times[1] - times[0]) / 1000.0;
            System.out.println("Waiting took " + Utils.timeToExactString(seconds));
        }
        if (times[1] != Long.MAX_VALUE && times[2] != Long.MAX_VALUE) {
            double seconds = (times[2] - times[1]) / 1000.0;
            System.out.println("Testing parameter sets took " + Utils.timeToExactString(seconds));
        }
        if (times[2] != Long.MAX_VALUE && times[3] != Long.MAX_VALUE) {
            double seconds = (times[3] - times[2]) / 1000.0;
            System.out.println("Correcting speeds down took " + Utils.timeToExactString(seconds));
        }
        if (times[3] != Long.MAX_VALUE && times[4] != Long.MAX_VALUE) {
            double seconds = (times[4] - times[3]) / 1000.0;
            System.out.println("Correcting speeds up again took " + Utils.timeToExactString(seconds));
        }
        if (times[4] != Long.MAX_VALUE && times[5] != Long.MAX_VALUE) {
            double seconds = (times[5] - times[4]) / 1000.0;
            System.out.println("Trying to go faster took " + Utils.timeToExactString(seconds));
        }
        System.out.println("Saving the chosen parameter set");
        try {
            Properties p = new Properties();
            controller.getParameters(p);

            FileOutputStream out = new FileOutputStream(new File("." + java.io.File.separator + controller.getTrackName() + BaseController.PARAMETER_EXT));
            p.store(out, "Selected during warmup");
            out.close();

        } catch (Exception e) {
            e.printStackTrace(System.out);
        }
    }

    public void init(TrackModel model, Plan2011 plan, SensorData data, BaseController c) {
        controller = c;
        this.plan = plan;
        lapCtr = ((int) (data.getDistanceRaced() / model.getLength())) + 1;
        if (TEXT_DEBUG) {
            System.out.println("Initializing Online Learner [" + model.getName() + "] @lap "
                    + lapCtr);
            System.out.println("Last lap time: " + Utils.timeToExactString(data.getLastLapTime()));
        }
        java.util.Arrays.fill(times, Long.MAX_VALUE);


        trackModel = model;
        for (int i = 0; i < model.size(); ++i) {
            TrackSegment s = model.getSegment(i);
            if (s.isCorner()) {
                //System.out.println("Segment " + i + " is a corner");
                DataSet ds = new DataSet();
                ds.corner = true;

                TrackSegment.Apex[] apexes = s.getApexes();

                ds.saveSpeeds = new double[apexes.length];

                for (int k = 0; k < apexes.length; ++k) {
                    apexes[k].targetSpeed = TrackSegment.DEFAULT_SPEED;
                    double speed = plan.getTargetSpeed(apexes[k]);
                    s.setTargetSpeed(k, speed);
                    ds.saveSpeeds[k] = speed;
                    //System.out.println("Default speed at apex[" + k + "]: " + Utils.dTS(speed));
                }

                meta.add(ds);

            } else {
                meta.add(new DataSet());
            }
        }

        times[0] = System.currentTimeMillis();

        if (data.getDistanceFromStartLine() < 20.0) {
            if (TEXT_DEBUG) {
                System.out.println("Near to the start finish line, no waiting needed");
            }

            if (controller.getParameterSetCount() > 1) {
                phase = LearningPhase.Testing;
                times[1] = System.currentTimeMillis();
                testingNextLap = lapCtr+2;

            } else {
                if (TEXT_DEBUG) {
                    System.out.println("Controller has only one parameter set, no testing needed");
                }
                phase = LearningPhase.GoingDown;
                times[1] = System.currentTimeMillis();
                times[2] = System.currentTimeMillis();
            }

            // TODO TESTINT
        } else {
            phase = LearningPhase.Waiting;
        }

        start = data.getDistanceRaced() - data.getDistanceFromStartLine();
        nextLap = start + model.getLength() + 0.5;

        lastData = data;
    }

    private void reinitTargetSpeeds(){
        for (int i = 0; i < meta.size(); ++i) {
            DataSet ds = meta.get(i);
            TrackSegment s = trackModel.getSegment(i);
            if (ds.corner) {
                //System.out.println("Segment " + i + " is a corner");
                TrackSegment.Apex[] apexes = s.getApexes();                

                for (int k = 0; k < apexes.length; ++k) {
                    apexes[k].targetSpeed = TrackSegment.DEFAULT_SPEED;
                    double speed = plan.getTargetSpeed(apexes[k]);
                    s.setTargetSpeed(k, speed);
                    ds.saveSpeeds[k] = speed;
                    //System.out.println("Default speed at apex[" + k + "]: " + Utils.dTS(speed));
                }
            }
        }
    }

    static class DataSet {

        public boolean onLimit = false;
        public boolean problematic = false;
        public boolean corner = false;
        public boolean offTrack = false;
        public double minusCorrection = 0.2;
        public double plusCorrection = 0.0;
        public double[] saveSpeeds = new double[0];
        public double latSpeedInt = 0.0;
        public double maxLatSpeed = 0.0;
    }
}
