/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.plan;

import de.delbrueg.experiment.PlanInterface;
import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.classification.Situation;
import de.janquadflieg.mrracer.classification.Situations;
import de.janquadflieg.mrracer.controller.BaseController;
import de.janquadflieg.mrracer.functions.GeneralisedLogisticFunction;
import de.janquadflieg.mrracer.gui.GraphicDebugable;
import de.janquadflieg.mrracer.opponents.OpponentObserver;
import de.janquadflieg.mrracer.telemetry.ModifiableSensorData;
import de.janquadflieg.mrracer.telemetry.SensorData;
import de.janquadflieg.mrracer.track.*;

import java.util.ArrayList;
import java.util.Properties;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.geom.Point2D;

import flanagan.interpolation.CubicSpline;

/**
 *
 * @author quad
 */
public class Plan2011
        implements GraphicDebugable, PlanInterface {

    /** Maximum possible speed. */
    public static final double MAX_SPEED = 330.0;
    /** Graphics Debug? */
    private static final boolean DEBUG = false;
    /** Debug data. */
    private ArrayList<SensorData> debugData = new ArrayList<SensorData>();
    /** Debug in text mode? */
    private static final boolean TEXT_DEBUG = false;
    /** In this mode, we ignore the target speeds saved in the track model and always use the logistic function. */
    private static boolean EA_MODE = false;
    /** Maximum deviation allowed before a replan occurs. */
    private final static double MAX_POS_DEVIATION_M = 1.0;
    /** Modifier for the target speeds. */
    private final static double SPEED_MODIFIER = 1.0;
    /** Look ahead. */
    private final static double LOOK_AHEAD = 200.0;
    /** The stage of the race. */
    private champ2011client.Controller.Stage stage = champ2011client.Controller.Stage.QUALIFYING;
    /** Online learning data during the warmup. */
    private OnlineLearning2011 onlineLearner = null;
    /** Observer for opponents. */
    private OpponentObserver observer;
    /** Last target point. */
    private Point2D lastTargetPoint = OpponentObserver.NO_RECOMMENDED_POINT;
    /** The Trackmodel. */
    private TrackModel trackModel = new TrackModel();
    /** Debug Info.*/
    private StringBuilder info = new StringBuilder();
    /** The plan. */
    private ArrayList<PlanElement2011> plan = new ArrayList<PlanElement2011>(20);
    /** Index of the current plan element. */
    private int planIndex;
    /** Debug painter. */
    private DebugPainter debugPainter;
    /** The estimated next race distance. */
    private double nextDistance = 0.0;
    /** The current race distance. */
    private double currentDistance = 0.0;
    /** The desired position for approching corners and at their apex. */
    private double offset = 0.0;
    /** Last data packet. */
    private ModifiableSensorData lastData = new ModifiableSensorData();
    /** Brake coefficient for corners. */
    private double brakeCornerCoeff = 0.5;
    /** String Identifier. */
    public static final String BRAKE_CORNER_COEFF = "-PLAN.BCC-";
    /** Target Speeds. */
    private GeneralisedLogisticFunction targetSpeeds = new GeneralisedLogisticFunction();
    /** String Identifier. */
    public static final String TARGET_SPEEDS = "-PLAN.targetSpeeds-";
    /** Controller. */
    private BaseController controller;

    public Plan2011(String s, boolean loadFromCP, OpponentObserver observer,
            BaseController c) {
        this.observer = observer;
        this.controller = c;
        if (DEBUG) {
            debugPainter = new DebugPainter();
            debugPainter.setName("Plan");
        }

        if (System.getProperties().containsKey("EAMode")) {
            System.out.println("Plan: Turning EA Mode on");
            EA_MODE = true;
        }

        Properties params = new Properties();
        params.setProperty(GeneralisedLogisticFunction.M, String.valueOf(0.2));
        targetSpeeds.setParameters(params, "");
    }

    public void setParameters(Properties params, String prefix) {
        targetSpeeds.setParameters(params, prefix + TARGET_SPEEDS);
        if (params.containsKey(prefix + Plan2011.BRAKE_CORNER_COEFF)) {
            this.brakeCornerCoeff = Double.parseDouble(params.getProperty(prefix + Plan2011.BRAKE_CORNER_COEFF));
            //System.out.println("Setting ["+prefix+Plan2011.BRAKE_CORNER_COEFF+"] to "+brakeCornerCoeff);
        }
    }

    public void getParameters(Properties params, String prefix) {
        targetSpeeds.getParameters(params, prefix + TARGET_SPEEDS);
        params.setProperty(prefix + Plan2011.BRAKE_CORNER_COEFF, String.valueOf(brakeCornerCoeff));
    }

    public void setStage(champ2011client.Controller.Stage s) {
        stage = s;
        observer.setStage(s);
        //System.out.println("Plan, stage="+stage.toString());
    }

    @Override
    public void reset() {
        observer.reset();
        lastTargetPoint = OpponentObserver.NO_RECOMMENDED_POINT;
        plan.clear();
        planIndex = 1;
        onlineLearner = null;
    }

    public void functionalReset() {
        plan.clear();
        planIndex = 1;
        observer.reset();
        lastTargetPoint = OpponentObserver.NO_RECOMMENDED_POINT;
    }

    private static double predictSpeed(double currentSpeed, double accDistance) {
        final double[] SPEED = {0.0443872, 46.2205, 61.2405, 75.9582,
            89.3808, 101.969, 113.35, 122.522, 125.621, 131.856, 138.126, 143.926, 149.735,
            155.024, 160.068, 164.839, 168.601, 169.251, 172.511, 175.752, 178.972, 181.884,
            184.779, 187.658, 190.522, 193.368, 196.198, 198.967, 201.38, 203.72, 205.99,
            208.192, 210.329, 212.401, 214.412, 216.133, 215.407, 216.995, 218.598,
            220.205, 221.813, 223.419, 225.022, 226.441, 227.849, 229.244, 230.624, 231.984,
            233.337, 234.678, 236.005, 237.321, 238.631, 239.932, 241.223, 242.491,
            243.731, 244.944, 245.747, 246.048, 247.099, 248.182, 249.269, 250.356, 251.444,
            252.516, 253.563, 254.508, 255.46, 256.402, 257.323, 258.222, 259.097, 259.944,
            260.758, 261.544, 262.315, 263.066, 263.798, 264.513, 265.213, 264.408,
            264.308, 264.882, 265.456, 266.032, 266.6, 267.156, 267.696, 268.213,
            268.69, 269.125, 269.532, 269.926, 270.286};

        accDistance = Math.max(0.0, accDistance);

        double delta = Math.abs(SPEED[0] - currentSpeed);

        int i = 1;
        for (; i < SPEED.length; ++i) {
            double delta2 = Math.abs(SPEED[i] - currentSpeed);
            if (delta2 < delta) {
                delta = delta2;
            } else {
                break;
            }
        }

        i += (int) Math.floor(accDistance / 10.0);

        if (i >= SPEED.length) {
            //System.out.println("Ui, can't tell...");
            return 275.0;
        } else {
            i = Math.max(0, i);
            return Math.max(SPEED[i], currentSpeed);
        }
    }

    private double calcApproachSpeed(double targetSpeed, double distance, double cc) {
        double c = 2 * 9.81 * 1.1 * cc;

        targetSpeed = targetSpeed / 3.6;

        double result = distance * c + (targetSpeed * targetSpeed);

        result = Math.sqrt(result);

        return result * 3.6;
    }

    private void calcApproachSpeed(PlanStackData planData) {
        int index = planData.currentSegment();
        TrackSegment segment = trackModel.getSegment(index);
        double end = planData.planEnd();

        if (TEXT_DEBUG) {
            println("calcApproachSpeed[" + planData.currentSegment() + "], " + planData.planEnd());
        }

        // search the segment which contains the point planEnd + LOOK_AHEAD
        do {
            index = trackModel.incIdx(index);
            segment = trackModel.getSegment(index);
            end += segment.getLength();

        } while (end - planData.planEnd() < LOOK_AHEAD);

        if (TEXT_DEBUG) {
            println("Lookahead falls into: " + index + ", end: " + end);
        }

        // back propagation of approachSpeed
        planData.approachSpeed = MAX_SPEED;

        while (index != planData.currentSegment()) {
            if (segment.isUnknown()) {
                planData.approachSpeed = 50.0;

            } else if (segment.isStraight()) {
                planData.approachSpeed = Math.min(MAX_SPEED,
                        calcApproachSpeed(planData.approachSpeed, segment.getLength(), 1.0));

            } else {
                // a corner
                TrackSegment.Apex[] apexes = segment.getApexes();
                double[] apexSpeeds = new double[apexes.length];
                for (int i = 0; i < apexes.length; ++i) {
                    apexSpeeds[i] = getTargetSpeed(apexes[i]);
                }

                for (int i = 0; i < apexes.length; ++i) {
                    // init helper data
                    double length;
                    double preSpeed;
                    double nextSpeed;
                    double brakeDistance;

                    if (i == 0) {
                        length = segment.getEnd() - apexes[apexes.length - (i + 1)].position;
                        preSpeed = apexSpeeds[apexSpeeds.length - (i + 1)];
                        nextSpeed = planData.approachSpeed;

                    } else {
                        length = apexes[apexes.length - i].position
                                - apexes[apexes.length - (i + 1)].position;
                        preSpeed = apexSpeeds[apexSpeeds.length - (i + 1)];
                        nextSpeed = apexSpeeds[apexSpeeds.length - i];
                    }

                    brakeDistance = calcBrakeDistance(preSpeed, nextSpeed, brakeCornerCoeff);

                    if (brakeDistance > length) {
                        double saveSpeed = calcApproachSpeed(nextSpeed, length, brakeCornerCoeff);
                        apexSpeeds[apexSpeeds.length - (i + 1)] = saveSpeed;
                    }

                    // if this was the last apex, check / set the approach speed
                    if (i == apexes.length - 1) {
                        length = apexes[apexes.length - (i + 1)].position - segment.getStart();
                        preSpeed = planData.speed();
                        nextSpeed = apexSpeeds[apexSpeeds.length - (i + 1)];

                        brakeDistance = calcBrakeDistance(preSpeed, nextSpeed, brakeCornerCoeff);

                        if (brakeDistance > length) {
                            double saveSpeed = calcApproachSpeed(nextSpeed, length, brakeCornerCoeff);
                            planData.approachSpeed = saveSpeed;

                        } else {
                            planData.approachSpeed = this.calcApproachSpeed(nextSpeed, length, brakeCornerCoeff);
                        }
                    }
                }
            }

            if (TEXT_DEBUG) {
                println("[" + index + "] " + planData.approachSpeed + "km/h");
            }

            index = trackModel.decIdx(index);
            segment = trackModel.getSegment(index);
        }

        if (TEXT_DEBUG) {
            println("Result: " + planData.approachSpeed + "km/h");
        }
    }    

    private static double calcBrakeDistance(double speed, double targetSpeed, double cc) {
        speed = speed / 3.6;
        targetSpeed = targetSpeed / 3.6;

        // 1/2 * erdbeschleunigung * reibungskoeffizient
        double divisor = 2 * 9.81 * 1.1 * cc;

        return ((speed * speed) - (targetSpeed * targetSpeed)) / divisor;
    }

    private static double calcBrakingZone(double length, double currentSpeed, double targetSpeed) {
        return calcBrakingZone(length, currentSpeed, targetSpeed, 1.0);
    }

    private static double calcBrakingZone(double length, double currentSpeed, double targetSpeed, double cc) {
        //System.out.println("Calculate braking zone: length=" + length + ", currentSpeed=" + currentSpeed);
        double result = calcBrakeDistance(predictSpeed(currentSpeed, length), targetSpeed, cc);

        for (int i = 0; i < 2; ++i) {
            //System.out.println(i + " " + result);
            result = calcBrakeDistance(predictSpeed(currentSpeed, length - result), targetSpeed, cc);
        }

        return Math.max(result, 0);
    }

    protected double calcSpeed(double v) {
        double value = Math.abs(v) / 100.0;
        double result = this.targetSpeeds.getMirroredValue(value);
        result *= 330 - 50;
        result += 50;

        return result;
    }

    @Override
    public javax.swing.JComponent[] getComponent() {
        if (DEBUG) {
            if (observer instanceof GraphicDebugable) {
                javax.swing.JComponent[] c = ((GraphicDebugable) observer).getComponent();
                javax.swing.JComponent[] result = new javax.swing.JComponent[c.length + 1];

                System.arraycopy(c, 0, result, 0, c.length);
                result[result.length - 1] = debugPainter;

                return result;

            } else {
                return new javax.swing.JComponent[]{debugPainter};
            }
        } else {
            return new javax.swing.JComponent[0];
        }
    }

    @Override
    public java.awt.geom.Point2D getTargetPosition() {
        double p = plan.get(planIndex).getPosition(nextDistance);

        double y = nextDistance - currentDistance;

        return new java.awt.geom.Point2D.Double(p, y);
    }

    @Override
    public double getTargetSpeed() {
        double speed = plan.get(planIndex).getSpeed(nextDistance);

        if (observer.otherCars() && observer.getRecommendedSpeed() != OpponentObserver.NO_RECOMMENDED_SPEED) {

            speed = Math.min(speed, observer.getRecommendedSpeed());
        }

        return speed;
    }

    public String getInfo() {
        return info.toString() + plan.get(planIndex).getInfo();
    }

    /**
     * Calculates the difference between the current position and the planned
     * target position in meter.
     * @param data Current sensor data.
     * @return Deviation from the planned racing line.
     */
    private double getDeviation(SensorData data) {
        double deviation = 0.0;
        if (!plan.isEmpty() && planIndex < plan.size()
                && trackModel != null && trackModel.initialized()) {
            double tw_half = trackModel.getWidth() * 0.5;
            double planned = plan.get(planIndex).getPosition(nextDistance) * tw_half;
            double current = data.getTrackPosition() * tw_half;
            deviation = Math.abs(current - planned);
        }
        return deviation;
    }

    public void updateOffTrack(SensorData data) {
        if (stage != champ2011client.Controller.Stage.WARMUP || !trackModel.complete() || EA_MODE) {
            return;
        }

        if (onlineLearner == null) {
            onlineLearner = new OnlineLearning2011();
            onlineLearner.init(trackModel, this, data, controller);
        }
        onlineLearner.update(data);
    }

    @Override
    public void update(SensorData data, Situation situation) {
        boolean repaint = false;

        this.currentDistance = data.getDistanceRaced();

        observer.update(data, situation);

        if (DEBUG) {
            repaint = Math.abs(data.getDistanceRaced() - debugPainter.lastPosition) > 10;
        }

        info = new StringBuilder();
        lastData.setData(data);

        nextDistance = data.getDistanceRaced() + Math.max(0.0, (data.getSpeed() / 3.6) / 50);

        // handle online learning
        if (stage == champ2011client.Controller.Stage.WARMUP && trackModel.complete() && !EA_MODE) {
            if (onlineLearner == null) {
                onlineLearner = new OnlineLearning2011();
                onlineLearner.init(trackModel, this, data, controller);
            }
            onlineLearner.update(data);
        }

        if (!this.trackModel.initialized()) {
            plan.clear();
            plan.add(new PlanElement2011(PlanElement2011.Phase.ACCELERATE,
                    data.getDistanceRaced(), data.getDistanceRaced() + 100, "Model not initialized"));
            planIndex = 0;
            info.append("Model not initialized");
            repaint = true;

            if (DEBUG) {
                debugData.clear();
            }

        } else {
            offset = ((trackModel.getWidth() - 4) / 2) / (trackModel.getWidth() / 2);

            while (planIndex < plan.size() && !plan.get(planIndex).contains(nextDistance)) {
                ++planIndex;
            }

            boolean newTargetPoint = lastTargetPoint == OpponentObserver.NO_RECOMMENDED_POINT && observer.getRecommendedPosition() != OpponentObserver.NO_RECOMMENDED_POINT;
            boolean targetPointChanged = lastTargetPoint != OpponentObserver.NO_RECOMMENDED_POINT && observer.getRecommendedPosition() != OpponentObserver.NO_RECOMMENDED_POINT && lastTargetPoint.distanceSq(observer.getRecommendedPosition()) != 0.0;

            boolean reactToOtherCars = observer.otherCars() && (newTargetPoint || targetPointChanged);

            double deviation = getDeviation(data);
            boolean offRaceLine = deviation > MAX_POS_DEVIATION_M
                    && ((stage == champ2011client.Controller.Stage.WARMUP && !trackModel.complete()) || (trackModel.complete() && trackModel.getSegment(data.getDistanceFromStartLine()).isStraight()));

            if (planIndex >= plan.size() || nextDistance > plan.get(plan.size() - 1).getEnd() || reactToOtherCars || offRaceLine) {
                if (TEXT_DEBUG && reactToOtherCars) {
                    println("Need to to replan, because of other cars");
                }

                if (TEXT_DEBUG && offRaceLine) {
                    println("Need to replan because I left the racing line");
                }

                if (TEXT_DEBUG && planIndex >= plan.size()) {
                    println("Need to replan because nextDistance is not covered by the plan");
                }

                plan(data);

                while (planIndex < plan.size() && !plan.get(planIndex).contains(nextDistance)) {
                    ++planIndex;
                    if (TEXT_DEBUG) {
                        println("Adjusting plan index according to the next distance");
                    }
                }

                lastTargetPoint = observer.getRecommendedPosition();
                repaint = true;
                if (DEBUG) {
                    debugData.clear();
                }
            }
        }
        if (DEBUG && repaint) {
            this.debugPainter.repaint();
        }
        if (DEBUG) {
            debugData.add(data);
        }
    }

    private void plan(SensorData data) {
        int index = trackModel.getIndex(data.getDistanceFromStartLine());
        TrackSegment current = trackModel.getSegment(index);

        if (current.isUnknown()) {
            if (TEXT_DEBUG) {
                println("");
                println("*****************************************************");
                println("**                  Within unknown segment         **");
                println("*****************************************************");
            }

            plan.clear();
            planIndex = 0;
            double remaining = current.getLength() - (data.getDistanceFromStartLine() - current.getStart());
            double end = data.getDistanceRaced() + remaining;

            if (TEXT_DEBUG) {
                println("Remaning: " + remaining + "m");
                println("End: " + end + "m");
                println("NextDistance: " + nextDistance);
            }

            if (end <= nextDistance + 10.0) {
                end = nextDistance + 10.0;
                if (TEXT_DEBUG) {
                    println("Too close to nextDistance, moving end to: " + end + "m");
                }
            }

            plan.add(new PlanElement2011(PlanElement2011.Phase.ACCELERATE,
                    data.getDistanceRaced(), end,
                    "Within unknown segment"));

        } else {
            planFast(data);
        }
    }

    private void planFast(SensorData data) {
        int index = trackModel.getIndex(data.getDistanceFromStartLine());
        TrackSegment current = trackModel.getSegment(index);

        if (TEXT_DEBUG) {
            println("");
            println("*****************************************************");
            println("**                CHECKING plan fast               **");
            println("*****************************************************");
            println(current.toString());
        }

        // Clear current plan
        plan.clear();

        // PlanStackData Object to control the planning process
        PlanStackData planData = new PlanStackData(data.getDistanceRaced());

        // the next corner 
        TrackSegment corner = null;

        // PlanElement created during the search process
        PlanElement2011 planElement = null;

        // reference speed from which to accelerate for accDist
        double refSpeed = data.getSpeed();
        // estimated speed at the corner
        double speedAtCorner = data.getSpeed();
        // distance to accelerate
        double accDist = 0.0;

        // add the current element
        accDist += current.getEnd() - data.getDistanceFromStartLine();
        speedAtCorner = predictSpeed(data.getSpeed(), accDist);
        planData.addSegment(index,
                current.getEnd() - data.getDistanceFromStartLine(),
                data.getSpeed());

        // search the next corner
        int searchIndex = trackModel.incrementIndex(index);

        while (corner == null && searchIndex != index) {
            TrackSegment element = trackModel.getSegment(searchIndex);

            planData.addSegment(searchIndex, element.getLength(),
                    speedAtCorner);

            if (element.isCorner()) {
                corner = element;
                planElement = planCorner(planData, data);

            } else if (element.isUnknown()) {
                corner = element;
                planElement = new PlanElement2011(PlanElement2011.Phase.TURN_IN,
                        planData.start(), planData.start(), "Unknown segment");

            } else if (element.isStraight()) {
                accDist += element.getLength();
                speedAtCorner = predictSpeed(refSpeed, accDist);

            }

            searchIndex = trackModel.incrementIndex(searchIndex);
        }

        if (corner == null) {
            if (TEXT_DEBUG) {
                println("corner == null, setze letztes stack element");
            }
            corner = trackModel.getSegment(planData.currentSegment());
        }

        if (TEXT_DEBUG) {
            println("Starting to plan, start=" + planData.planStart() + ", end=" + planData.planEnd());
            println("I'm at " + index + ", planning towards " + planData.currentSegment() + " where I will arrive with " + speedAtCorner + "km/h");

            if (!corner.isUnknown()) {
                println(corner.toString());
                TrackSegment.Apex[] apexes = corner.getApexes();
                for (TrackSegment.Apex a : apexes) {
                    println("Apex: " + a.position + ", " + Situations.toString(a.type) + ", speed: " + Utils.dTS(getTargetSpeed(a)) + "km/h");
                }

            } else {
                println("The corner is an unknown segment.");
            }
        }

        // plan back to front, corner entry
        if (planElement != null) {
            if (TEXT_DEBUG) {
                println("Using the PlanElement created during the search for the last segment");
            }

            plan.add(0, planElement);
            planData.popSegment();
        }

        while (planData.hasMoreSegments()) {
            if (TEXT_DEBUG) {
                println("Planning for tracksegment " + planData.currentSegment());
            }

            current = trackModel.getSegment(planData.currentSegment());

            if (current.isStraight()) {
                planElement = planForStraightMiddleSegmentNew(planData,
                        data);

            } else { // corner
                planElement = planCorner(planData, data);
            }

            plan.add(0, planElement);

            planData.popSegment();
        }

        if (TEXT_DEBUG) {
            println("");
        }

        planIndex = 0;

        if (DEBUG) {
            debugPainter.repaint();
        }
    }

    protected double getTargetSpeed(TrackSegment.Apex a) {
        if (a.unknown) {
            return 50.0;
        }

        if (a.targetSpeed == TrackSegment.DEFAULT_SPEED || EA_MODE) {
            return calcSpeed(a.value) * SPEED_MODIFIER;

        } else {
            return a.targetSpeed * SPEED_MODIFIER;
        }
    }

    private PlanElement2011 planCorner(PlanStackData planData, SensorData data) {
        TrackSegment current = trackModel.getSegment(planData.currentSegment());

        double start = planData.start();
        double end = planData.end();
        double offsetFromStart = 0.0;    // offset within the segment

        TrackSegment.Apex[] apexes = current.getApexes();

        if (TEXT_DEBUG) {
            println("");
            println("----------  CORNER  ---------------");
            println("Start/End: " + Utils.dTS(start) + ", " + Utils.dTS(end) + " [" + Utils.dTS(end - start) + "m]");
            for (int i = 0; i < apexes.length; ++i) {
                TrackSegment.Apex a = apexes[i];
                println("Apex[" + i + "]: " + a.position + ", " + Situations.toString(a.type) + ", speed: " + Utils.dTS(getTargetSpeed(a)) + "km/h");
            }
        }

        // if this is the first segment to plan for, adjust start
        if (planData.first()) {
            offsetFromStart = data.getDistanceFromStartLine() - current.getStart();

            if (offsetFromStart > 0.0) {
                if (TEXT_DEBUG) {
                    println("This is the first segment to plan for and i'm "
                            + offsetFromStart + "m into the segment, adjusting start.");
                }
                start -= offsetFromStart;
                if (TEXT_DEBUG) {
                    println("New Start/End: " + Utils.dTS(start) + ", " + Utils.dTS(end) + " [" + Utils.dTS(end - start) + "m]");
                }
            }
        }

        if (planData.last()) {
            // look ahead to avoid surprisingly slow corners
            calcApproachSpeed(planData);
        }

        double[] apexSpeeds = new double[apexes.length];
        for (int i = 0; i < apexes.length; ++i) {
            apexSpeeds[i] = getTargetSpeed(apexes[i]);
        }

        if (TEXT_DEBUG) {
            planData.print();
            println("");
        }

        ArrayList<Point2D> points = new ArrayList<Point2D>();

        // last point         
        points.add(0, new Point2D.Double(end, planData.approachSpeed));

        for (int i = 0; i < apexes.length; ++i) {
            if (TEXT_DEBUG) {
                println(i + "/" + (apexes.length - 1));
            }
            // init helper data
            double length;
            double preSpeed;
            double nextSpeed;
            double brakeDistance;

            if (i == 0) {
                length = current.getEnd() - apexes[apexes.length - (i + 1)].position;
                preSpeed = apexSpeeds[apexSpeeds.length - (i + 1)];
                nextSpeed = planData.approachSpeed;

            } else {
                length = apexes[apexes.length - i].position
                        - apexes[apexes.length - (i + 1)].position;
                preSpeed = apexSpeeds[apexSpeeds.length - (i + 1)];
                nextSpeed = apexSpeeds[apexSpeeds.length - i];
            }

            brakeDistance = calcBrakeDistance(preSpeed, nextSpeed, brakeCornerCoeff);

            if (TEXT_DEBUG) {
                println("Length: " + length + ", brakeDistance: " + brakeDistance);
            }

            if (brakeDistance >= length) {
                double saveSpeed = calcApproachSpeed(nextSpeed, length, brakeCornerCoeff);

                apexSpeeds[apexSpeeds.length - (i + 1)] = saveSpeed;

                if (TEXT_DEBUG) {
                    println("Save speed: " + Utils.dTS(saveSpeed) + "km/h");
                }

            } else {
                double bz = calcBrakingZone(length, preSpeed, nextSpeed, brakeCornerCoeff);
                double accSpeed = predictSpeed(preSpeed, (length - bz));
                double position = start + (apexes[apexes.length - (i + 1)].position - current.getStart() + (length - bz));

                if (TEXT_DEBUG) {
                    println("Calculated braking zone: " + bz + "m");
                    println("In apex acceleration for " + (length - bz) + "m, " + accSpeed + "km/h");
                    println("Position: " + position + ", last position: " + points.get(0).getX());
                }

                if (bz >= 0.0 && length > bz) {
                    if(bz > 0.0){
                        points.add(0, new Point2D.Double(position, accSpeed));
                    }
                    
                    double smallOffset = (length - bz) * 0.01;
                    position = start + (apexes[apexes.length - (i + 1)].position - current.getStart() + smallOffset);
                    points.add(0, new Point2D.Double(position, accSpeed));
                }
            }

            // apex
            points.add(0, new Point2D.Double(start + (apexes[apexes.length - (i + 1)].position - current.getStart()),
                    apexSpeeds[apexSpeeds.length - (i + 1)]));

            // if this was the last apex, check / set the approach speed
            if (i == apexes.length - 1) {
                length = apexes[apexes.length - (i + 1)].position - current.getStart();
                preSpeed = planData.speed();
                nextSpeed = apexSpeeds[apexSpeeds.length - (i + 1)];

                brakeDistance = calcBrakeDistance(preSpeed, nextSpeed, brakeCornerCoeff);

                if (TEXT_DEBUG) {
                    println("Checking corner entry....");
                    println("Length: " + length + ", brakeDistance: " + brakeDistance);
                }

                if (brakeDistance > length) {
                    double saveSpeed = calcApproachSpeed(nextSpeed, length, brakeCornerCoeff);

                    planData.approachSpeed = saveSpeed;

                    if (TEXT_DEBUG) {
                        println("Save speed: " + Utils.dTS(saveSpeed) + "km/h");
                    }

                } else {
                    planData.approachSpeed = planData.speed();
                }
            }
        }

        // first point, approachSpeed wurde oben in der Schleife gesetzt!
        points.add(0, new Point2D.Double(start, planData.approachSpeed));

        if (TEXT_DEBUG) {
            println("Speeds:");
            for (int i = 0; i < points.size(); ++i) {
                println(points.get(i).getX() + ", " + points.get(i).getY());
            }
            println("-------------------------------");
        }

        LinearInterpolator speed = new LinearInterpolator(points);

        PlanElement2011 element = new PlanElement2011(PlanElement2011.Phase.TURN_IN, start, end,
                "Turn in");
        element.setSpeed(speed);

        return element;
    }

    private PlanElement2011 planStraight(PlanStackData planData, SensorData data) {
        TrackSegment current = trackModel.getSegment(planData.currentSegment());

        double start = planData.start();
        double end = planData.end();
        double offsetFromStart = 0.0;    // offset within the segment        

        if (TEXT_DEBUG) {
            println("");
            println("----------  Straight  ---------------");
            println("Start/End: " + Utils.dTS(start) + ", " + Utils.dTS(end) + " [" + Utils.dTS(end - start) + "m]");
        }

        // if this is the first segment to plan for, adjust start
        if (planData.first()) {
            offsetFromStart = data.getDistanceFromStartLine() - current.getStart();

            if (offsetFromStart > 0.0) {
                if (TEXT_DEBUG) {
                    println("This is the first segment to plan for and i'm "
                            + offsetFromStart + "m into the segment, adjusting start.");
                }
                start -= offsetFromStart;
                if (TEXT_DEBUG) {
                    println("New Start/End: " + Utils.dTS(start) + ", " + Utils.dTS(end) + " [" + Utils.dTS(end - start) + "m]");
                }
            }
        }

        if (planData.last()) {
            // look ahead to avoid surprisingly slow corners
            calcApproachSpeed(planData);
        }

        ArrayList<Point2D> points = new ArrayList<Point2D>();

        if (TEXT_DEBUG) {
            println("Speeds:");
            for (int i = 0; i < points.size(); ++i) {
                println(points.get(i).getX() + ", " + points.get(i).getY());
            }
            println("-------------------------------");
        }

        LinearInterpolator speed = new LinearInterpolator(points);

        PlanElement2011 element = new PlanElement2011(PlanElement2011.Phase.ACCELERATE, start, end,
                "Turn in");
        element.setSpeed(speed);

        return element;
    }

    private PlanElement2011 planForStraightMiddleSegmentNew(PlanStackData planData,
            SensorData data) {
        int index = planData.currentSegment();
        TrackSegment current = trackModel.getSegment(index);
        TrackSegment next = trackModel.getSegment(trackModel.incrementIndex(index));
        TrackSegment prev = trackModel.getSegment(trackModel.decrementIndex(index));

        double end = planData.end();
        double start = planData.start();

        if (TEXT_DEBUG) {
            println("");
            println("------------ planForStraightMiddleSegment New------------");
            println("Start/End: " + Utils.dTS(start) + ", " + Utils.dTS(end) + " [" + Utils.dTS(end - start) + "m]" + " - ");
        }

        if (TEXT_DEBUG) {
            planData.print();
            println("");
        }

        // plan like the last element?
        boolean planLikeLast = current.contains(data.getDistanceFromStartLine());

        if (planLikeLast) {
            start = data.getDistanceRaced();

            if (TEXT_DEBUG) {
                println("This is the last segment to plan for...");
            }
        }


        // check if we can combine the planning of this segment and the previous
        if (!planLikeLast && prev.isStraight()) {
            if (TEXT_DEBUG) {
                println("Previous is also straight, checking further...");
            }
            if (start - prev.getLength() <= data.getDistanceRaced()) {
                if (TEXT_DEBUG) {
                    println("Prev is the last segment to plan for");
                }
                start = data.getDistanceRaced();
                planLikeLast = true;

            } else {
                if (TEXT_DEBUG) {
                    println("Prev is also a middle segment");
                }
                start -= prev.getLength();
                int prevIndex = trackModel.decrementIndex(index);
                int prevPrevIndex = trackModel.decrementIndex(prevIndex);
                if (TEXT_DEBUG) {
                    println("Switching prev from " + prevIndex + " to " + prevPrevIndex);
                }
                prev = trackModel.getSegment(prevPrevIndex);
            }

            if (TEXT_DEBUG) {
                println("Moving start to " + Utils.dTS(start) + ", new length " + Utils.dTS(end - start) + "m");
            }
            planData.popSegment();
        }

        double brakeDistance = 0.0;

        if (planData.approachSpeed != Plan2011.MAX_SPEED) {
            brakeDistance = calcBrakingZone(end - start,
                    planData.speed(), planData.approachSpeed);
        }

        if (TEXT_DEBUG) {
            println("Brake distance: " + Utils.dTS(brakeDistance) + "m");
        }

        Point2D targetPosition = OpponentObserver.NO_RECOMMENDED_POINT;

        if (observer.otherCars() && observer.getRecommendedPosition() != OpponentObserver.NO_RECOMMENDED_POINT) {
            Point2D recommendation = new Point2D.Double();
            recommendation.setLocation(observer.getRecommendedPosition());

            /*if (recommendation.getY() < data.getDistanceRaced() && recommendation.getY() <= trackModel.getLength()) {
                // FIX_ME -> new observer -> allways distanced raced
                if (TEXT_DEBUG) {
                    println("I guess the recommended position is measured in distance from startline, not distance raced, fixing that...");
                }
                double oldY = recommendation.getY();
                double newY = data.getDistanceRaced() - data.getDistanceFromStartLine();
                newY += oldY;
                if (TEXT_DEBUG) {
                    println("Switching from " + Utils.dTS(oldY) + " to " + Utils.dTS(newY));
                }
                recommendation.setLocation(recommendation.getX(), newY);
            }*/

            if (start <= recommendation.getY() && recommendation.getY() < end) {
                targetPosition = new Point2D.Double();
                targetPosition.setLocation(recommendation);
            }
        }

        // track position
        ArrayList<CubicSpline> positions = new ArrayList<CubicSpline>();


        double currStart = start;
        double remainingLength = end - start;
        double currPosition = getAnchorPoint(prev, current);
        if (planLikeLast) {
            currPosition = data.getTrackPosition();
        }

        if (TEXT_DEBUG) {
            println(Utils.dTS(remainingLength) + "m remain...");
        }

        if (targetPosition != OpponentObserver.NO_RECOMMENDED_POINT) {
            if (TEXT_DEBUG) {
                println("I need to take care of other cars, point is " + targetPosition.toString());
                println("Right now, i'm planning with currStart: " + Utils.dTS(currStart));
            }
            if (targetPosition.getY() < currStart) {
                if (TEXT_DEBUG) {
                    println("Point is before currStart, moving it to +1m");
                }
                targetPosition.setLocation(targetPosition.getX(), targetPosition.getY() + 1.0);
            }

            if (TEXT_DEBUG) {
                println("Checking, if there is enough room...");
            }

            double lengthNeeded = targetPosition.getY() - currStart;
            //lengthNeeded += 10.0;
            //lengthNeeded += 150.0;

            if (TEXT_DEBUG) {
                println("I need at least " + Utils.dTS(lengthNeeded) + "m, " + Utils.dTS(remainingLength) + "m remain");
            }

            if (remainingLength < lengthNeeded) {
                targetPosition = OpponentObserver.NO_RECOMMENDED_POINT;
                if (TEXT_DEBUG) {
                    println("Cannot overtake, not enough room");
                }
            }
        }


        if (targetPosition != OpponentObserver.NO_RECOMMENDED_POINT) {
            if (TEXT_DEBUG) {
                println("Trying to overtake");
            }

            // switch Position 1
            double[] xP = new double[3];
            double[] yP = new double[3];

            xP[0] = currStart;
            xP[2] = targetPosition.getY();
            xP[1] = (xP[0] + xP[2]) / 2.0;

            yP[0] = currPosition;
            yP[2] = targetPosition.getX();
            yP[1] = (yP[0] + yP[2]) / 2.0;

            if (TEXT_DEBUG) {
                println("Switch Position 1:");
                for (int k = 0; k < xP.length; ++k) {
                    println(xP[k] + " , " + yP[k]);
                }
            }

            CubicSpline spline = new CubicSpline(xP, yP);
            spline.setDerivLimits(0.0, 0.0);
            positions.add(spline);

            currStart = xP[2];
            remainingLength = end - currStart;
            currPosition = targetPosition.getX();


            // overtaking
            if (TEXT_DEBUG) {
                println("Overtaking line");
            }

            xP = new double[3];
            yP = new double[3];

            xP[0] = currStart;
            xP[2] = end - 150.0;
            xP[1] = (xP[0] + xP[2]) / 2.0;

            yP[0] = targetPosition.getX();
            yP[2] = targetPosition.getX();
            yP[1] = targetPosition.getX();

            if (TEXT_DEBUG) {
                println("Overtaking line:");
                for (int k = 0; k < xP.length; ++k) {
                    println(xP[k] + " , " + yP[k]);
                }
            }

            spline = new CubicSpline(xP, yP);
            spline.setDerivLimits(0.0, 0.0);
            positions.add(spline);

            currStart = xP[2];
            remainingLength = end - currStart;
            currPosition = targetPosition.getX();
        }


        // simply drive towards the target position for the next corner
        if (TEXT_DEBUG) {
            println("Planning towards the next corner...");
        }

        double[] xP = new double[3];
        double[] yP = new double[3];

        xP[0] = currStart;
        xP[2] = end;
        xP[1] = (xP[0] + xP[2]) / 2.0;

        yP[0] = currPosition;
        yP[2] = getAnchorPoint(current, next);
        yP[1] = (yP[0] + yP[2]) / 2.0;

        if (TEXT_DEBUG) {
            println("Position towards the next corner:");
            for (int k = 0; k < xP.length; ++k) {
                println(xP[k] + " , " + yP[k]);
            }
        }

        CubicSpline spline = new CubicSpline(xP, yP);
        spline.setDerivLimits(0.0, 0.0);
        positions.add(spline);

        // target speed
        double[] xS;
        double[] yS;

        if (brakeDistance >= end - start || brakeDistance == 0.0) {
            xS = new double[3];
            yS = new double[3];

            xS[0] = start;
            xS[2] = end;
            xS[1] = (xS[0] + xS[2]) / 2.0;

            if (brakeDistance == 0.0) {
                yS[0] = MAX_SPEED;
                yS[1] = MAX_SPEED;
                yS[2] = MAX_SPEED;
                planData.approachSpeed = MAX_SPEED;

            } else {
                yS[0] = planData.approachSpeed;
                yS[1] = planData.approachSpeed;
                yS[2] = planData.approachSpeed;
            }


        } else {
            xS = new double[4];
            yS = new double[4];            

            xS[0] = start;
            xS[1] = end - brakeDistance;
            xS[2] = end - (brakeDistance * 0.99);
            xS[3] = end;
            
            yS[0] = MAX_SPEED;
            yS[1] = MAX_SPEED;            
            yS[2] = planData.approachSpeed;
            yS[3] = planData.approachSpeed;

            planData.approachSpeed = MAX_SPEED;
        }

        if (TEXT_DEBUG) {
            println("Speed:");
            for (int i = 0; i < xS.length; ++i) {
                println(xS[i] + " , " + yS[i]);
            }
        }


        LinearInterpolator speed = new LinearInterpolator(xS, yS);

        PlanElement2011 element;

        if (brakeDistance == 0.0) {
            element = new PlanElement2011(PlanElement2011.Phase.ACCELERATE, xS[0], xS[xS.length - 1],
                    "Accelerate");
        } else {
            element = new PlanElement2011(PlanElement2011.Phase.BRAKE, xS[0], xS[xS.length - 1],
                    "Brake");
        }

        for (CubicSpline cs : positions) {
            element.attachPosition(new de.janquadflieg.mrracer.functions.FlanaganCubicWrapper(cs));
        }

        element.setSpeed(speed);        

        return element;
    }

    @Override
    public void setTrackModel(TrackModel t) {
        this.trackModel = t;
        observer.setTrackModel(t);
        this.onlineLearner = null;

        /*if(t.complete()){
        System.out.println(t.getName());
        for(int i=0; i < t.size(); ++i){
        TrackSegment seg = t.getSegment(i);
        if(seg.isCorner()){
        System.out.println("");
        TrackSegment.Apex[] apexes = seg.getApexes();
        for(TrackSegment.Apex a: apexes){
        String s = a.value+" default speed: "+getSpeed(a.value)+" stored speed: ";
        if(a.targetSpeed == TrackSegment.DEFAULT_SPEED){
        s += "default";

        } else {
        s += a.targetSpeed;
        }
        System.out.println(s);
        }
        }
        }
        }*/
    }

    private void print(String s) {
        System.out.print(s);
    }

    private void println(String s) {
        System.out.println(s);
    }

    private double getAnchorPoint(TrackSegment first, TrackSegment second) {
        if (first.getDirection() == Situations.DIRECTION_FORWARD && second.getDirection() == Situations.DIRECTION_RIGHT && !second.isFull()) {
            //System.out.println("F / R "+offset);

            return offset;

        } else if (first.getDirection() == Situations.DIRECTION_FORWARD && second.getDirection() == Situations.DIRECTION_LEFT && !second.isFull()) {
            //System.out.println("F / L");

            return -offset;

        } else if (first.getDirection() == Situations.DIRECTION_RIGHT && second.getDirection() == Situations.DIRECTION_FORWARD && !first.isFull()) {
            //System.out.println("R / F");

            return offset;

        } else if (first.getDirection() == Situations.DIRECTION_LEFT && second.getDirection() == Situations.DIRECTION_FORWARD && !first.isFull()) {
            //System.out.println("L / F");

            return -offset;

        } else if (first.getDirection() == Situations.DIRECTION_LEFT && second.getDirection() == Situations.DIRECTION_LEFT) {
            //System.out.println("L / L");

            return -offset;

        } else if (first.getDirection() == Situations.DIRECTION_RIGHT && second.getDirection() == Situations.DIRECTION_RIGHT) {
            //System.out.println("R / R");

            return offset;
        }
        //System.out.println("sonst");
        return 0;
    }

    public void saveOnlineData() {
        //System.out.println("SaveOnlineData");
        //System.out.println("Stage: "+stage);
        //System.out.println("OnlineLearner: "+onlineLearner);
        //System.out.println("EA MODE: "+EA_MODE);
        if (stage == champ2011client.Controller.Stage.WARMUP && onlineLearner != null && !EA_MODE) {
            onlineLearner.saveData();
        }
    }

    private class DebugPainter
            extends javax.swing.JPanel {

        private final Color DARK_GREEN = new Color(0, 129, 36);
        private final Color ORANGE = new Color(255, 154, 23);
        private double lastPosition = 0.0;

        @Override
        public void paintComponent(Graphics graphics) {
            Graphics2D g = (Graphics2D) graphics;
            try {
                paintComponent(g);
            } catch (Exception e) {
                e.printStackTrace(System.out);
            }
        }

        public void paintComponent(Graphics2D g) {
            lastPosition = lastData.getDistanceRaced();
            Dimension size = getSize();

            g.setColor(Color.WHITE);
            g.fillRect(0, 0, getWidth(), getHeight());

            g.setColor(Color.BLACK);

            g.translate(10, 10);

            double totalLength = 0.0;

            for (int i = 0; i < plan.size(); ++i) {
                totalLength += plan.get(i).getLength();
            }

            double ppm = (1.0 * (size.width - 20)) / totalLength;
            int offset = 0;

            for (int i = 0; i < Plan2011.this.plan.size(); ++i) {
                PlanElement2011 e = Plan2011.this.plan.get(i);

                g.translate(offset, 0);

                int width = (int) Math.round(e.getLength() * ppm);

                e.draw(g, new Dimension(width, size.height - 10), ppm, lastData);

                g.translate(-offset, 0);

                offset += width;
            }

            if (Plan2011.this.plan.size() > 0) {
                g.setColor(Color.MAGENTA);
                double start = Plan2011.this.plan.get(0).getStart();
                try {
                    for (SensorData d : debugData) {
                        if (d.getDistanceRaced() >= start) {
                            int x = (int) Math.round((d.getDistanceRaced() - start) * ppm);
                            double v = d.getTrackPosition();
                            v = Math.max(-1.0, Math.min(1.0, v));
                            int y = 100 - (int) Math.round(100 * ((v + 1.0) / 2.0));
                            g.drawRect(x, y, 1, 1);
                        }
                    }
                } catch (java.util.ConcurrentModificationException e) {
                }
            }

            g.translate(-10, -10);
        }
    }
}


