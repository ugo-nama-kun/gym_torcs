package de.delbrueg.predictor.position;

import javax.vecmath.Point2d;
import javax.vecmath.Vector2d;

/**
 * This is the first line-based Position predictor.
 * @author Tim
 */
public class WeightedLineTargetPositionPredictor extends TargetPositionPredictor{

    @Override
    public Point2d predictFuturePosition(final double target_position_weight) {
        debugString = "";

        // calc step size along center circle
        Point2d p_left = data.lines.getLeftLine().getNearestPointOnLine(data.position);
        Point2d p_right = data.lines.getRightLine().getNearestPointOnLine(data.position);

        // get interpolate between left and right
        double left_scale = (1+data.targetTrackPosition.getX())/2.0;
        p_left.scale(left_scale);
        double right_scale = (1-data.targetTrackPosition.getX())/2.0;
        p_right.scale(right_scale);
        p_left.add(p_right);

//        System.out.println("apprx pos: "+p_left);
        
        // go along the lines
        Vector2d lines_dir = new Vector2d(data.lines.getCenterLine().getDirection());
        lines_dir.normalize();
        lines_dir.scale(data.speed*data.timeStep);
        p_left.add(lines_dir);
        
        return p_left;
    }

}
