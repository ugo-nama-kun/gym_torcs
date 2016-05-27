package de.delbrueg.steering.classification;

import de.janquadflieg.mrracer.telemetry.SensorData;
import de.delbrueg.math.Circle;
import de.delbrueg.math.PseudoCircle;
import de.delbrueg.predictor.lookahead_distance.LookAheadDistancePredictor;
import de.delbrueg.steering.behaviour.CircleSteeringSettings;
import de.janquadflieg.mrracer.Utils;
import javax.vecmath.Point2d;
import java.awt.geom.Point2D;

/**
 *
 * @author Tim
 */
public class CircleClassifier{

    private double trackWidth;
    CircleSteeringSettings settings;

    private boolean DEBUG_TEXT = false;

    public CircleClassifier(double trackWidth, float[] angles, CircleSteeringSettings settings, boolean noisy){
        this.trackWidth = trackWidth;
        this.settings = settings;        
    }

    /**
     * calculate circles that approximate the left/right track edges
     * @param data the data to analyse for left/right track circles
     * @return CircleSituation (can be invalid!!! iff no circles are present)
     */
    public CircleSituation classify(SensorData data, TrackPointSituation sit) {
        // used as weights of pseudo circles
        int left_size = sit.getLeftTrackPoints().size();
        int right_size = sit.getRightTrackPoints().size();

        if( DEBUG_TEXT )
            System.out.println(Utils.iTS(left_size) + " | " + Utils.iTS(right_size));

        PseudoCircle leftPseudoCircle = new PseudoCircle(sit.getLeftTrackPoints());
        PseudoCircle rightPseudoCircle = new PseudoCircle(sit.getRightTrackPoints());

        try{
            // first, just use the mean circles
            Circle leftCircle = leftPseudoCircle.getMeanCircle();
            Circle rightCircle = rightPseudoCircle.getMeanCircle();

            if( DEBUG_TEXT )
            {
                System.out.println("pure pseudo circles:");
                System.out.println("left: (" + Utils.dTS(leftCircle.getCenter().getX()) + ", " + Utils.dTS(leftCircle.getCenter().getY()) + ")");
                System.out.println("left rad: " + Utils.dTS(leftCircle.getRadius()));
                System.out.println("right: (" + Utils.dTS(rightCircle.getCenter().getX()) + ", " + Utils.dTS(rightCircle.getCenter().getY()) + ")");
                System.out.println("right rad: " + Utils.dTS(rightCircle.getRadius()));
            }

            // calc left/right circles based on the other one, if one alone has not enough points
            if( (left_size < 3) || ( left_size < right_size && settings.only_use_side_with_more_points )){

                // use right circle to define left circle
                // curve to the right=add or left = subtract
                int sgn = (rightCircle.getCenter().x >= 0) ? 1 : -1;
                leftCircle = new Circle(rightCircle.getCenter(),rightCircle.getRadius() + sgn*trackWidth);

            }
            else if( right_size < 3 || ( left_size > right_size && settings.only_use_side_with_more_points )){

                // use left circle to define right circle
                // curve to the right=subtract or left = add
                int sgn = (leftCircle.getCenter().x >= 0) ? -1 : 1;
                rightCircle = new Circle(leftCircle.getCenter(),leftCircle.getRadius() + sgn*trackWidth);
            }


            if( DEBUG_TEXT )
            {
                System.out.println("filtered circles:");
                System.out.println("left: (" + Utils.dTS(leftCircle.getCenter().getX()) + ", " + Utils.dTS(leftCircle.getCenter().getY()) + ")");
                System.out.println("left rad: " + Utils.dTS(leftCircle.getRadius()));
                System.out.println("right: (" + Utils.dTS(rightCircle.getCenter().getX()) + ", " + Utils.dTS(rightCircle.getCenter().getY()) + ")");
                System.out.println("right rad: " + Utils.dTS(rightCircle.getRadius()));
            }
            
            // combine the mean ones
            // get the weighted mean center point, but mean radius
            Circle middleCircle = Circle.getWeightedMeanCircle(leftCircle, rightCircle, right_size/(right_size+left_size));
            middleCircle.setRadius((leftCircle.getRadius()+rightCircle.getRadius())/2);


            if( DEBUG_TEXT )
            {
                System.out.println("center: (" + Utils.dTS(middleCircle.getCenter().getX()) + ", " + Utils.dTS(middleCircle.getCenter().getY()) + ")");
                System.out.println("center rad: " + Utils.dTS(middleCircle.getRadius()));
            }
            
            // alter radius of center circle to fit our current pos (0,0)
            Circle positionCircle = new Circle(middleCircle);
            positionCircle.setRadius(positionCircle.getCenter().distance(new Point2d(0,0)));

            double current_position_by_circles = (positionCircle.getRadius() - middleCircle.getRadius())/trackWidth;

            if( DEBUG_TEXT )
            {
                System.out.println("position: (" + Utils.dTS(positionCircle.getCenter().getX()) + ", " + Utils.dTS(positionCircle.getCenter().getY()) + ")");
                System.out.println("position rad: " + Utils.dTS(positionCircle.getRadius()));
                System.out.println("current_position_by_circles:" + Utils.dTS(current_position_by_circles));
            }
            
            // calc rating ( = weighted sum of quadratic failure over all sensors )
            double rating = 0;
            for(Point2D p : sit.getLeftTrackPoints()){
                Point2d pd = new Point2d(p.getX(),p.getY());
                rating += Math.pow(leftCircle.getDistance(pd),2);
            }
            for(Point2D p : sit.getRightTrackPoints()){
                Point2d pd = new Point2d(p.getX(),p.getY());
                rating += Math.pow(rightCircle.getDistance(pd),2);
            }

            return new CircleSituation(leftCircle, rightCircle, middleCircle, positionCircle, rating);
        }
        catch(Exception e){
            // now isValid returns false and the caller should know we did not succeed
            return new CircleSituation(null,null,null,null,10000000);
        }
    }
    
}
