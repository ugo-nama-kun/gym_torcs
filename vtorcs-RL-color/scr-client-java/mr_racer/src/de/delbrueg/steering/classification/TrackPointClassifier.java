/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.classification;

import de.janquadflieg.mrracer.telemetry.SensorData;
import java.awt.geom.Point2D;

import flanagan.analysis.Regression;

/**
 * Fills a TrackPointSituation with content from SensorData
 * @author Tim
 */
public class TrackPointClassifier {

    // the distance how far to look ahead
    private double distance;

    // only track points that have a higher distance than this threshold will be counted
    private double threshold;

    private float[] angles = new float[19];

    private boolean noisy = false;

    public TrackPointClassifier( float[] f, boolean b){
        System.arraycopy(f, 0, angles, 0, f.length);
        this.noisy = b;
    }

    /*
     * the maximum distance up to which the sensors will be used
     */
    public void setDistance(double distance) {
        this.distance = distance;
    }

    /**
     * this is the minimum distance between two sensor points to be recognised as 2 different points
     * @param threshold
     */
    public void setThreshold(double threshold) {
        this.threshold = threshold;
    }



    public TrackPointSituation classify( SensorData data ){

        TrackPointSituation sit = new TrackPointSituation();

        // Calculate coordinates
        Point2D[] points = SensorData.calculateTrackPointsWithCarDir(data, angles);
//        Point2D[] points = SensorData.calculateTrackPoints(data, angles);

        // Find the indices of the biggest sensor value
        int leftIndex = SensorData.maxTrackIndexLeft(data);
        int rightIndex = SensorData.maxTrackIndexRight(data);

        // begin jq
        if (leftIndex > 2 && noisy) {
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
                leftIndex = 0;
            }
        }

        if (rightIndex < points.length - 3 && noisy) {
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
                rightIndex = points.length;
            }
        }
        // end jq

        // first, copy the left points out of the array
        Point2D last = null;
        for( int i=0; i < leftIndex; i++ ){
            if( points[i].getY() <= distance )
            {
                if( last==null || last.distance(points[i]) > threshold ){
                    sit.getLeftTrackPoints().add(points[i]);
                    last = points[i];
                }
            }
        }

        // now the right ones
        last = null;
        for( int i=rightIndex+1; i < points.length; i++ ){
            if( points[i].getY() <= distance )
            {
                if( last==null || last.distance(points[i]) > threshold ){
                    sit.getRightTrackPoints().add(points[i]);
                    last = points[i];
                }
            }
        }

//        System.out.println("left: " + Utils.dTS(sit.getLeftTrackPoints().size())
//                + " right: " + Utils.dTS(sit.getRightTrackPoints().size()));

        return sit;
    }
}
