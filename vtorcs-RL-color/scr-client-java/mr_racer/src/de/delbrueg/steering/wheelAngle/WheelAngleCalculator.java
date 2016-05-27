/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.wheelAngle;

/**
 * This is the abstract base class for all classes that provide a formula to calculate the angle of the front wheels to arrive at a given point.
 * @author Tim
 */
public abstract class WheelAngleCalculator {

    protected WheelAngleCalcData data = null;

    public void setData(WheelAngleCalcData data) {
        this.data = data;
    }

    /**
     * this classes main calculation function
     * must be implemented in subclasses
     * @return angle in radians [-PI/2,PI/2], negative = left
     */
    public abstract double getWheelTurnAngle();
}
