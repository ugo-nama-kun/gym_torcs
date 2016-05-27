/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.classification;

import flanagan.analysis.Regression;

import java.awt.geom.Point2D;
import java.util.*;
import javax.vecmath.Vector2d;

import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.telemetry.ModifiableSensorData;
import de.janquadflieg.mrracer.telemetry.SensorData;

/**
 *
 * @author quad
 */
public class AngleClassifierWithQuadraticRegression {

    private float[] angles = new float[19];
    public final static double STRAIGHT_FULL = 5.0;
    public final static double FULL_MEDIUM = 20.0;
    public final static double MEDIUM_SLOW = 35.0;
    public final static double SLOW_HAIRPIN = 60.0;        

    private ArrayList<double[]> buffer = new ArrayList<double[]>();


    public AngleClassifierWithQuadraticRegression() {
        // init angles
        for (int i = 0; i < 19; ++i) {
            angles[i] = -90 + i * 10;
        }
    }

    public Situation classify(SensorData data) {
        if (data.getTrackPosition() > 1.0 || data.getTrackPosition() < -1.0) {
            int side = Situations.DIRECTION_LEFT;

            if (data.getTrackPosition() < -1.0) {
                side = Situations.DIRECTION_RIGHT;
            }

            buffer.clear();

            return new Situation(Situations.ERROR_NONE | Situations.TYPE_OUTSIDE | side, 0, 0);
        }

        if (Math.toDegrees(data.getAngleToTrackAxis()) < -80
                || Math.toDegrees(data.getAngleToTrackAxis()) > 80) {

            buffer.clear();

            return new Situation(Situations.ERROR_UNABLE_TO_CLASSIFY, 0, 0);
        }

        double measure = measure2(data);

        buffer.add(new double[]{data.getDistanceRaced(), measure});
        while(buffer.size() > 10 || (data.getDistanceRaced()-buffer.get(0)[0] > 3.0)){
            buffer.remove(0);
        }

        double value = 0.0;
        for(int i=0; i < buffer.size(); ++i){
            value += buffer.get(i)[1];
        }
        value /= buffer.size();

        //value = measure;

        //System.out.print(data.getDistanceFromStartLineS()+" "+Utils.dTS(value));

        double absv = Math.abs(value);
        double sign = Math.signum(value);

        int direction = Situations.DIRECTION_FORWARD;
        int type = Situations.TYPE_FULL;

        if (absv <= STRAIGHT_FULL) {
            //System.out.println(" Straight");
            double dtc = biggestSensorValue(data);
            if (dtc < 90) {
                return new Situation(Situations.STRAIGHT_AC, value, dtc);

            } else {
                return new Situation(Situations.STRAIGHT, value, 0);
            }

        } else {
            // some kind of type
            if (sign == -1) {
               // System.out.println(" Right");
                direction = Situations.DIRECTION_RIGHT;
            } else {
               // System.out.println(" Left");
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

    public double measure2(SensorData data) {
        // Calculate coordinates
        Point2D[] points = SensorData.calculateTrackPoints(data);

        // Find the indeces of the biggest sensor value
        int leftIndex = SensorData.maxTrackIndexLeft(data);
        int rightIndex = SensorData.maxTrackIndexRight(data);

        if (leftIndex > 2) {
            double[] x = new double[leftIndex];
            double[] y = new double[leftIndex];

            for (int i = 0; i < x.length; ++i) {
                x[i] = points[i].getY();
                y[i] = points[i].getX();
            }

            Regression reg = new Regression(x, y);

            try {
                reg.polynomial(2);
                double[] values = reg.getYcalc();                

                for (int i = 0; i < values.length; ++i) {
                    points[i].setLocation(values[i], points[i].getY());
                }
                
            } catch (IllegalArgumentException e) {
                //e.printStackTrace(System.out);
                leftIndex = 0;
            }
        }

        if (rightIndex < points.length - 3) {
            double[] x = new double[points.length - (rightIndex + 1)];
            double[] y = new double[points.length - (rightIndex + 1)];

            for (int i = 0; i < x.length; ++i) {
                x[i] = points[points.length - (1 + i)].getY();
                y[i] = points[points.length - (1 + i)].getX();
            }

            Regression reg = new Regression(x, y);
            try {
                reg.polynomial(2);
                double[] values = reg.getYcalc();

                for (int i = 0; i < values.length; ++i) {
                    points[points.length - (1 + i)].setLocation(values[i], points[points.length - (1 + i)].getY());
                }
                
            } catch (IllegalArgumentException e) {
                //e.printStackTrace(System.out);
                rightIndex = points.length;
            }
        }

        return calculateAngle(points, leftIndex, rightIndex);
    }

    public double measure2b(SensorData d) {
        ModifiableSensorData data = new ModifiableSensorData();
        data.setData(d);

        double[] track = d.getTrackEdgeSensors();

        // Find the indeces of the biggest sensor value
        int leftIndex = SensorData.maxTrackIndexLeft(data);
        int rightIndex = SensorData.maxTrackIndexRight(data);
        
        if (leftIndex > 2) {
            double[] x = new double[leftIndex];
            double[] y = new double[leftIndex];

            for (int i = 0; i < x.length; ++i) {
                x[i] = i * 1.0;
                y[i] = track[i];
            }

            Regression reg = new Regression(x, y);

            try {
                reg.polynomial(2);
                double[] values = reg.getYcalc();

                for (int i = 0; i < values.length; ++i) {
                    track[i] = values[i];
                }

            } catch (IllegalArgumentException e) {
                leftIndex = 0;
            }
        }

        if (rightIndex < track.length - 3) {
            double[] x = new double[track.length - (rightIndex + 1)];
            double[] y = new double[track.length - (rightIndex + 1)];

            for (int i = 0; i < x.length; ++i) {
                x[i] = i*1.0;
                y[i] = track[track.length - (1 + i)];
            }

            Regression reg = new Regression(x, y);
            try {
                reg.polynomial(2);
                double[] values = reg.getYcalc();

                for (int i = 0; i < values.length; ++i) {
                    track[i] = values[i];                    
                }

            } catch (IllegalArgumentException e) {
                rightIndex = track.length;
            }
        }

        data.setTrackEdgeSensors(track);

        // Calculate coordinates
        Point2D[] points = SensorData.calculateTrackPoints(data);        

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

    public void reset(){
        buffer.clear();
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
