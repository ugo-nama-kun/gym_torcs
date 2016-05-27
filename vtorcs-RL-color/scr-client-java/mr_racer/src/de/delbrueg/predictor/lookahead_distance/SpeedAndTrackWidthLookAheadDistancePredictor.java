/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.predictor.lookahead_distance;

/**
 * predicts the lookahead distance so that we use all points we can reach in k seconds
 * @author Tim
 */
public class SpeedAndTrackWidthLookAheadDistancePredictor extends LookAheadDistancePredictor{

    /**
     * predict k seconds
     */
    public final int k = 2;
    
    @Override
    public double predictMaxLookAheadDistance() {
        return Math.max(trackWidth*3, k*speed);
    }

    @Override
    public double predictLookAheadThreshold() {
        return trackWidth / 6.0;
    }


}
