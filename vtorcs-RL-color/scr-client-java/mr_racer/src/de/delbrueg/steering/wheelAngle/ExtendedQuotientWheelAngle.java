/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.wheelAngle;

 /**
 * Calculates the quotient of trackWidth and radius of driveCircle
 * NEEDS A SIGNCORRECTOR!!! Because the result will always be positive!
  *
 * Formula out of Fahrwerktechnik Handbuch, page 91
  * 
 * @author Tim
 */
public class ExtendedQuotientWheelAngle extends WheelAngleCalculator {

    /**
     * used to calculate the lateral acceleration
     */
    double lastLateralSpeed = 0;

    @Override
    public double getWheelTurnAngle() {
        double quot = data.trackWidth / data.driveCircle.getRadius();
        double mass = data.carMass / data.trackWidth;
        double lateralAcceleration = (data.lateralSpeed-lastLateralSpeed)/data.timeStep;

        return quot + mass*data.carPropSteeringFactor*lateralAcceleration;
    }

}
