/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.math;

import de.janquadflieg.mrracer.Utils;
import de.delbrueg.exceptions.PointsCollinearException;
import java.security.InvalidParameterException;
import java.util.List;
import javax.vecmath.Matrix3d;
import javax.vecmath.Point2d;
import java.util.ArrayList;
import javax.vecmath.SingularMatrixException;
import javax.vecmath.Vector2d;
import javax.vecmath.Vector3d;

/**
 * represents a circle as you know it from school based on center and circle
 * can also represent a circle with infinite radius = straight!
 *
 * @author Tim
 * @param P the point class (Point2d for example)
 */
public class Circle {

    // center of the circle
    private Point2d center = new Point2d();

    // radius of the circle
    private double radius = 0;

    public Circle(){
        center = new Point2d();
        radius = 0;
    }

    public Circle( Circle c){
        center = c.getCenter();
        radius = c.getRadius();
    }

    /**
     * calc a circle based on center and radius
     * @param center 
     * @param radius
     * @throws NullPointerException if center is null
     * @throws InvalidParameterException if radius is zero
     */
    public Circle ( final Point2d center, final double radius ){

        if( center==null )
            throw new NullPointerException("Circle.set: center of circle is null!!!");
        if( radius<=0 )
            throw new InvalidParameterException("Circle.set: radius of circle is zero!!!");

        this.center = center;
        this.radius = radius;
    }


    /**
     * calc a circle based on 3 points that are placed on the circle
     * @param p1
     * @param p2
     * @param p3
     * @throws PointsCollinearException if the 3 points form a line
     * @throws NullPointerException if one point is null
     */
    public Circle ( final Point2d p1, final Point2d p2, final Point2d p3 ) throws PointsCollinearException{

        if( p1==null || p2==null || p3==null )
            throw new NullPointerException("Circle.set: at least one point is null!!!");
        
        /**
         * solve equations based on http://www.arndt-bruenner.de/mathe/scripts/kreis3p.htm
         */

        // coefficient matrix of (A,B,C)
        javax.vecmath.Matrix3d m = new Matrix3d();
        m.m00 = 1;          m.m10 = 1;          m.m20 = 1;
        m.m01 = -p1.x;      m.m11 = -p2.x;      m.m21 = -p3.x;
        m.m02 = -p1.y;      m.m12 = -p2.y;      m.m22 = -p3.y;

        try{

            m.invert();

            // right side of equation
            javax.vecmath.Vector3d right = new Vector3d();
            right.x = -(p1.x*p1.x + p1.y*p1.y);
            right.y = -(p2.x*p2.x + p2.y*p2.y);
            right.z = -(p3.x*p3.x + p3.y*p3.y);

            // abc = (A,B,C) = m^(-1) * right
            // not supported (???) in vecmath.... dumb idiots...
            Vector3d abc = new Vector3d();
            javax.vecmath.Vector3d x = new Vector3d();
            m.getRow(0, x);
            abc.x = right.dot(x);
            m.getRow(1, x);
            abc.y = right.dot(x);
            m.getRow(2, x);
            abc.z = right.dot(x);

            // now the center is:
            center.x = abc.y/2.0d;      // B/2
            center.y = abc.z/2.0d;      // C/2

            // and radius = x*x + y*y - A
            radius = Math.sqrt(center.x*center.x + center.y*center.y - abc.x);
        }

        // the 3 points are colinear
        catch(SingularMatrixException _){
            radius = 1.0;
            center.x = 0;
            center.y = 0;

            throw new PointsCollinearException(p1.x,p1.y,p2.x,p2.y,p3.x,p3.y);
        }
    }

    /**
     * set Circle by point, direction of circle in that point, and another point
     * 
     * @param pos first point on circle
     * @param direction direction of circle in pos
     * @param other_point other point on circle
     */
    public Circle (final Point2d pos, final Vector2d direction, final Point2d other_point){

        // we know the circle center lies on this line
        Line point_center_line = new Line(pos, MathUtil.getOrthogonalDirection(direction));

        // and also on this dividing line in the middle of boths points
        Point2d middle = new Point2d ( pos );
        middle.add(other_point);
        middle.scale(0.5);

        Vector2d point_diff = new Vector2d ( pos );
        point_diff.sub(other_point);
        point_diff = MathUtil.getOrthogonalDirection(point_diff);
        Line middle_line = new Line(middle, point_diff);

        // so the center should be the intersection of both lines
        center = point_center_line.intersectionWith(middle_line);

        if( center==null )
        {
            // boths lines are parallel
            // center is in the middle betweenpos and other_point
            center = new Point2d(pos);
            center.add(other_point);
            center.scale(0.5);
        }

        // radius is the distance to pos
        radius = center.distance(pos);
    }

    public enum RotationDirection{
        clockwise,
        counter_clockwise;
    }    

    public Point2d getCenter() {
        return new Point2d(center);
    }

    public double getRadius() {
        return radius;
    }

