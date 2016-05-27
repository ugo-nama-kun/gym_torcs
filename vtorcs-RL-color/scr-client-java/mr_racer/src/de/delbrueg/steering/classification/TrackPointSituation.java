/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.classification;

import java.awt.geom.Point2D;
import java.util.Vector;

/**
 * Situation that describes the known points of the track, divided into left and right
 * @author Tim
 */
public class TrackPointSituation {
    /**
     * points of the track we see through our sensors (left)
     * (coordinates relative to our car)
     */
    private Vector<Point2D> leftTrackPoints = new Vector<Point2D>();


    /**
     * points of the track we see through our sensors (right)
     * (coordinates relative to our car)
     */
    private Vector<Point2D> rightTrackPoints = new Vector<Point2D>();


    /**
     * points of the track we see through our sensors (left)
     * (coordinates relative to our car)
     */
    public Vector<Point2D> getLeftTrackPoints() {
        return leftTrackPoints;
    }

    /**
     * points of the track we see through our sensors (right)
     * (coordinates relative to our car)
     */
    public Vector<Point2D> getRightTrackPoints() {
        return rightTrackPoints;
    }
}
