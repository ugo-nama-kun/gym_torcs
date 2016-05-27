/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.wheelAngle;

import de.delbrueg.math.Line;

/**
 *
 * @author Tim
 */
public class SignDirectionCorrector<T extends WheelAngleCalculator> extends WheelAngleCalculator{

    private final T calculator;
    public SignDirectionCorrector(T calculator) {
        this.calculator = calculator;
    }

    @Override
    public double getWheelTurnAngle() {

        calculator.setData(data);

        double angle = Math.abs(calculator.getWheelTurnAngle());

        // left or right?
        // not always right!!!
        // test, if left or right of car_dir!!!
        Line cur_drive_line = new Line(data.carPos, data.carDir);
        boolean rightCurve = !cur_drive_line.isPointOnTheLeft(data.driveCircle.getCenter());

        if( rightCurve )
            angle *= -1;

        return angle;
    }

    
    
}
