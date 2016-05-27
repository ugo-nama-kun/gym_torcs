/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.wheelAngle;

/**
 *
 * @author Tim
 */
public enum WheelAngleCalculators {

    /**
     * Just calculates the quotient of trackWidth and radius of driveCircle.
     * Uses a sign corrector to get left/right signums
     */
    SimpleQuotient {

        protected WheelAngleCalculator create() {
            return new SignDirectionCorrector<WheelAngleCalculator>(new SimpleQuotientWheelAngle());
        }
    },
    /**
     * Calculates the quotient of trackWidth and radius of driveCircle.
     * It also corrects under/oversteering and takes into account the lateral acceleration.
     * 
     * Uses a sign corrector to get left/right signums
     */
    ExtendedQuotient {

        protected WheelAngleCalculator create() {
            return new SignDirectionCorrector<WheelAngleCalculator>(new ExtendedQuotientWheelAngle());
        }
    };


    /**
     * abstract version so that the others can override it
     * @return
     */
    protected abstract WheelAngleCalculator create();

    /**
     * the TargetPositionPredictor we use
     */
    private WheelAngleCalculator wac = null;

    /**
     * get the TargetPositionPredictor, create one if neccessary
     * @return
     */
    public WheelAngleCalculator get(){

        if( wac == null ){
            wac = /*new TestStraightFirstWheelAngleCalculator(*/create()/*)*/;
        }

        return wac;
    }
}
