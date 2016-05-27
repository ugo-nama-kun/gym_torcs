/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.wheelAngle;

import de.delbrueg.math.Circle;
import javax.vecmath.Point2d;
import javax.vecmath.Vector2d;

/**
 *
 * @author Tim
 */
public class WheelAngleCalcData {

    /**
     * the circle we should follow
     */
    public final Circle driveCircle;

    /**
     * movement speed orthogonal to the car direction
     * in meter per second
     */
    public final double lateralSpeed;

    /**
     * distance of front/rear axis in meter
     */
    public final double trackWidth;

    /**
     * position of car
     */
    public final Point2d carPos;

    /**
     * direction of car
     */
    public final Vector2d carDir;

    /**
     * mass of car in kg
     */
    public final double carMass;

    /**
     * negative = oversteering
     * zero = neutral
     * positive = understeering
     */
    public final double carPropSteeringFactor;

    public final double timeStep;

    public WheelAngleCalcData(Circle driveCircle, double lateralSpeed, double trackWidth, Point2d carPos, Vector2d carDir, double carMass, double carPropSteeringFactor, double timeStep) {
        this.driveCircle = driveCircle;
        this.lateralSpeed = lateralSpeed;
        this.trackWidth = trackWidth;
        this.carPos = carPos;
        this.carDir = carDir;
        this.carMass = carMass;
        this.carPropSteeringFactor = carPropSteeringFactor;
        this.timeStep = timeStep;
    }

}
