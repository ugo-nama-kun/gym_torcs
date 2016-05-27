/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.behaviour;

import de.janquadflieg.mrracer.classification.*;
import de.janquadflieg.mrracer.functions.GeneralisedLogisticFunction;
import de.janquadflieg.mrracer.telemetry.ModifiableAction;
import de.janquadflieg.mrracer.telemetry.SensorData;
import de.janquadflieg.mrracer.track.*;
import static de.janquadflieg.mrracer.data.CarConstants.CAR_WIDTH;

import java.util.Properties;

/**
 *
 * @author quad
 */
public class DampedAccelerationBehaviour
        implements AccelerationBehaviour {

    private Situation s;
    private double trackWidth;
    
    /** Anti lock brakes. */
    private Behaviour abs = new de.janquadflieg.mrracer.behaviour.ABS();
    private double targetSpeed = 0.0;
    private TrackSegment current;    
    private GeneralisedLogisticFunction accDamp = new GeneralisedLogisticFunction();
    public static final String ACC_DAMP = "-DAB.accDamp-";
    private GeneralisedLogisticFunction brakeDamp = new GeneralisedLogisticFunction();
    public static final String BRAKE_DAMP = "-DAB.brakeDamp-";
    private boolean dampOnStraight = false;
    public static final String ACC_DAMP_ON_STRAIGHT = "-DAB.dos-";

    /** Track margin in meter at the outside of a corner. */
    private static final double OUTSIDE_MARGIN = 1.0;

    public DampedAccelerationBehaviour() {
    }

    @Override
    public void setParameters(Properties params, String prefix){
        accDamp.setParameters(params, prefix+ACC_DAMP);
        brakeDamp.setParameters(params, prefix+BRAKE_DAMP);
        dampOnStraight = Boolean.parseBoolean(params.getProperty(prefix+ACC_DAMP_ON_STRAIGHT, "false"));
        //System.out.println("dampOnStraight: "+dampOnStraight);
    }

    @Override
    public void getParameters(Properties params, String prefix){
        accDamp.getParameters(params, prefix+ACC_DAMP);
        brakeDamp.getParameters(params, prefix+BRAKE_DAMP);
        params.put(prefix+ACC_DAMP_ON_STRAIGHT, String.valueOf(dampOnStraight));
    }

    @Override
    public void execute(SensorData data, ModifiableAction action) {
        if (data.getSpeed() <= targetSpeed) {
            action.setAcceleration(1.0);
            action.setBrake(0.0);

        } else if (data.getSpeed() > targetSpeed + 2.0) {
            action.setAcceleration(0.0);
            action.setBrake(1.0);

        } else {
            action.setAcceleration(0.2);
            action.setBrake(0.0);

            if ((current != null && !current.isFull()) || !s.isFull()) {
                if(data.getSpeed() > 40.0){
                    action.setAcceleration(0.0);
                }
            }
        }

        if (action.getAcceleration() > 0.0 &&
                ((current != null && current.isCorner()) || s.isCorner())) {            
            boolean mirror = false;
            double trackPos = data.getTrackPosition();
            
            if(current != null){
                mirror = current.isRight();

            } else {
                mirror = s.isRight();
            }

            if(mirror){
                trackPos *= -1.0;                
            }

            double absTrackPos = SensorData.calcAbsoluteTrackPosition(trackPos, trackWidth);

            // outside
            if(absTrackPos+(CAR_WIDTH*0.5) > trackWidth-OUTSIDE_MARGIN && data.getSpeed() > 75.0){
                double beta = ((absTrackPos+(CAR_WIDTH*0.5))-(trackWidth-OUTSIDE_MARGIN))/OUTSIDE_MARGIN;                
                beta = Math.min(beta, 1.0);
                double factor = 1.0 - (0.8 * (1.0-beta));
                action.setAcceleration(action.getAcceleration()*factor);
            }

            double steeringAngle = Math.abs(action.getSteering());
            double damp = accDamp.getMirroredValue(steeringAngle);

            action.setAcceleration(action.getAcceleration()*damp);

            if(data.getSpeed() < 10.0 && action.getAcceleration() < 0.5){
                action.setAcceleration(0.5);
            }
        }

        if (action.getAcceleration() > 0.0 && dampOnStraight &&
                ((current != null && current.isStraight()) || s.isStraight() || s.isStraightAC())) {
            double steeringAngle = Math.abs(action.getSteering());
            double damp = accDamp.getMirroredValue(steeringAngle);

            action.setAcceleration(action.getAcceleration()*damp);

            if(data.getSpeed() < 10.0 && action.getAcceleration() < 0.5){
                action.setAcceleration(0.5);
            }
        }

        if(action.getBrake() > 0.0){
            double steeringAngle = Math.abs(action.getSteering());

            double damp = brakeDamp.getMirroredValue(steeringAngle);

            action.setBrake(damp);
        }

        abs.execute(data, action);
        action.limitValues();
    }

    @Override
    public void reset() {
    }

    @Override
    public void shutdown() {
    }

    /**
     * The current situation of the car, as classified by an
     * appropriate classifier chosen by the controller which uses this behaviour.
     */
    @Override
    public void setSituation(de.janquadflieg.mrracer.classification.Situation s) {
        this.s = s;
    }

    /**
     * The segment of the trackmodel containing the current position of the car.
     * Might be null, if the trackmodel has not beeen initialized or unknown, if
     * the controller is still learning the track during the first lap.
     * @param s The track segment.
     */
    @Override
    public void setTrackSegment(TrackSegment s) {
        this.current = s;
    }

    /**
     * The desired target speed.
     * @param speed The speed.
     */
    @Override
    public void setTargetSpeed(double speed) {
        this.targetSpeed = speed;
    }

    /**
     * The width of the race track in meter.
     * @param width
     */
    @Override
    public void setWidth(double width) {
        trackWidth = width;
    }   
}