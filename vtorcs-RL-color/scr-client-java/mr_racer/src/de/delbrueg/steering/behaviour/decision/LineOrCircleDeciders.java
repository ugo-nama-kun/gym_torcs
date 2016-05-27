/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.behaviour.decision;

/**
 *
 * @author Tim
 */
public enum LineOrCircleDeciders {
    
    QuadraticFailureDecider{
        @Override
        protected LineOrCircleDecider create(){
            return new QuadraticFailureDecider();
        }
    },
    RealPositionBasedDecider{
        @Override
        protected LineOrCircleDecider create(){
            return new RealPositionBasedDecider();
        }
    },
    LinesOnlyDecider{
        @Override
        protected LineOrCircleDecider create(){
            return new ConstantDecider(true);
        }
    },
    CirclesOnlyDecider{
        @Override
        protected LineOrCircleDecider create(){
            return new ConstantDecider(false);
        }
    };

    /**
     * abstract version so that the others can override it
     * @return
     */
    protected abstract LineOrCircleDecider create();

    /**
     * the TargetPositionPredictor we use
     */
    private LineOrCircleDecider tpp = null;

    /**
     * get the TargetPositionPredictor, create one if neccessary
     * @return
     */
    public LineOrCircleDecider get(){

        if( tpp == null ){
            tpp = create();
        }

        return tpp;
    }
}
