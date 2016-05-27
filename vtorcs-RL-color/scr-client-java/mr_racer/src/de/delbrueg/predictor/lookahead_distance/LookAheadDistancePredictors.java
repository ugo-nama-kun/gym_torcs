/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.predictor.lookahead_distance;

/**
 *
 * @author Tim
 */
public enum LookAheadDistancePredictors {

    simplyTrackWidth{

        @Override
        public LookAheadDistancePredictor create() {
            return new SimplyTrackWidthLookAheadDistancePredictor();
        }

    },

    speedAndTrackWidth{

        @Override
        public LookAheadDistancePredictor create() {
            return new SpeedAndTrackWidthLookAheadDistancePredictor();
        }

    };

    


    public abstract LookAheadDistancePredictor create();
}
