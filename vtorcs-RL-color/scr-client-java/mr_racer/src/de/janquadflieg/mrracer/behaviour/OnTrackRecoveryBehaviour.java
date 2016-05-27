/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.behaviour;


import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.telemetry.*;



/**
 *
 * @author Jan Quadflieg
 */
public class OnTrackRecoveryBehaviour
        implements Behaviour {

    /** Gearchange behaviour. */
    private Behaviour gearChange = new SimpleGearChangeBehaviour();
    /** A counter to determine if the car is stuck. */
    private int speedCounter = 0;
    private boolean forwardAcc = true;
    /** Should the car drive backwards? */
    private boolean reverse = false;
    /** The last speed value. */
    private double lastSpeed = 0;
    /** Angles. */
    private float[] angles = new float[19];
    /** Desired position when driving backwards. */
    private static final double BACK_POS = -0.7;
    /** Desired angle when driving backwards. */
    private static final double BACK_ANGLE_D = -5.0;
    private boolean firstCall = true;

    public OnTrackRecoveryBehaviour() {
        // init angles
        for (int i = 0; i < 19; ++i) {
            angles[i] = -90 + i * 10;
        }
    }

    public OnTrackRecoveryBehaviour(float[] f) {
        System.arraycopy(f, 0, angles, 0, f.length);
    }

    @Override
    public void execute(SensorData data, ModifiableAction action) {
        //System.out.println("ONTR");
        double trackAngleD = Math.toDegrees(data.getAngleToTrackAxis());
        // looking to the right is the same as looking left mirrored
        boolean mirror = trackAngleD > 0;
        double steeringD = 0;

        if(mirror){
            trackAngleD *= -1.0;
        }

        if(firstCall){
            firstCall = false;            
            reverse = data.getSpeed() < 0.0;
        }

        if (reverse) {
            driveBackward(data, action);

        } else {
            driveForward(data, action);
        }

        // steering basically just depends on the direction in which we're driving
        if (data.getSpeed() < 0.0) {
            if (trackAngleD <= -90) {
                steeringD = 45;

            } else {
                steeringD = (1.0 - ((trackAngleD + 90) / 90)) * 45;
            }

        } else {
            // try to drive forward, life can be so simple
            if (trackAngleD <= -90.0) {
                steeringD = -45;

            } else if (trackAngleD > -90 && trackAngleD <= -9) {
                steeringD = (1.0 - ((trackAngleD + 90) / 90)) * -45.0;

            } else if (trackAngleD > -9 && trackAngleD <= 9) {
                // steering based on the "biggest sensor value" heuristic
                int index = SensorData.maxTrackIndexLeft(data);
                steeringD = -angles[index];
            }
        }

        if (mirror) {
            action.setSteering(steeringD / -45);

        } else {
            action.setSteering(steeringD / 45);
        }

        action.setClutch(0.0);
        action.limitValues();
    }

    private void driveBackward(SensorData data, ModifiableAction action) {
        double trackAngleD = Math.toDegrees(data.getAngleToTrackAxis());
        double trackPos = data.getTrackPosition();

        forwardAcc = false;

        // looking to the right is the same as looking left mirrored
        boolean mirror = trackAngleD > 0;

        if (mirror) {
            trackAngleD *= -1;
            trackPos *= -1;
        }

        boolean reachedPosition = trackPos < -0.7;

        if (reachedPosition || trackAngleD > BACK_ANGLE_D) {
            reverse = false;
            speedCounter = 0;
            driveForward(data, action);
        }

        // normierte distanz beim winkel
        double distanceAngle = ((trackAngleD - BACK_ANGLE_D) * (trackAngleD - BACK_ANGLE_D))
                / ((-180.0 - BACK_ANGLE_D) * (-180.0 - BACK_ANGLE_D));
        // normierte distanz bei der position
        double distancePos = ((trackPos - BACK_POS) * (trackPos - BACK_POS))
                / ((1.0 - BACK_POS) * (1.0 - BACK_POS));
        distancePos = Math.sqrt(distancePos);
        distanceAngle = Math.sqrt(distanceAngle);

        double distance = Math.min(distanceAngle, distancePos);
        double targetSpeed = -15.0 + -20.0 * distance;

        //System.out.println("Speed: "+Utils.dTS(data.getSpeed()));
        //System.out.println("DA: "+Utils.dTS(distanceAngle)+" DP: "+Utils.dTS(distancePos)+" D: "+Utils.dTS(distance)+" - "+Utils.dTS(targetSpeed));

        if (data.getSpeed() > 0.0 || data.getSpeed() < (targetSpeed-5.0)) {
            action.setAcceleration(0);
            action.setBrake(0.5);
            action.setGear(0);

        } else if (data.getSpeed() > targetSpeed) {
            action.setAcceleration(0.5);
            action.setBrake(0.0);
            action.setGear(-1);

        } else {
            action.setGear(0);
            action.setAcceleration(0.0);
            action.setBrake(0.0);
        }
    }

    private void driveForward(SensorData data, ModifiableAction action) {
        double trackAngleD = Math.toDegrees(data.getAngleToTrackAxis());

        // looking to the right is the same as looking left mirrored
        boolean mirror = trackAngleD > 0;

        if (mirror) {
            trackAngleD *= -1;
        }

        // when facing away from the track, we might hit an obstacle and get stuck
        if ((data.getSpeed() <= lastSpeed+2.0 && data.getSpeed() < 10)) {
            //|| Math.abs(lastRaceDistance - data.getDistanceRaced()) < 0.2) {
            ++speedCounter;
        }

        lastSpeed = data.getSpeed();

        // check if stuck
        if (speedCounter > 50 * 2) {
            reverse = true;
            driveBackward(data, action);
            return;
        }       

        if (data.getSpeed() < -5.0 && !forwardAcc) {
            action.setGear(0);
            action.setAcceleration(0);
            action.setBrake(0.5);

        } else {
            forwardAcc = true;
            action.setGear(1);
            action.setAcceleration(0.5);
            action.setBrake(0);

            if (data.getSpeed() > 30) {
                gearChange.execute(data, action);
            } else {
                gearChange.reset();
            }
        }
    }

    /**
     * The current situation of the car, as classified by an
     * appropriate classifier chosen by the controller which uses this behaviour.
     */
    @Override
    public void setSituation(de.janquadflieg.mrracer.classification.Situation s) {
    }

    @Override
    public void reset() {
        reverse = false;
        gearChange.reset();
        firstCall = true;
    }

    @Override
    public void shutdown() {
        gearChange.shutdown();
    }

    @Override
    public void setParameters(java.util.Properties params, String prefix){
    }

    @Override
    public void getParameters(java.util.Properties params, String prefix){
    }
}
