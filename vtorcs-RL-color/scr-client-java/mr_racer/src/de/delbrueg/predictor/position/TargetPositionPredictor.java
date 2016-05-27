package de.delbrueg.predictor.position;

import de.delbrueg.steering.classification.CircleSituation;
import de.delbrueg.steering.classification.LineSituation;
import javax.vecmath.Point2d;
import javax.vecmath.Vector2d;

/**
 * used to predict the future 2d position of a car
 * @author Tim
 */
public abstract class TargetPositionPredictor {

    public String debugString = "";

    protected TargetPosPredictorData data;

    public void setData(TargetPosPredictorData data) {
        this.data = data;
    }


    /** Predicts the point we want to reach in the next step. Before use, please set all the parameters!
     *
     * @param target_position_weight weight of the target_position adjustment against normal track following
     *          1 = full adjustment
     *          0 = just hold position, dont try to reach the target
     * @return the point to reach in this timestep
     */
    public abstract Point2d predictFuturePosition( final double target_position_weight );
}