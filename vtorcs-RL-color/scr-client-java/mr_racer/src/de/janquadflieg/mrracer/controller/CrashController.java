/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.controller;

import de.janquadflieg.mrracer.telemetry.*;
import de.janquadflieg.mrracer.track.*;

import java.util.ArrayList;

/**
 *
 * @author quad
 */
public class CrashController
        extends BaseController {

    private BaseController controller;
    private int gameTickCtr = 0;
    private int lapCtr = -1;
    private SensorData lastData = null;
    private boolean wasRecovering = false;
    private int nextCrashIdx = 0;
    private ArrayList<Integer> crashPoints = new ArrayList<Integer>();
    private ArrayList<Double> crashAngles = new ArrayList<Double>();

    public CrashController(BaseController c) {
        super(null);
        this.controller = c;

        crashPoints.add(1);
        crashAngles.add(0.1);

        crashPoints.add(4);
        crashAngles.add(-0.5);

        crashPoints.add(10);
        crashAngles.add(0.1);

        crashPoints.add(13);
        crashAngles.add(0.1);

        crashPoints.add(21);
        crashAngles.add(0.1);

        crashPoints.add(25);
        crashAngles.add(0.1);
    }

    @Override
    public champ2011client.Action unsaveControl(champ2011client.SensorModel m) {
        SensorData data = new SensorData(m);
        ++gameTickCtr;
        if (lastData != null && data.getDistanceFromStartLine() < lastData.getDistanceFromStartLine()
                && data.getDistanceRaced() > lastData.getDistanceRaced()) {
            ++lapCtr;
        }

        if (firstPacket) {
            firstPacket(data);
        }

        champ2011client.Action action = controller.control(m);

        if (!data.onTrack()) {
            if(!wasRecovering){
                nextCrashIdx = (nextCrashIdx + 1) % crashPoints.size();
            }
            wasRecovering = true;

        } else if (initiateCrash(data)) {
            int idx = crashPoints.indexOf(trackModel.getIndex(data.getDistanceFromStartLine()));
            action.steering *= crashAngles.get(idx);
            action.limitValues();

        } else if (data.onTrack() && wasRecovering
                && Math.abs(Math.toDegrees(data.getAngleToTrackAxis())) < 9.0) {
            wasRecovering = false;
        }

        lastData = data;

        return action;
    }

    private boolean initiateCrash(SensorData d) {
        int segIdx = trackModel.getIndex(d.getDistanceFromStartLine());
        TrackSegment seg = trackModel.getSegment(segIdx);

        // first lap?
        boolean result = lapCtr >= 0;

        // segment is a corner?
        //result &= seg.isCorner();

        // have we just recovered from a crash?
        result &= this.wasRecovering == false;

        // is this the next crash segment?
        result &= crashPoints.get(nextCrashIdx) == segIdx;

        //result &= crashPoints.contains(segIdx);

//        if (result && segIdx == 4) {
//            System.out.println("Crash @" + d.getDistanceFromStartLineS() + " " + wasRecovering);
//        }

        return result;
    }

    private void firstPacket(SensorData data) {
        if (getStage() == Stage.WARMUP) {
            System.out.println("CrashController cannot be used in warmup mode!");
            System.exit(1);

        } else if (getStage() == Stage.QUALIFYING || getStage() == Stage.RACE) {
            TrackModel result = trackDB.getByName(getTrackName());

            if (result == TrackDB.UNKNOWN_MODEL) {
                System.out.println("Failed to get the trackmodel for track " + getTrackName() + ".");
                System.exit(1);

            } else {
                trackModel = result;
            }

        } else {
            System.out.println("Unknown stage");
            System.out.println("Stage given was: " + getStage());
            System.exit(1);
        }

        firstPacket = false;
    }

    public void resetFull() {
        super.resetFull();
        controller.resetFull();
    }

    public void reset() {
        super.reset();
        controller.reset();


    }

    public void shutdown() {
        super.shutdown();
        controller.shutdown();

    }
}
