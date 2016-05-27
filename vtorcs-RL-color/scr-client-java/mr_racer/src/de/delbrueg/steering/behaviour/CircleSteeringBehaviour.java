/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.behaviour;

import de.delbrueg.steering.behaviour.decision.LineOrCircleDecider;
import de.delbrueg.steering.classification.CircleClassifier;
import de.delbrueg.steering.classification.CircleSituation;
import de.delbrueg.steering.classification.TrackPointClassifier;
import de.delbrueg.steering.classification.TrackPointSituation;
import de.delbrueg.predictor.lookahead_distance.LookAheadDistancePredictor;
import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.telemetry.ModifiableAction;
import de.janquadflieg.mrracer.telemetry.SensorData;
import de.janquadflieg.mrracer.track.TrackSegment;
import de.delbrueg.math.*;
import de.delbrueg.predictor.position.TargetPosPredictorData;
import de.delbrueg.predictor.position.TargetPositionPredictor;
import de.delbrueg.steering.behaviour.decision.LineOrCircleDecisionData;
import de.delbrueg.steering.classification.LineClassifier;
import de.delbrueg.steering.classification.LineSituation;
import de.delbrueg.steering.wheelAngle.WheelAngleCalcData;
import de.delbrueg.steering.wheelAngle.WheelAngleCalculator;
import de.janquadflieg.mrracer.behaviour.SteeringBehaviour;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;
import javax.vecmath.Point2d;
import javax.vecmath.Vector2d;

/**
 * This steering behaviour approximates the track and car movement with circles.
 *
 * @author Tim
 */
public class CircleSteeringBehaviour implements SteeringBehaviour{

    private float[] angles = new float[19];

    /**
     * a list of the last target positions
     */
    private List<Double> targetPositions = new LinkedList<Double>();

    private int numTargetPositions = 8;

    /**
     * the average direction of the last targetPositions
     */
    private double targetDirection = 0;


    /**
     * position to arrive at in next timestep
     * x = left/right in [-1,1]
     * y = distance delta along middle circle
     */
    private java.awt.geom.Point2D targetPosition = new java.awt.geom.Point2D.Double(0.0, 1.0);
    private double track_width;             // width of track in meters
    private CircleSteeringSettings settings;
    
    // maximum wheel turn possible
    private final double max_angle = Math.toRadians(45);

    // for debug
    private String lastDebugInfo = "";
    
    @Deprecated
    private TrackSegment currentSegment;    // not needed for now

    // JQ
    private boolean noisy = false;

    private static final boolean DEBUG_TEXT = false;

    public CircleSteeringBehaviour(float[] f) {
        System.arraycopy(f, 0, angles, 0, f.length);
        reset();
    }

    // JQ
    public void setNoisy(boolean b){
        noisy = b;
    }    

    @Override
    public void setTargetPosition(java.awt.geom.Point2D position) {
        double lastTargetPos = targetPosition.getX();
        targetPosition = position;
        
        if( CircleSteeringSettings.ignore_plan )
            targetPosition.setLocation(0, targetPosition.getY());

        // add as last
        targetPositions.add(position.getX());

        // remove first
        if( targetPositions.size() > numTargetPositions)
            targetPositions.remove(0);

        // calc new average targetDirection
        ListIterator<Double> it = targetPositions.listIterator();        
        Double tp_from = it.next();
        Double tp_to = null;
        targetDirection = 0;
        double smooth_target_pos = targetPosition.getX();
        while( it.hasNext() ){

            tp_to = it.next();

            targetDirection = 0.5*targetDirection + 0.5*(tp_to-tp_from);

            smooth_target_pos = 0.5*smooth_target_pos + 0.5*tp_to;

            tp_from = tp_to;
            tp_to = null;
        }
        
        if( CircleSteeringSettings.smooth_target_pos)
        {
            targetPosition.setLocation(smooth_target_pos, targetPosition.getY());
        }
        
        if (DEBUG_TEXT)
        {
            System.out.println("targetPosition: " + targetPosition);
            System.out.println("targetDirection: " + Utils.dTS(targetDirection));
        }
    }

    @Override
    @Deprecated
    public void setTrackSegment(TrackSegment s) {
        currentSegment = s;
    }

    @Override
    public void setWidth(double width) {
        track_width = width;
    }

    @Override
    public void reset() {
        targetPosition.setLocation(0.0, 1.0);
        currentSegment = null;
        track_width = 0.0;
    }

