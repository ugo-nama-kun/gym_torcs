/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.plan;

import java.awt.geom.Point2D;
import java.util.ArrayList;

/**
 *
 * @author quad
 */
public class LinearInterpolator
implements de.janquadflieg.mrracer.functions.Interpolator{
    private double[] x_values;
    private double[] y_values;

    public LinearInterpolator(ArrayList<Point2D> points){
        for(int i = 1; i< points.size(); ++i){
            if(points.get(i).getX() <= points.get(i-1).getX()){
                throw new RuntimeException("x["+i+"] "+points.get(i).getX()+" is less or equal x["+
                        (i-1)+"] "+points.get(i-1).getX());
            }
        }

        x_values = new double[points.size()];
        y_values = new double[points.size()];

        for(int i = 0; i< points.size(); ++i){
            x_values[i] = points.get(i).getX();
            y_values[i] = points.get(i).getY();
        }
    }

    public LinearInterpolator(double[] x, double[] y){
        if(x.length != y.length){
            throw new RuntimeException("Number of data points doesn't match the" +
                    "number of function values");
        }
        for(int i = 1; i< x.length; ++i){
            if(x[i] <= x[i-1]){
                throw new RuntimeException("x["+i+"] "+x[i]+" is less or equal x["+
                        (i-1)+"] "+x[i-1]);
            }
        }

        this.x_values = new double[x.length];
        System.arraycopy(x, 0, this.x_values, 0, x.length);
        this.y_values = new double[y.length];
        System.arraycopy(y, 0, this.y_values, 0, y.length);
    }

    @Override
    public double getXmin(){
        return x_values[0];
    }

    @Override
    public double getXmax(){
        return x_values[x_values.length-1];
    }

    @Override
    public double interpolate(double x){
        if(x < x_values[0] || x > x_values[x_values.length-1]){
            throw new RuntimeException("Requested value "+x+" lies outside the data " +
                    "points of this interpolator");
        }
        int lower = 0;
        while(x_values[lower+1] < x){
            ++lower;
        }
        int upper = lower+1;

        double alpha = (x-x_values[lower]) / (x_values[upper]-x_values[lower]);

        double result = (1.0 - alpha) * y_values[lower] + alpha * y_values[upper];

        return result;
    }

    public static void main(String[] args){
        double[] x = {0.0, 10.0, 20.0, 30.0};
        double[] y = {0.0, 1.0, 1.0, 0.0};

        LinearInterpolator lp = new LinearInterpolator(x, y);

        double v = 0.0;

        for(int i=0; i <=30; ++i){
            v = 1.0*i;
            System.out.println(v+" "+lp.interpolate(v));
        }
    }
}