/*private PlanElement2011 planStraightFullExit(PlanStackData planData,
            SensorData data) {
        int index = planData.currentSegment();
        TrackSegment current = trackModel.getSegment(index);
        TrackSegment next = trackModel.getSegment(trackModel.incrementIndex(index));


        double start = planData.start();
        double end = planData.end();

        double nextSegTargetSpeed = planData.approachSpeed;

        if (TEXT_DEBUG) {
            println("");
            println("-----------planExit, Straight or Full----------");
            print("Start/End: " + Utils.dTS(start) + ", " + Utils.dTS(end) + " [" + Utils.dTS(end - start) + "] - ");

            if (current.isStraight()) {
                println("Straight");
            } else {
                println("Full");
            }
        }

        double brakeDistance = 0.0;

        if (planData.approachSpeed != Plan2011.MAX_SPEED) {
            brakeDistance = calcBrakingZone(end - start,
                    planData.speed(), planData.approachSpeed);
        }

        if (TEXT_DEBUG) {
            println("Brake distance: " + Utils.dTS(brakeDistance) + "m");
        }

        if (next.isCorner()) {
            TrackSegment.Apex[] apexes = next.getApexes();
            if (getTargetSpeed(apexes[0]) < planData.speed() - 25.0 && current.getEnd() - data.getDistanceFromStartLine() <= 100.0) {
                if (TEXT_DEBUG) {
                    println("Next speed is smaller than current speed");
                }
                nextSegTargetSpeed = getTargetSpeed(apexes[0]);
            }
        }



        // target speed
        double[] xS;
        double[] yS;

        if (brakeDistance >= (end - start) || brakeDistance == 0.0) {
            xS = new double[3];
            yS = new double[3];

            xS[0] = start;
            xS[2] = end;
            xS[1] = (xS[0] + xS[2]) / 2.0;

            if (nextSegTargetSpeed == Plan2011.MAX_SPEED && brakeDistance == 0) {
                yS[0] = Plan2011.MAX_SPEED;
                yS[1] = Plan2011.MAX_SPEED;
                yS[2] = Plan2011.MAX_SPEED;

            } else if (brakeDistance == 0.0) {
                yS[0] = planData.approachSpeed;
                yS[1] = planData.approachSpeed;
                yS[2] = planData.approachSpeed;

            } else {
                yS[0] = planData.approachSpeed;
                yS[1] = planData.approachSpeed;
                yS[2] = planData.approachSpeed;
            }


        } else {
            xS = new double[4];
            yS = new double[4];

            xS[0] = start;
            xS[3] = end;
            xS[1] = end - brakeDistance;
            xS[2] = (xS[1] + xS[3]) / 2.0;

            yS[0] = MAX_SPEED;
            yS[3] = planData.approachSpeed;
            yS[1] = MAX_SPEED - 30;
            yS[2] = (yS[1] + yS[3]) / 2.0;
        }

        if (TEXT_DEBUG) {
            println("Speed:");
            for (int i = 0; i < xS.length; ++i) {
                println(xS[i] + " , " + yS[i]);
            }
        }

        LinearInterpolator speed = new LinearInterpolator(xS, yS);

        PlanElement2011 element;

        if (brakeDistance == 0.0) {
            element = new PlanElement2011(PlanElement2011.Phase.ACCELERATE, xS[0], xS[xS.length - 1],
                    "Accelerate");
        } else {
            element = new PlanElement2011(PlanElement2011.Phase.BRAKE, xS[0], xS[xS.length - 1],
                    "Brake");
        }

        element.setSpeed(speed);

        return element;
    }*/
