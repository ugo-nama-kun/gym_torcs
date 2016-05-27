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
public class SinusPlan implements PlanInterface{

    private SensorData data = null;
    
    private double offset = 0;
    
    private final double period;
    private final double width;
    private final double speed;

    public SinusPlan(double period, double speed, double width) {
        this.period = period;
        this.speed = speed;
        this.width = width;
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
        if( data == null){
            // calc offset
            offset = Math.asin(data.getTrackPosition());
        }
        else{
            this.data = data;
        }

    }

    @Override
    public Point2D getTargetPosition() {
        if(data == null)
            return new Point2D.Double(0,0);
        else{
            double p = width*Math.sin(offset + period*data.getDistanceRaced());
            return new Point2D.Double(p,100);
        }
    }

    @Override
    public double getTargetSpeed() {
        return speed;
    }

}
