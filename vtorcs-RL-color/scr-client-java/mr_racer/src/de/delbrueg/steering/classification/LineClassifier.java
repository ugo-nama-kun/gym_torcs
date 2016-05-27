/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.classification;

import de.janquadflieg.mrracer.telemetry.SensorData;
import de.delbrueg.math.Line;
import de.delbrueg.math.PseudoLine;
import de.delbrueg.predictor.lookahead_distance.LookAheadDistancePredictor;
import de.delbrueg.steering.behaviour.CircleSteeringSettings;
import de.janquadflieg.mrracer.Utils;
import javax.vecmath.Point2d;
import java.awt.geom.Point2D;
import javax.vecmath.Vector2d;

/**
 *
 * @author Tim
 */
public class LineClassifier{

    private double trackWidth;    
    private boolean DEBUG_TEXT = false;

    public LineClassifier(double trackWidth, float[] angles, CircleSteeringSettings settings, boolean noisy){
        this.trackWidth = trackWidth;        
    }

    /**
     * calculate Lines that approximate the left/right track edges
     * @param data the data to analyse for left/right track Lines
     * @return LineSituation (can be invalid!!! iff no Lines are present)
     */
    public LineSituation classify(SensorData data, TrackPointSituation sit) {         

        // used as weights of pseudo Lines
        int left_size = sit.getLeftTrackPoints().size();
        int right_size = sit.getRightTrackPoints().size();

        if( DEBUG_TEXT )
            System.out.println(Utils.iTS(left_size) + " | " + Utils.iTS(right_size));

        PseudoLine leftPseudoLine = new PseudoLine(sit.getLeftTrackPoints());
        PseudoLine rightPseudoLine = new PseudoLine(sit.getRightTrackPoints());
        
        try{
            // first, just use the mean Lines
            Line leftLine = leftPseudoLine.getMeanLine();
            Line rightLine = rightPseudoLine.getMeanLine();

            if( DEBUG_TEXT )
            {
                System.out.println("pure pseudo lines:");
                System.out.println("left point: (" + Utils.dTS(leftLine.getPoint().getX()) + ", " + Utils.dTS(leftLine.getPoint().getY()) + ")");
                System.out.println("left direction: (" + Utils.dTS(leftLine.getDirection().getX()) + ", " + Utils.dTS(leftLine.getDirection().getY()) + ")");
                System.out.println("right point: (" + Utils.dTS(rightLine.getPoint().getX()) + ", " + Utils.dTS(rightLine.getPoint().getY()) + ")");
                System.out.println("right direction: (" + Utils.dTS(rightLine.getDirection().getX()) + ", " + Utils.dTS(rightLine.getDirection().getY()) + ")");
            }

            /**
             * make sure lines point to front
             */
            if(leftLine.getDirection().getY() < 0)
                leftLine.getDirection().scale(-1.0d);
            if(rightLine.getDirection().getY() < 0)
                rightLine.getDirection().scale(-1.0d);

            // calc left/right Lines based on the other one, if one alone has not enough points
            if( left_size < right_size ){

                // use right Line to define left Line
                leftLine = rightLine.getLeftParallel(trackWidth);

            }
            else if( left_size > right_size ){

                // use left Line to define right Line
                rightLine = leftLine.getLeftParallel(-trackWidth);
            }


            if( DEBUG_TEXT )
            {
                System.out.println("filtered pseudo lines:");
                System.out.println("left point: (" + Utils.dTS(leftLine.getPoint().getX()) + ", " + Utils.dTS(leftLine.getPoint().getY()) + ")");
                System.out.println("left direction: (" + Utils.dTS(leftLine.getDirection().getX()) + ", " + Utils.dTS(leftLine.getDirection().getY()) + ")");
                System.out.println("right point: (" + Utils.dTS(rightLine.getPoint().getX()) + ", " + Utils.dTS(rightLine.getPoint().getY()) + ")");
                System.out.println("right direction: (" + Utils.dTS(rightLine.getDirection().getX()) + ", " + Utils.dTS(rightLine.getDirection().getY()) + ")");
            }
            
            // combine the mean ones
            // get the weighted mean center point, mean direction
            Point2d middle_pos = new Point2d(leftLine.getPoint());
            middle_pos.add(rightLine.getPoint());
            middle_pos.scale(0.5d);
            Vector2d middle_dir = new Vector2d(leftLine.getDirection());
            middle_dir.add(rightLine.getDirection());
            middle_dir.scale(0.5d);
            Line middleLine = new Line(middle_pos, middle_dir);
            
            // center Line to fit our current pos (0,0)
            Line positionLine = new Line(middleLine);
            positionLine.getPoint().x = 0;
            positionLine.getPoint().y = 0;

            if( DEBUG_TEXT )
            {
                System.out.println("center line:");
                System.out.println("center point: (" + Utils.dTS(middleLine.getPoint().getX()) + ", " + Utils.dTS(middleLine.getPoint().getY()) + ")");
                System.out.println("center direction: (" + Utils.dTS(middleLine.getDirection().getX()) + ", " + Utils.dTS(middleLine.getDirection().getY()) + ")");
                System.out.println("position point: (" + Utils.dTS(positionLine.getPoint().getX()) + ", " + Utils.dTS(positionLine.getPoint().getY()) + ")");
                System.out.println("position direction: (" + Utils.dTS(positionLine.getDirection().getX()) + ", " + Utils.dTS(positionLine.getDirection().getY()) + ")");

                System.out.println("LinePos:"+Utils.dTS(rightLine.getNearestPointOnLine(positionLine.getPoint()).distance(positionLine.getPoint())/trackWidth*2-1));
            }

            // calc rating ( = weighted sum of quadratic failure over all sensors )
            double rating = 0;
            for(Point2D p : sit.getLeftTrackPoints()){
                Point2d pd = new Point2d(p.getX(),p.getY());
                rating += Math.pow(leftLine.getDistance(pd),2);
            }
            for(Point2D p : sit.getRightTrackPoints()){
                Point2d pd = new Point2d(p.getX(),p.getY());
                rating += Math.pow(rightLine.getDistance(pd),2);
            }

            return new LineSituation(leftLine, rightLine, middleLine, positionLine, rating);
        }
        catch(Exception e){
            // now isValid returns false and the caller should know we did not succeed
            return new LineSituation(null,null,null,null, 100000);
        }
    }
    
}
