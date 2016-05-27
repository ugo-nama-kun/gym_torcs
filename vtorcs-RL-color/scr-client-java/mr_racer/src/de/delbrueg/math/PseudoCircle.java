/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.math;

import de.delbrueg.exceptions.PointsCollinearException;
import java.util.LinkedList;
import java.util.List;
import javax.vecmath.Point2d;
import java.awt.geom.Point2D;
import java.util.Collection;

/**
 * A PseudoCircle represents a circle, but does not have to be exact.
 * To use it, add points and then call getCircle().
 * @author Tim
 */
public class PseudoCircle {

    // points that define the meanCircle
    private List<Point2d> points = new LinkedList<Point2d>();

    // the resulting circle that all points should be near to
    private Circle meanCircle = null;

    // true if we have to rebuild the meanCircle
    private boolean dirty = true;

    public PseudoCircle(){

    }

    public PseudoCircle( Collection<Point2d> ps){
        points.addAll(ps);
    }

    // List because a Collection gives a name clash with the above constructor
    public PseudoCircle( List<Point2D> ps){
        for( Point2D x: ps){
            this.addPoint(x);
        }
    }


    /**
     * removes all points
     *
     */
    public void clearPoints(){
        points.clear();
        dirty = true;
    }

    /**
     * Add points with addPoint. When ready, call getMeanCircle().
     * @param x the point.x to add
     * @param y the point.y to add
     */
    public void addPoint( final double x, final double y ){
        points.add(new Point2d(x,y));

        dirty = true;
    }

    /**
     * Add points with addPoint. When ready, call getMeanCircle().
     * @param p the point to add
     */
    public void addPoint( final Point2d point ){
        points.add(new Point2d(point));

        dirty = true;
    }

    /**
     * Add points with addPoint. When ready, call getMeanCircle().
     * @param p the point to add
     */
    public void addPoint( final Point2D point ){
        this.addPoint(point.getX(),point.getY());
    }

    /**
     * @return Circle that allpoints should be near to
     */
    public Circle getMeanCircle() throws PointsCollinearException{
        if(dirty)
           recalculate();

        return meanCircle;
    }
    
    /**
     * recalculates the circle
     */
    private void recalculate() throws PointsCollinearException
    {
        // - for every triple of points, make a circle
        // - aggregate
        // - calc mean
        List<Circle> circles = new LinkedList<Circle> ();

        // aggregate
        for(int i = 0; i < points.size(); i++)
        {
            Circle c = new Circle(points.get(i), points.get((i+1)%points.size()), points.get((i+2)%points.size()));
            circles.add(c);
        }

        // calc mean and save
        meanCircle = Circle.getMeanCircle(circles);
        dirty = false;
    }
}
