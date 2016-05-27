/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.experiment;

import de.janquadflieg.mrracer.classification.Situation;
import de.janquadflieg.mrracer.telemetry.SensorData;
import de.janquadflieg.mrracer.track.TrackModel;
import java.awt.geom.Point2D;

/**
 *
 * @author Tim
 */
public class RandomPlan implements PlanInterface {

    private SensorData data = null;

    private final double frequency;
    private final double window_width;
    private final double speed;
    private final double timestep;
    private final double max_angle = Math.PI/2;
    private double last_target = 0.0;

    /**
     *
     * @param variation the maximum change in direction (radian angle)
     * @param speed
     * @param width
     */
    public RandomPlan(double variation, double speed, double window_width, double timestep) {
        this.frequency = variation;
        this.speed = speed;
        this.window_width = window_width;
        this.timestep = timestep;
    }

    @Override
    public void reset() {

    }

    @Override
    public void setTrackModel(TrackModel t) {
        // not needed
    }

    @Override
    public void update(SensorData data, Situation s) {
        this.data = data;
    }

    @Override
    public Point2D getTargetPosition() {

        if(data == null)
            return new Point2D.Double(0,100);

        if( Math.random() < frequency )
        {

            // calc random number window
            double win_min = Math.max(-0.8, data.getTrackPosition()-window_width);
            double win_max = Math.min(0.8, data.getTrackPosition()+window_width);

            // get random number in [0,1]
            double r = Math.random();

            double new_pos = r*(win_max-win_min)+win_min;


            last_target = new_pos;

        
            return new Point2D.Double(new_pos,100);
        }
        else {
            return new Point2D.Double(last_target,100);
        }
    }

    @Override
    public double getTargetSpeed() {
        return speed;
    }



}
