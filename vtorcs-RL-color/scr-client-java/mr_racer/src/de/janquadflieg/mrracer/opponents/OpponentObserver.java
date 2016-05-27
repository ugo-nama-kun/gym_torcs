/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.opponents;

import champ2011client.Controller.Stage;

import de.janquadflieg.mrracer.classification.Situation;
import de.janquadflieg.mrracer.telemetry.SensorData;
import de.janquadflieg.mrracer.track.TrackModel;

/**
 *
 * @author quad
 */
public interface OpponentObserver {

    public static final java.awt.geom.Point2D NO_RECOMMENDED_POINT = null;

    public static final double NO_RECOMMENDED_SPEED = -1.0;

    public void update(SensorData data, Situation s);

    /**
     * Returns the recommended position on the track to avoid other cars. The
     * x-coordinate corresponds to the position on the track and the y-coordinate
     * to the distance between the current position of the car and the target point.
     * Might return NO_RECOMMENDED_POINT if there is no recommendation.
     *
     * @return
     */
    public java.awt.geom.Point2D getRecommendedPosition();

    /**
     * Returns the recommended speed to avoid crashing into other cars.
     * Might return NO_RECOMMENDE_SPEED if there is no need to slow down.
     *
     * @return
     */
    public double getRecommendedSpeed();

    public boolean otherCars();

    public void setTrackModel(TrackModel trackModel);

    public void setStage(Stage s);

    public void reset();

}
