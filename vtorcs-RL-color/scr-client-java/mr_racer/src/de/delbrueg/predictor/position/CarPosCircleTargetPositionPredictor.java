/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.predictor.position;

import de.janquadflieg.mrracer.Utils;
import javax.vecmath.Point2d;
import de.delbrueg.math.*;
import javax.vecmath.Vector2d;

/**
 * Predicts the target position based on a circle through the current position.
 * Our current position is translated along this circle by step_size.
 * Now the target track position is calculated relative to the translated pos.
 * @author Tim
 */
public class CarPosCircleTargetPositionPredictor extends TargetPositionPredictor{

    @Override
    public Point2d predictFuturePosition(final double target_position_weight) {
        debugString = "";

        // calc step size along center circle
        final double step_size = data.speed*data.timeStep;    // in meter
        debugString += "| " + Utils.dTS(data.circles.getCenterCircle().getRadius());

        // set radius so that we are on the circle
        Circle car_circle = data.circles.getPositionCircle();


        Line cur_drive_line = new Line(data.position, data.direction);
        boolean rightCurve = !cur_drive_line.isPointOnTheLeft(car_circle.getCenter());

        Circle.RotationDirection rot_dir = rightCurve ?
            Circle.RotationDirection.clockwise :
            Circle.RotationDirection.counter_clockwise;

        // target point on car circle
        Point2d pred_pos = car_circle.goAlong(data.position, rot_dir, step_size);

        // curve right = -1, left = 1
        int sgn = (car_circle.getCenter().x >= 0) ? -1 : 1;

        // positive = wannabe inner, negative = wannabe outer
        double pos_modifier= sgn * (data.targetTrackPosition.getX()-data.trackPosition);

//        System.out.println("PosModif: " + Utils.dTS(pos_modifier));
        
        // move the point to reflect the target track position
        Vector2d dir_to_circle_center = new Vector2d(car_circle.getCenter());
        dir_to_circle_center.sub(pred_pos);
        dir_to_circle_center.normalize();
        dir_to_circle_center.scale(data.trackWidth*pos_modifier*target_position_weight);

        pred_pos.add(dir_to_circle_center);

        return pred_pos;
    }
}
