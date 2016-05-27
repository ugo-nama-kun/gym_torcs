/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.plan;

import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.telemetry.SensorData;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics2D;

import java.util.ArrayList;

import de.janquadflieg.mrracer.functions.Interpolator;

/**
 *
 * @author Jan Quadflieg
 */
public class PlanElement2011 {

    private static final Color DARK_GREEN = new Color(0, 129, 36);

    public enum Phase {

        ACCELERATE, BRAKE, POSITION, TURN_IN
    }
    private double start, end;
    private ArrayList<Interpolator> positions = new ArrayList<Interpolator>();
    private LinearInterpolator speed;
    private String info = "";
    private Phase type;

    public PlanElement2011(Phase t, double start, double end, String info) {
        this.type = t;
        this.start = start;
        this.end = end;
        this.info = info;
    }

    public void draw(Graphics2D g, Dimension size, double ppm, SensorData data) {
        if (type == Phase.ACCELERATE) {
            g.setColor(DARK_GREEN);

        } else if (type == Phase.POSITION) {
            g.setColor(Color.MAGENTA);

        } else if (type == Phase.BRAKE) {
            g.setColor(Color.RED);

        } else {
            g.setColor(Color.GREEN);
        }

        g.fillRect(0, 0, size.width, 100);
        g.fillRect(0, 120, size.width, 100);


        g.setColor(Color.WHITE);
        g.fillRect(4, 124, size.width - 8, 92);
        g.fillRect(4, 4, size.width - 8, 92);

        double minSpeed = Double.MAX_VALUE;
        int minSpeedX = 0;

        for (int x = 0; x < size.width; ++x) {
            double mpp = getLength() / (size.width * 1.0);
            double d = getStart() + (mpp * x);
            double v = getSpeed(d);

            if(v < minSpeed){
                minSpeedX = x;
                minSpeed = v;
            }

            g.setColor(Color.GREEN);
            int y = 120 + 100 - (int) Math.round(100 * (v / 350.0));
            g.drawRect(x, y, 1, 1);

            v = getPosition(d);
            v = Math.max(-1.0, Math.min(1.0, v));
            y = 100 - (int) Math.round(100 * ((v + 1.0) / 2.0));
            g.drawRect(x, y, 1, 1);
        }

        g.setColor(Color.BLACK);
        g.drawString(Utils.dTS(minSpeed), minSpeedX, 120 + 100 - (int) Math.round(100 * (minSpeed / 350.0)));

        if (contains(data.getDistanceRaced())) {
            g.setColor(Color.MAGENTA);
            int x = (int) Math.round((data.getDistanceRaced() - getStart()) * ppm);
            g.drawLine(x, 0, x, 100);
            g.drawLine(x, 120, x, 220);

        }
    }

    public Phase getPhase() {
        return type;
    }

    public String getInfo() {
        return info;
    }

    public boolean contains(double d) {
        return (d >= start && d <= end);
    }

    public double getStart() {
        return start;
    }

    public double getEnd() {
        return end;
    }

    public double getLength() {
        return getEnd() - getStart();
    }

    protected void attachPosition(Interpolator spline) {
        this.positions.add(spline);
    }

    protected void setPosition(Interpolator spline) {
        if (spline.getXmin() != start || spline.getXmax() != end) {
            System.out.println(spline.getXmin() + "," + this.start);
            System.out.println(spline.getXmax() + "," + this.end);
            throw new RuntimeException("Position spline limits don't match this plan element.");
        }
        this.positions.clear();
        this.positions.add(spline);
    }

    protected void setSpeed(LinearInterpolator spline) {
        if (spline.getXmin() != start || spline.getXmax() != end) {
            System.out.println(spline.getXmin() + "," + this.start);
            System.out.println(spline.getXmax() + "," + this.end);
            throw new RuntimeException("Speed spline limits don't match this plan element.");
        }
        this.speed = spline;
    }

    public double getPosition(double d) {
        if (positions.isEmpty()) {
            return 0.0;
        } else {
            for (Interpolator position : positions) {
                if (position.getXmin() <= d && position.getXmax() >= d) {
                    return position.interpolate(d);
                }
            }
        }
        StringBuilder b = new StringBuilder();
        b.append("Unable to interpolate for position: ");
        b.append(d);
        b.append("\n");
        for (Interpolator position : positions) {
            b.append("Interpolator[").append(position.getClass().getName()).append("]: [");
            b.append(position.getXmin());
            b.append(", ");
            b.append(position.getXmax());
            b.append("]\n");
        }
        throw new RuntimeException(b.toString());
    }

    public double getSpeed(double d) {
        if (speed == null) {
            return 50.0;
        }
        return speed.interpolate(d);
    }

    @Override
    public String toString() {
        return "PlanElement " + type + " " + Utils.dTS(getStart())
                + "-" + Utils.dTS(getPosition(getStart())) + " : "
                + Utils.dTS(getEnd())
                + "-" + Utils.dTS(getPosition(getEnd()));
    }

    public static void main(String[] args){
        double[] x = {0, 1, 2};
        double[] y = {30, 30, 30};
        //CubicSpline spline = new CubicSpline(x, y);
        /*for(double i=0.1; i <= 2.0; i=i+0.1){
            System.out.println(i+" "+spline.interpolate(i));
        }*/
    }
}
