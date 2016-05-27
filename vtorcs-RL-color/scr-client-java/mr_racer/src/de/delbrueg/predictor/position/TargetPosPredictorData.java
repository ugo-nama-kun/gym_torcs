/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.predictor.position;

import de.delbrueg.steering.classification.CircleSituation;
import de.delbrueg.steering.classification.LineSituation;
import java.awt.geom.Point2D;
import javax.vecmath.Point2d;
import javax.vecmath.Vector2d;

/**
 *
 * @author Tim
 */
public class TargetPosPredictorData {

    public final Point2d position;
    public final double trackPosition;        //in [-1,1]
    public final java.awt.geom.Point2D targetTrackPosition;  //x in [-1,1]
    public final Vector2d direction;
    public final double speed;                //in m/s !!!
    public final CircleSituation circles;     //local track as circles
    public final LineSituation lines;     //local track as lines
    public final double trackWidth;           //width of the current track
    public final double timeStep;      //in seconds

    public TargetPosPredictorData(Point2d position, double trackPosition, Point2D targetTrackPosition, Vector2d direction, double speed, CircleSituation circles, LineSituation lines, double trackWidth, double timeStep) {
        this.position = position;
        this.trackPosition = trackPosition;
        this.targetTrackPosition = targetTrackPosition;
        this.direction = direction;
        this.speed = speed;
        this.circles = circles;
        this.lines = lines;
        this.trackWidth = trackWidth;
        this.timeStep = timeStep;
    }

    
}
