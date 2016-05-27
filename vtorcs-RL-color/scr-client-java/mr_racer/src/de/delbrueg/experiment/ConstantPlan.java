package de.delbrueg.experiment;

import de.janquadflieg.mrracer.classification.Situation;
import de.janquadflieg.mrracer.telemetry.SensorData;
import de.janquadflieg.mrracer.track.TrackModel;
import java.awt.geom.Point2D;

/**
 *
 * @author Tim
 */
public class ConstantPlan implements PlanInterface {

    private final double pos;
    private final double speed;

    private double curr_pos;
    private final double pos_change_per_tick;

    public ConstantPlan(double speed, double pos, double pos_change_per_tick) {
        this.pos = pos;
        this.speed = speed;
        this.pos_change_per_tick = pos_change_per_tick;
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
        curr_pos = data.getTrackPosition();
    }

    @Override
    public Point2D getTargetPosition() {
        double change = (pos-curr_pos);
        change = Math.signum(change) * Math.min(Math.abs(change), pos_change_per_tick);
        return new Point2D.Double(curr_pos + change,100);
    }

    @Override
    public double getTargetSpeed() {
        return speed;
    }



}
