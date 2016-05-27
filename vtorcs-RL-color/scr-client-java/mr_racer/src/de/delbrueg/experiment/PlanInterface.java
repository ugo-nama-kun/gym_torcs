/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.experiment;

import de.janquadflieg.mrracer.classification.Situation;
import de.janquadflieg.mrracer.telemetry.SensorData;
import de.janquadflieg.mrracer.track.TrackModel;

/**
 * Introduced to implement custom plans for experiments.
 * @author Tim
 */
public interface PlanInterface {
    
    public void reset();
    public void setTrackModel(TrackModel t);

    public void update(SensorData data, Situation s);

    public java.awt.geom.Point2D getTargetPosition();
    public double getTargetSpeed();
}
