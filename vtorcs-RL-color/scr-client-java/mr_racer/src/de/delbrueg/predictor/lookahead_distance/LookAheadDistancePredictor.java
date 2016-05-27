/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.predictor.lookahead_distance;

/**
 * predicts the distance up to which we should use sensors to get the track circles
 * @author Tim
 */
public abstract class LookAheadDistancePredictor {

    /**
     * speed in meter per second
     */
    protected double speed;


    /**
     * @param speed in meter per second
     */
    public final void setSpeed( double speed ){
        this.speed = speed;
    }

    /**
     * width of racetrack in meters
     */
    double trackWidth;

    /**
     *
     * @param track_width in meters
     */
    public final void setTrackWidth( double track_width ){
        trackWidth = track_width;
    }

    /**
     * get the distance up to which we should use sensors to get the track circles
     * @return
     */
    public abstract double predictMaxLookAheadDistance();

    /**
     * get the min distance which we should use sensors to get the track circles
     * @return
     */
    public abstract double predictLookAheadThreshold();
}