    @Override
    public void execute(SensorData data, ModifiableAction action) {
        try{

            lastDebugInfo = "tW: " + Utils.dTS(track_width) + "| ";

            // convert to m/s
            final double speed = Math.max(15,data.getSpeed()/3.6);

            // car position
            final Point2d car_pos = new Point2d(0,0);

            // car direction as normalized vector
            // negative angleToTrackAxis = counter-clockwise
            Vector2d car_dir = de.delbrueg.math.MathUtil.rotateVector(
                    new Vector2d(0,1),
                    Circle.RotationDirection.clockwise,
                    data.getAngleToTrackAxis());
            car_dir.normalize();

            LookAheadDistancePredictor dist_pred = CircleSteeringSettings.lookahead_dist_predictor.create();
            // setup predictor & classifier
            dist_pred.setSpeed(data.getSpeed()/3.6);
            dist_pred.setTrackWidth(track_width);

            TrackPointClassifier pointClassifier = new TrackPointClassifier(angles, noisy);
            pointClassifier.setDistance(dist_pred.predictMaxLookAheadDistance());
            pointClassifier.setThreshold(dist_pred.predictLookAheadThreshold());

            TrackPointSituation sit = pointClassifier.classify(data);

            // calc situations
            final CircleSituation circle_sit =
                    new CircleClassifier(track_width, angles, settings, noisy).classify(data, sit);
            final LineSituation line_sit =
                    new LineClassifier(track_width, angles, settings, noisy).classify(data, sit);



            // calc target Point
            TargetPositionPredictor targetPredictor;
            LineOrCircleDecider decider = CircleSteeringSettings.line_or_circle_decider.get();
            decider.setData(new LineOrCircleDecisionData(circle_sit, line_sit, track_width, data.getTrackPosition()));

            if( circle_sit.isValid() && (!CircleSteeringSettings.use_lines || !decider.isLinear() )){
                // use the circles
                lastDebugInfo = "cR: " + Utils.dTS(circle_sit.getCenterCircle().getRadius()) + "| ";
                lastDebugInfo += "tP: circle| ";
                targetPredictor = CircleSteeringSettings.target_pos_predictor_circles.get();
            }
            else{
                // use the lines
                lastDebugInfo += "tP: line| ";
                targetPredictor = CircleSteeringSettings.target_pos_predictor_lines.get();
            }


            // adjust timestep with lateral speed
            double time_step = CircleSteeringSettings.time_step;
            if( CircleSteeringSettings.timestep_dynamic ){

                // positive aTTA = right, negative PosDiff = right
                double dynamic_interval = 0.5*Math.pow(data.getAngleToTrackAxis()-targetDirection,2)-1;
                time_step *= 1.0- CircleSteeringSettings.timestep_dynamic_interval_size*dynamic_interval;

                if(DEBUG_TEXT)
                {
                    System.out.println("angleToTrackAxis: " + Utils.dTS(data.getAngleToTrackAxis()));
                    System.out.println("targetDirection: " + Utils.dTS(targetDirection));
                    System.out.println("dynamic_interval: " + Utils.dTS(dynamic_interval));
                    System.out.println("time_step: " + Utils.dTS(time_step));
                }
            }
            TargetPosPredictorData tppd = new TargetPosPredictorData
                    (car_pos, data.getTrackPosition(), targetPosition,
                    car_dir, speed, circle_sit, line_sit,
                    track_width, time_step);

            targetPredictor.setData(tppd);
            Point2d targetPoint = targetPredictor.predictFuturePosition( CircleSteeringSettings.target_position_weight );
//            lastDebugInfo += targetPredictor.debugString;

            if( DEBUG_TEXT )
                System.out.println("predicted targetPoint: " + targetPoint);

            Line l = new Line(car_pos, car_dir);
            if( l.contains(targetPoint) )
            {
                // just drive straight
                action.setSteering(0);
            }
            else{

                // calc the circle we have to drive in order to arrive at targetPoint
                Circle driveCircle = new de.delbrueg.math.Circle(car_pos, car_dir, targetPoint);
                lastDebugInfo += "| dC: " + Utils.dTS(driveCircle.getRadius());

                /**
                 * calculate the angle of the front wheels to drive along the driveCircle
                 */
                WheelAngleCalculator angleCalc = CircleSteeringSettings.wheel_angle_calculator.get();
                WheelAngleCalcData wacd = new WheelAngleCalcData(
                        driveCircle, data.getLateralSpeed()/3.6,
                        CircleSteeringSettings.car_track_width, car_pos, car_dir,
                        CircleSteeringSettings.car_mass, CircleSteeringSettings.car_prop_steering_factor, time_step);

                angleCalc.setData(wacd);
                double angle = angleCalc.getWheelTurnAngle();

                // apply it!
                action.setSteering(angleToSteering(angle));
            }
        }
        catch(Exception e){
            e.printStackTrace();
            System.out.println("error reported: " + e.toString());
            action.setSteering(0);
        }
    }

    /**
     * normalizes angle to [-1,1]
     * @param angle the angle to normalize should be in [-max_angle,max_angle]
     */
    private double angleToSteering(final double angle){
//        lastDebugInfo += "| angle: " + Utils.dTS(angle);
        double trunc_angle = angle / max_angle;
        trunc_angle = Utils.truncate(angle, -1, 1);
//        lastDebugInfo += "| norm: " + Utils.dTS(trunc_angle);
        return trunc_angle;
    }

    @Override
    public String getDebugInfo(){
        return lastDebugInfo;
    }

    @Override
    public void shutdown() {
    }

    /**
     * The current situation of the car, as classified by an
     * appropriate classifier chosen by the controller which uses this behaviour.
     */
    @Override
    public void setSituation(de.janquadflieg.mrracer.classification.Situation s){

    }

     @Override
    public void setParameters(java.util.Properties params, String prefix){
    }

    @Override
    public void getParameters(java.util.Properties params, String prefix){
    }
}
