package de.delbrueg.predictor.position;

import javax.vecmath.Point2d;
import de.delbrueg.math.Circle;
import de.delbrueg.math.Line;
import de.janquadflieg.mrracer.Utils;

/**
 *
 * @deprecated Outdated because left/rightCircle are not precise enough. Use CarPosCircleTargetPositionPredictor
 * @author Tim
 */
@Deprecated
public class WeightedCircleTargetPositionPredictor extends TargetPositionPredictor{

    @Override
    public Point2d predictFuturePosition(final double target_position_weight) {
        debugString = "";

        // calc step size along center circle
        Circle pos_circle = data.circles.getPositionCircle();
        double cr = pos_circle.getRadius();
        final double step_size = data.speed*data.timeStep;    // in meter
        debugString += "| " + Utils.dTS(cr);

        Line cur_drive_line = new Line(data.position, data.direction);
        boolean rightCurve = !cur_drive_line.isPointOnTheLeft(pos_circle.getCenter());

        Circle.RotationDirection rot_dir = rightCurve ?
            Circle.RotationDirection.clockwise :
            Circle.RotationDirection.counter_clockwise;

        Point2d currentPosTarget = pos_circle.goAlong(data.position,rot_dir,step_size);

        // in = -1, center = 0, out = 1
        double inOutTargetPosition = (pos_circle.getCenter().x > 0 ? -1 : 1) * (data.targetTrackPosition.getX()-data.trackPosition);
        debugString += "| IO:"+Utils.dTS(inOutTargetPosition);

        // calc target point (with target position)
        Point2d targetPoint = pos_circle.getScaled(1+inOutTargetPosition*data.trackWidth/(2*cr)).getNearestPointOnCircle(currentPosTarget);
        
        return targetPoint;
    }

}
