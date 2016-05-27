/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.predictor.lookahead_distance;

/**
 * predicts the lookahead distance just by the track width
 * @author Tim
 */
public class SimplyTrackWidthLookAheadDistancePredictor extends LookAheadDistancePredictor{

    @Override
    public double predictMaxLookAheadDistance() {
        return trackWidth*7;
    }

    @Override
    public double predictLookAheadThreshold() {
        return trackWidth / 100.0;
    }


}
