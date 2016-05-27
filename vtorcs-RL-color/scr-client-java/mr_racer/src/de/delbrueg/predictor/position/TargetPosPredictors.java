/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.delbrueg.predictor.position;

/**
 *
 * @author Tim
 */
public enum TargetPosPredictors {

    /**
     * calc a circle that corresponds to the current car pos and approximate the next step size based on this
     */
    CarPosCircle {

        protected TargetPositionPredictor create() {
            return new CarPosCircleTargetPositionPredictor();
        }
    },
    /**
     *
     * @deprecated Outdated because left/rightCircle are not precise enough. Use CarPosCircleTargetPositionPredictor
     * @author Tim
     */
    CenterCircle {

        protected TargetPositionPredictor create() {
            return new CenterCircleTargetPositionPredictor();
        }
    },
    /**
     * use the exact data as given from plan module
     */
    ExactCenterCircle {

        protected TargetPositionPredictor create() {
            return new ExactCenterCircleTargetPositionPredictor();
        }
    },
    /**
     * @deprecated Outdated because left/rightCircle are not precise enough. Use CarPosCircleTargetPositionPredictor
     */
    WeightedCircle {

        protected TargetPositionPredictor create() {
            return new WeightedCircleTargetPositionPredictor();
        }
    },
    WeightedLineTargetPositionPredictor{

        protected TargetPositionPredictor create() {
            return new WeightedLineTargetPositionPredictor();
        }
    };


    /**
     * abstract version so that the others can override it
     * @return
     */
    protected abstract TargetPositionPredictor create();

    /**
     * the TargetPositionPredictor we use
     */
    private TargetPositionPredictor tpp = null;

    /**
     * get the TargetPositionPredictor, create one if neccessary
     * @return
     */
    public TargetPositionPredictor get(){
        
        if( tpp == null ){
            tpp = create();
        }

        return tpp;
    }
}
