package de.delbrueg.predictor.position;

import javax.vecmath.Point2d;
import de.delbrueg.math.Circle;
import de.janquadflieg.mrracer.Utils;

/**
 *
 * @deprecated Outdated because left/rightCircle are not precise enough. Use CarPosCircleTargetPositionPredictor
 * @author Tim
 */
@Deprecated
public class CenterCircleTargetPositionPredictor extends TargetPositionPredictor{

    @Override
    public Point2d predictFuturePosition(final double target_position_weight) {
        debugString = "";

        // calc step size along center circle
        Circle cc = data.circles.getCenterCircle();
        double cr = cc.getRadius();                 // circle radius
        final double step_size = data.speed*data.timeStep;    // in meter
        debugString += "| " + Utils.dTS(cr);

        Point2d nearestPointOnCenterCircle = cc.getNearestPointOnCircle(data.position);
        Point2d nullPosTarget = cc.goAlong(nearestPointOnCenterCircle,Circle.RotationDirection.clockwise,step_size);

        // in = -1, center = 0, out = 1
        double inOutTargetPosition = (cc.getCenter().x > 0 ? -1 : 1) * (data.targetTrackPosition.getX());
        debugString += "| "+Utils.dTS(inOutTargetPosition);

        // calc target point (with target position)
        Point2d targetPoint = cc.getScaled(1+inOutTargetPosition*data.trackWidth/(2*cr)).getNearestPointOnCircle(nullPosTarget);

        debugString += "| ";

        return targetPoint;
    }

}
