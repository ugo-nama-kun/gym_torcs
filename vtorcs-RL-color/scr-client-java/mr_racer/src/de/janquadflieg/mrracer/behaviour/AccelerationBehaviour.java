/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.behaviour;

import de.janquadflieg.mrracer.track.TrackSegment;

/**
 *
 * @author quad
 */
public interface AccelerationBehaviour
extends Behaviour{
    /**
     * The segment of the trackmodel containing the current position of the car.
     * Might be null, if the trackmodel has not beeen initialized or unknown, if
     * the controller is still learning the track during the first lap.
     * @param s The track segment.
     */
    public void setTrackSegment(TrackSegment s);

    /**
     * The desired target speed.
     * @param speed The speed.
     */
    public void setTargetSpeed(double speed);

    /**
     * The width of the race track in meter.
     * @param width
     */
    public void setWidth(double width);

}
