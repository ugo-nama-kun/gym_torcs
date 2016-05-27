package de.delbrueg.predictor.position;

import javax.vecmath.Point2d;
import de.delbrueg.math.Circle;
import de.delbrueg.math.Line;
import de.janquadflieg.mrracer.Utils;

/**
 * This is the resurrection of CenterCircleTargetPositionPredictor.
 * Now its better because we use the step_size given from the plan module, so
 * at least the communication between the 2 modules is exact!
 *
 * ignores time_step and speed, just uses the given targetTrackPosition!
 *
 * @author Tim
 */
public class ExactCenterCircleTargetPositionPredictor extends TargetPositionPredictor{

    @Override
    public Point2d predictFuturePosition(final double target_position_weight) {
        debugString = "";

        // calc step size along center circle
        Circle cc = data.circles.getCenterCircle();
        double cr = cc.getRadius();                 // circle radius
        final double step_size = data.targetTrackPosition.getY();    // in meter
        debugString += "| " + Utils.dTS(cr);

        Line cur_drive_line = new Line(data.position, data.direction);
        boolean rightCurve = !cur_drive_line.isPointOnTheLeft(cc.getCenter());

        Circle.RotationDirection rot_dir = rightCurve ?
            Circle.RotationDirection.clockwise :
            Circle.RotationDirection.counter_clockwise;

        Point2d nearestPointOnCenterCircle = cc.getNearestPointOnCircle(data.position);
        Point2d nullPosTarget = cc.goAlong(nearestPointOnCenterCircle,rot_dir,step_size);

        // in = -1, center = 0, out = 1
        double inOutTargetPosition = (cc.getCenter().x > 0 ? -1 : 1) * (data.targetTrackPosition.getX());
        debugString += "| "+Utils.dTS(inOutTargetPosition);

        // calc target point (with target position)
        Point2d targetPoint = cc.getScaled(1+inOutTargetPosition*data.trackWidth/(2*cr)).getNearestPointOnCircle(nullPosTarget);

        debugString += "| ";

        return targetPoint;
    }

}
