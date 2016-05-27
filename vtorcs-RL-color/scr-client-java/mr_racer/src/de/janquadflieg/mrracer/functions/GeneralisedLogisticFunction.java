/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.functions;

import java.util.Properties;

/**
 *
 * @author Jan Quadflieg
 */
public class GeneralisedLogisticFunction {

    /** Lower asymptote. */
    private double a = 0;
    /** Identifier for the lower asymptote. */
    public static final String LOWER_ASYMPTOTE_A = "-GLF.A-";
    /** Upper asymptote. */
    private double k = 1;
    /** Identifier for the Upper asymptote. */
    public static final String UPPER_ASYMPTOTE_K = "-GLF.K-";
    /** Growth rate. */
    private double b = 10;
    /** Identifier for the growth rate. */
    public static final String GROWTH_RATE_B = "-GLF.B-";
    /** v. */
    private double v = 1;
    /** Identifier for v. */
    public static final String V = "-GLF.V-";
    /** The position of maximum growth M. */
    private double m = 0.5;
    /** Identifier for v. */
    public static final String M = "-GLF.M-";
    /** Q. */
    private double q = 1;
    public static final String Q = "-GLF.Q-";

    public void setParameters(Properties params, String prefix) {        
        if (params.containsKey(prefix+GROWTH_RATE_B)) {
            this.b = Double.parseDouble(params.getProperty(prefix+GROWTH_RATE_B));
            //System.out.println("Setting ["+prefix+GROWTH_RATE_B+"] to "+b);
        }

        if (params.containsKey(prefix+LOWER_ASYMPTOTE_A)) {
            this.a = Double.parseDouble(params.getProperty(prefix+LOWER_ASYMPTOTE_A));
            //System.out.println("Setting ["+prefix+LOWER_ASYMPTOTE_A+"] to "+a);
        }

        if (params.containsKey(prefix+UPPER_ASYMPTOTE_K)) {
            this.k = Double.parseDouble(params.getProperty(prefix+UPPER_ASYMPTOTE_K));
            //System.out.println("Setting ["+prefix+UPPER_ASYMPTOTE_K+"] to "+k);
        }

        if(params.containsKey(prefix+M)){
            this.m = Double.parseDouble(params.getProperty(prefix+M));
            //System.out.println("Setting ["+prefix+M+"] to "+m);
        }

        if(params.containsKey(prefix+V)){
            this.v = Double.parseDouble(params.getProperty(prefix+V));
            //System.out.println("Setting ["+prefix+V+"] to "+v);
        }

        if(params.containsKey(prefix+Q)){
            this.q = Double.parseDouble(params.getProperty(prefix+Q));
            //System.out.println("Setting ["+prefix+Q+"] to "+q);
        }
    }

    public void getParameters(Properties params, String prefix){
        params.setProperty(prefix+GROWTH_RATE_B, String.valueOf(b));
        params.setProperty(prefix+LOWER_ASYMPTOTE_A, String.valueOf(a));
        params.setProperty(prefix+UPPER_ASYMPTOTE_K, String.valueOf(k));
        params.setProperty(prefix+M, String.valueOf(m));
        params.setProperty(prefix+V, String.valueOf(v));
        params.setProperty(prefix+Q, String.valueOf(q));        
    }

    private double compute(double d){
        double d1 = k - a;

        double d2 = Math.pow(1 + (q * Math.exp(-b*(d-m))), (1.0 / v));

        return d1 / d2;
    }

    public double getValue(double d){
        return a + compute(d);
    }

    public double getMirroredValue(double d){
        return k - compute(d);
    }

    public static void main(String args[]){
        GeneralisedLogisticFunction f = new GeneralisedLogisticFunction();
        for(int i=0; i < 110; i=i+5){
            System.out.println(f.getMirroredValue(i*0.01));
        }
    }
}