    public double getLength() {
        return 2*Math.PI*getRadius();
    }

    /**
     * calc the mean circle of a given circle list
     * line segments are NOT used
     * @param circles   circles of which the mean circle should be calculated
     * @param weight   0 means only circle1, 1 means only circle2, others interpolate
     * @return the mean circle
     */
    public static Circle getWeightedMeanCircle(final Circle circle1, final Circle circle2, final double weight){

        // make sure weight is in [0..1]
        double wt = Utils.truncate(weight, 0, 1);

        ArrayList<Circle> l = new ArrayList<Circle>(2);
        l.add(circle1);l.add(circle2);

        ArrayList<Double> w = new ArrayList<Double>(2);
        w.add(1-wt);w.add(wt);

        return getWeightedMeanCircle (l,w);
    }

    /**
     * calc the mean circle of a given circle list
     * line segments are NOT used
     * @param circles   circles of which the mean circle should be calculated
     * @param weights   weights of circles, sum MUST BE 1
     * @return the mean circle
     */
    public static Circle getWeightedMeanCircle(final List<Circle> circles, final List<Double> weights ){

        {
            double _sum = de.delbrueg.math.MathUtil.sum_double(weights);
            if(Math.abs(_sum-1)>0.0000001)
                System.out.println("Circle.getgetWeightedMeanCircle: sum of weights is not 1 but: " + _sum);
        }

        // for now, just use the mean center and mean radius
        Point2d p = new Point2d();
        double r = 0;
        int num = 0;
        int i = 0;
        for(Circle c: circles){

            // weight for this circle
            double w = weights.get(i);
            i++;
            
            // dont use line segments
            if( c.getRadius() > 0) {
                
                // scale
                Point2d center_scaled = new Point2d(c.getCenter());
                center_scaled.scale(w);
                double radius_scaled = c.getRadius()*w;
                
                // aggregate
                p.add(center_scaled);
                r += radius_scaled;

                num+=1;
            }
        }

        // the circle to return
        Circle n = new Circle();
        
        // if nothing was found
        if( num <= 0 )
            return n;

        // return as circle
        n.radius = r;
        n.center = p;
        return n;
    }

    /**
     * calc the mean circle of a given circle list
     * line segments are NOT used
     * @param circles   circles of which the mean circle should be calculated
     * @return the mean circle
     */
    public static Circle getMeanCircle(final Circle circle1, final Circle circle2 ){
        ArrayList<Circle> l = new ArrayList<Circle>(2);
        l.add(circle1);l.add(circle2);
        return getMeanCircle (l);
    }

    /**
     * calc the mean circle of a given circle list
     * line segments are NOT used
     * @param circles   circles of which the mean circle should be calculated
     * @return the mean circle
     */
    public static Circle getMeanCircle(final List<Circle> circles ){

        // the circle to return
        Circle n = new Circle();

        // for now, just use the mean center and mean radius
        Point2d p = new Point2d();
        double r = 0;
        int num = 0;
        for(Circle c: circles){

            // dont use line segments
            if( c.getRadius() > 0) {
                p.add(c.getCenter());
                r += c.getRadius();
                num+=1;
            }
        }

        // if nothing was found
        if( num <= 0 )
            return n;

        // scale with size^(-1)
        r /= num;
        p.scale(1.0/num);

        // return as circle
        n.radius = r;
        n.center = p;
        return n;
    }

    public Point2d getNearestPointOnCircle(final Point2d origin){

        // diff = radius*normalze(origin-center)
        Point2d diff = new Point2d(origin);
        diff.sub(center);
        diff = MathUtil.normalize(diff);
        diff.scale(radius);

        // target = center + diff
        Point2d p = new Point2d(center);
        p.add(diff);

        return p;
    }

    /**
     * goes from "point" up a "distance" along this circle
     * @param from point on circle
     * @param dir direction: (clockwise, counter-clockwise)
     * @param distance how far to go
     * @return resulting point on circle
     */
    public Point2d goAlong(final Point2d point, final RotationDirection dir, final double distance) throws InvalidParameterException{

        // calc angle (in radians) by distance
        double angle = distance / this.getRadius();

        Point2d target = MathUtil.rotatePoint(point,center,dir,angle);

        return target;
    }

    /**
     * return a scaled version (only radius is scaled)
     * @param scale scale to apply to radius
     * @return scaled version of this circle
     * @exception InvalidParameterException if scale is <= zero
     */
    public Circle getScaled( final double scale ) throws InvalidParameterException{
        
        Circle c = new Circle(getCenter(), scale * getRadius());

        return c;
    }

    public void setCenter(Point2d center) {
        this.center = center;
    }

    public void setRadius(double radius) {
        this.radius = radius;
    }

    /**
     * calc the distance to a point
     * @param p
     * @return
     */
    public double getDistance( Point2d p){
        Point2d p_near = getNearestPointOnCircle(p);

        return p_near.distance(p);
    }

    
}
