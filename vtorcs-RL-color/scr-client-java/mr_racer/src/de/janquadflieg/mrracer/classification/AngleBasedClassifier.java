/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.classification;

import de.janquadflieg.mrracer.telemetry.SensorData;

import java.awt.geom.Point2D;
import javax.vecmath.Vector2d;

/**
 *
 * @author Jan Quadflieg
 */
public class AngleBasedClassifier {

    private boolean advanced = false;
    private float[] angles = new float[19];

    public final static double STRAIGHT_FULL = 3.0;
    public final static double FULL_MEDIUM = 20.0;
    public final static double MEDIUM_SLOW = 35.0;
    public final static double SLOW_HAIRPIN = 60.0;

    private int maxNum = Integer.MIN_VALUE;

    public AngleBasedClassifier(int n){
        advanced = true;

        // init angles
        for (int i = 0; i < 19; ++i) {
            angles[i] = -90 + i * 10;
        }

        maxNum = n;
    }

    public AngleBasedClassifier(boolean b) {
        advanced = b;

        // init angles
        for (int i = 0; i < 19; ++i) {
            angles[i] = -90 + i * 10;
        }
    }

    public AngleBasedClassifier(float[] f) {
        advanced = true;
        System.arraycopy(f, 0, angles, 0, f.length);
    }

    public Situation classify(SensorData data) {
        if (data.getTrackPosition() > 1.0 || data.getTrackPosition() < -1.0) {
            int side = Situations.DIRECTION_LEFT;

            if (data.getTrackPosition() < -1.0) {
                side = Situations.DIRECTION_RIGHT;
            }

            return new Situation(Situations.ERROR_NONE | Situations.TYPE_OUTSIDE | side, 0, 0);
        }

        if (Math.toDegrees(data.getAngleToTrackAxis()) < -80
                || Math.toDegrees(data.getAngleToTrackAxis()) > 80) {

            return new Situation(Situations.ERROR_UNABLE_TO_CLASSIFY, 0, 0);
        }

        double value;
        if (advanced) {
            value = measure2(data);
        } else {
            value = measure(data);
        }
        double absv = Math.abs(value);
        double sign = Math.signum(value);

        int direction = Situations.DIRECTION_FORWARD;
        int type = Situations.TYPE_FULL;

        if (value >= -3.0 && value <= 3.0) {
            double dtc = biggestSensorValue(data);
            if (dtc < 90) {
                return new Situation(Situations.STRAIGHT_AC, value, dtc);

            } else {
                return new Situation(Situations.STRAIGHT, value, 0);
            }

        } else {
            // some kind of type
            if (sign == -1) {
                direction = Situations.DIRECTION_RIGHT;
            } else {
                direction = Situations.DIRECTION_LEFT;
            }


            if (absv > STRAIGHT_FULL && absv <= FULL_MEDIUM) {
                type = Situations.TYPE_FULL;

            } else if (absv > FULL_MEDIUM && absv <= MEDIUM_SLOW) {
                type = Situations.TYPE_MEDIUM;

            } else if (absv > MEDIUM_SLOW && absv <= SLOW_HAIRPIN) {
                type = Situations.TYPE_SLOW;

            } else {
                type = Situations.TYPE_HAIRPIN;
            }

            return new Situation(Situations.ERROR_NONE | direction | type, value, 0);
        }
    }

    public double measure(SensorData data) {
        // Calculate coordinates
        Point2D[] points = SensorData.calculateTrackPoints(data, angles);

        // Find the index of the biggest sensor value
        int index = SensorData.maxTrackIndexLeft(data);

        return calculateAngle(points, index, index);
    }

    public double measure2(SensorData data) {
        // Calculate coordinates
        Point2D[] points = SensorData.calculateTrackPoints(data);

        // Find the indeces of the biggest sensor value
        int leftIndex = SensorData.maxTrackIndexLeft(data);
        int rightIndex = SensorData.maxTrackIndexRight(data);

        if(this.maxNum != Integer.MIN_VALUE){
            leftIndex = Math.min(leftIndex, maxNum+2);
            rightIndex = Math.max(rightIndex, 18-(maxNum+2));
        }

        return calculateAngle(points, leftIndex, rightIndex);
    }

    private static double calculateAngle(Point2D[] points, int leftIndex, int rightIndex) {
        double angleLeft = 0;
        if (leftIndex > 2) {
            Vector2d[] vectors = new Vector2d[leftIndex - 1];
            for (int i = 0; i < vectors.length; ++i) {
                Point2D first = points[i];
                Point2D second = points[i + 1];

                vectors[i] = new Vector2d(second.getX() - first.getX(),
                        second.getY() - first.getY());
                vectors[i].normalize();
            }

            //System.out.print("Left: ");
            angleLeft = AngleBasedClassifier.sumAngles(vectors);
            //System.out.println("");
        }

        double angleRight = 0;
        if (rightIndex < points.length - 3) {
            Vector2d[] vectors = new Vector2d[points.length - (rightIndex + 2)];
            for (int i = 0; i < vectors.length; ++i) {
                Point2D first = points[points.length - (1 + i)];
                Point2D second = points[points.length - (2 + i)];

                vectors[i] = new Vector2d(second.getX() - first.getX(),
                        second.getY() - first.getY());
                vectors[i].normalize();
            }

            //System.out.print("Right: ");
            angleRight = AngleBasedClassifier.sumAngles(vectors);
            //System.out.println("");
        }

        return angleLeft + angleRight;
    }

    private double biggestSensorValue(SensorData data) {
        double max = Double.NEGATIVE_INFINITY;

        double[] trackSensors = data.getTrackEdgeSensors();
        for (int i = 0; i < trackSensors.length; ++i) {
            max = Math.max(max, trackSensors[i]);
        }

        return max;
    }

    public static double sumAngles(Vector2d[] vectors) {
        double result = 0;

        for (int i = 0; i < vectors.length - 1; ++i) {
            Vector2d first = vectors[i];
            Vector2d second = vectors[i + 1];

            double angleD = Math.toDegrees(first.angle(second));

            double crossZ = (first.getX() * second.getY())
                    - (first.getY() * second.getX());

            double sign = Math.signum(crossZ);
            angleD *= sign;

            //System.out.println("Angle[i]: "+angleD);

            //System.out.print(Utils.doubleToString(angleD)+", ");

            result += angleD;
        }

        return result;
    }
}
