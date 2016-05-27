/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.behaviour.decision;

/**
 *
 * @author Tim
 */
public class RealPositionBasedDecider extends LineOrCircleDecider {

    @Override
    public boolean isLinear() {

        double current_position_by_circles = (
                data.circleSit.getPositionCircle().getRadius() - 
                data.circleSit.getCenterCircle().getRadius())/data.trackWidth;
        
        double current_position_by_lines =
                data.lineSit.getRightLine().getNearestPointOnLine(data.lineSit.getPositionLine().getPoint())
                    .distance(data.lineSit.getPositionLine().getPoint())/data.trackWidth*2-1;

        return Math.abs(current_position_by_lines - data.trackPosition) <= Math.abs(current_position_by_circles - data.trackPosition);
    }
}
