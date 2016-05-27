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
public interface SteeringBehaviour
extends Behaviour{

    /**
     * The segment of the trackmodel containing the current position of the car.
     * Might be null, if the trackmodel has not beeen initialized or unknown, if
     * the controller is still learning the track during the first lap.
     * @param s The track segment.
     */
    public void setTrackSegment(TrackSegment s);

    /**
     * The desired target position. The x coordinate represents the position on
     * the track (1 left edge of the track, 0 middle, -1 right edge) and the y
     * coordinate the distance in meters we expect to travel within the
     * next timestep (based on the current speed of the car).
     * @param position The desired position.
     */
    public void setTargetPosition(java.awt.geom.Point2D position);

    /**
     * The width of the race track in meter.
     * @param width
     */
    public void setWidth(double width);

    
    public String getDebugInfo();
}
