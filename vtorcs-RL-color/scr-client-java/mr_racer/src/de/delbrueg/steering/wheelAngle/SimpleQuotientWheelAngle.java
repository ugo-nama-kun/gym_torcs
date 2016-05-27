/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.wheelAngle;

/**
 * Just calculates the quotient of trackWidth and radius of driveCircle
 * NEEDS A SIGNCORRECTOR!!! Because the result will always be positive!
 *
 * Formula out of Fahrwerktechnik Handbuch
 * @author Tim
 */
public class SimpleQuotientWheelAngle extends WheelAngleCalculator {

    @Override
    public double getWheelTurnAngle() {
        return data.trackWidth / data.driveCircle.getRadius();
    }

}
