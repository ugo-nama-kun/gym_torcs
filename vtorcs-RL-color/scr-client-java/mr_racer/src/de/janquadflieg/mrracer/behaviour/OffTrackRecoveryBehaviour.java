/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.behaviour;

import de.janquadflieg.mrracer.telemetry.*;

import java.util.Properties;

/**
 *
 * @author quad
 */
public class OffTrackRecoveryBehaviour
        implements Behaviour {

    /** A counter to determine if the car is stuck. */
    private int speedCounter = 0;
    /** Should the car drive backwards? */
    private boolean reverse = false;
    /** The last speed value. */
    private double lastSpeed = 0;
    /** The last race distance. */
    private double lastRaceDistance = 0.0;    
    /** Target angle in degrees to get back on track when driving forward. */
    private double forwardAngleD = 45.0;
    public static final String F_ANGLE = "-OTRB.fowardAngleD-";
    /** Minimum acceleration when driving forwards. */
    private double forwardMinAcc = 0.2;
    public static final String F_MIN_ACC = "-OTRB.forwardMinAcc-";
    /** Maximum acceleration when driving forwards. */
    private double forwardMaxAcc = 1.0;
    public static final String F_MAX_ACC = "-OTRB.forwardMaxAcc-";
    /** Target angle in degrees to get back on track when driving backward. */
    private double backwardAngleD = -45.0;
    public static final String B_ANGLE = "-OTRB.backwardAngleD-";
    /** Minimum acceleration when driving backwards. */
    private double backwardMinAcc = 0.2;
    public static final String B_MIN_ACC = "-OTRB.backwardMinAcc-";
    /** Maximum acceleration when driving backwards. */
    private double backwardMaxAcc = 1.0;
    public static final String B_MAX_ACC = "-OTRB.backwardMaxAcc-";

    public void setParameters(Properties params, String prefix) {
        if (params.containsKey(prefix + F_ANGLE)) {
            forwardAngleD = Double.parseDouble(params.getProperty(prefix + F_ANGLE));
            //System.out.println("Setting ["+prefix+F_ANGLE+"] to "+forwardAngleD);
        }
        if (params.containsKey(prefix + F_MIN_ACC)) {
            forwardMinAcc = Double.parseDouble(params.getProperty(prefix + F_MIN_ACC));
            //System.out.println("Setting ["+prefix+F_MIN_ACC+"] to "+forwardMinAcc);
        }
        if (params.containsKey(prefix + F_MAX_ACC)) {
            forwardMaxAcc = Double.parseDouble(params.getProperty(prefix + F_MAX_ACC));
            //System.out.println("Setting ["+prefix+F_MAX_ACC+"] to "+forwardMaxAcc);
        }

        if (params.containsKey(prefix + B_ANGLE)) {
            backwardAngleD = Double.parseDouble(params.getProperty(prefix + B_ANGLE));
            //System.out.println("Setting ["+prefix+B_ANGLE+"] to "+backwardAngleD);
        }
        if (params.containsKey(prefix + B_MIN_ACC)) {
            backwardMinAcc = Double.parseDouble(params.getProperty(prefix + B_MIN_ACC));
            //System.out.println("Setting ["+prefix+B_MIN_ACC+"] to "+backwardMinAcc);
        }
        if (params.containsKey(prefix + B_MAX_ACC)) {
            backwardMaxAcc = Double.parseDouble(params.getProperty(prefix + B_MAX_ACC));
            //System.out.println("Setting ["+prefix+B_MAX_ACC+"] to "+backwardMaxAcc);
        }
    }

    public void getParameters(Properties params, String prefix) {
        params.setProperty(prefix + F_ANGLE, String.valueOf(forwardAngleD));
        params.setProperty(prefix + F_MIN_ACC, String.valueOf(forwardMinAcc));
        params.setProperty(prefix + F_MAX_ACC, String.valueOf(forwardMaxAcc));

        params.setProperty(prefix + B_ANGLE, String.valueOf(backwardAngleD));
        params.setProperty(prefix + B_MIN_ACC, String.valueOf(backwardMinAcc));
        params.setProperty(prefix + B_MAX_ACC, String.valueOf(backwardMaxAcc));
    }

    @Override
    public void execute(SensorData data, ModifiableAction action) {
        if (reverse) {
            driveBackward(data, action);

        } else {
            driveForward(data, action);
        }
        action.setClutch(0.0);
        lastRaceDistance = data.getDistanceRaced();
    }

    private void driveBackward(SensorData data, ModifiableAction action) {
        // right of the track is the same as left mirrored
        boolean mirror = data.getTrackPosition() < -1;

        double trackAngleD = Math.toDegrees(data.getAngleToTrackAxis());
        double steeringD = 0;

        if (mirror) {
            trackAngleD *= -1;
        }

        if (trackAngleD > 0) {
            reverse = false;
            speedCounter = 0;
            driveForward(data, action);
        }       

        if (trackAngleD <= backwardAngleD - 90.0) {
            steeringD = 45.0;

        } else if (trackAngleD <= backwardAngleD + 90.0) {
            double lb = backwardAngleD - 90.0;
            double ub = backwardAngleD + 90.0;
            steeringD = 45.0 - (90.0 * ((trackAngleD - lb) / (ub - lb)));

        } else {
            steeringD = -45.0;
        }

        /*if(trackAngleD > -25 && trackAngleD <= 0){
        steeringD = ((trackAngleD+25) / 25) * -20;

        } else if(trackAngleD > -90 && trackAngleD <= -25){
        steeringD = ((trackAngleD+25) / -65) * 45;

        } else {
        steeringD = 45;
        }*/

        if (mirror) {
            steeringD *= -1.0;
        }

        action.setSteering(steeringD / 45.0);

        if (data.getSpeed() > 1) {
            action.setAcceleration(0);
            action.setBrake(0.5);

        } else {
            double acc = backwardMinAcc
                    + (backwardMaxAcc - backwardMinAcc) * (1.0 - Math.abs(action.getSteering()));
            action.setAcceleration(acc);
            action.setBrake(0);
        }

        if (data.getSpeed() < -50.0) {
            action.setAcceleration(0.0);
        }

        action.setGear(-1);
    }

    private void driveForward(SensorData data, ModifiableAction action) {
        // right of the track is the same as left mirrored
        boolean mirror = data.getTrackPosition() < -1;

        double trackAngleD = Math.toDegrees(data.getAngleToTrackAxis());
        double steeringD = 0;

        if (mirror) {
            trackAngleD *= -1;
        }

        // when facing away from the track, we might hit an obstacle and get stuck
        if ((data.getSpeed() <= lastSpeed && data.getSpeed() < 10)
                || Math.abs(lastRaceDistance - data.getDistanceRaced()) < 0.2) {
            ++speedCounter;
        }

        lastSpeed = data.getSpeed();

        // when facing away from the race track, allways drive backwards
        if (trackAngleD < 0 || speedCounter > 50) {
            reverse = true;            
            driveBackward(data, action);
            return;

        } else if (trackAngleD <= (forwardAngleD + 90.0)) {
            //steeringD = -45.0 + (45.0 * ((trackAngleD + 90.0) / (forwardAngleD - -90.0)));
            //steeringD = ((trackAngleD - 10) / -100) * -45;
            double lb = forwardAngleD - 90.0;
            double ub = forwardAngleD + 90.0;
            steeringD = -45.0 + (90.0 * ((trackAngleD - lb) / (ub - lb)));

        } else if (trackAngleD > forwardAngleD + 90.0) {
            //steeringD = 45.0 * ((trackAngleD - forwardAngleD) / (180.0 - forwardAngleD));
            steeringD = 45.0;
        }

        if (mirror) {
            steeringD *= -1;
        }

        action.setSteering(steeringD / 45.0);

        if (data.getSpeed() < -1) {
            action.setAcceleration(0);
            action.setBrake(0.5);

        } else {
            double acc = forwardMinAcc
                    + (forwardMaxAcc - forwardMinAcc) * (1.0 - Math.abs(action.getSteering()));
            acc *= 1.0+((speedCounter/50.0)*3.0);
            action.setAcceleration(acc);
            action.setBrake(0);
        }

        action.setGear(1);

        if (data.getSpeed() > 50) {
            action.setAcceleration(0.0);
        }
    }

    @Override
    public void reset() {
        speedCounter = 0;
        reverse = false;
        lastSpeed = 0;
    }

    /**
     * The current situation of the car, as classified by an
     * appropriate classifier chosen by the controller which uses this behaviour.
     */
    @Override
    public void setSituation(de.janquadflieg.mrracer.classification.Situation s) {
    }

    public void shutdown() {
    }
}
