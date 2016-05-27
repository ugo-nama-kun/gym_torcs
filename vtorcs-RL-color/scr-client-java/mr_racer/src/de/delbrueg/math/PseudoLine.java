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
import javax.vecmath.Vector2d;

/**
 * A PseudoLine represents a Line, but does not have to be exact.
 * To use it, add points and then call getLine().
 * @author Tim
 */
public class PseudoLine {

    // points that define the meanLine
    private List<Point2d> points = new LinkedList<Point2d>();

    // the resulting Line that all points should be near to
    private Line meanLine = null;

    // true if we have to rebuild the meanLine
    private boolean dirty = true;

    public PseudoLine(){

    }

    public PseudoLine( Collection<Point2d> ps){
        points.addAll(ps);
    }

    // List because a Collection gives a name clash with the above constructor
    public PseudoLine( List<Point2D> ps){
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
     * Add points with addPoint. When ready, call getMeanLine().
     * @param x the point.x to add
     * @param y the point.y to add
     */
    public void addPoint( final double x, final double y ){
        points.add(new Point2d(x,y));

        dirty = true;
    }

    /**
     * Add points with addPoint. When ready, call getMeanLine().
     * @param p the point to add
     */
    public void addPoint( final Point2d point ){
        points.add(new Point2d(point));

        dirty = true;
    }

    /**
     * Add points with addPoint. When ready, call getMeanLine().
     * @param p the point to add
     */
    public void addPoint( final Point2D point ){
        this.addPoint(point.getX(),point.getY());
    }

    /**
     * @return Line that allpoints should be near to
     */
    public Line getMeanLine(){
        if(dirty)
           recalculate();

        return meanLine;
    }
    
    /**
     * recalculates the Line
     */
    private void recalculate()
    {
        // aggregate
        Point2d point_sum = new Point2d();
        Vector2d direction_sum = new Vector2d();
        int n = points.size();
        for(int i = 0; i < n; i++)
        {
            Line c = new Line(points.get(i), points.get((i+1)%n));
            c.getDirection().normalize();

            // make sure y dir is positive
            if( c.getDirection().y < 0 || (c.getDirection().y == 0 && c.getDirection().x < 0) )
                c.getDirection().scale(-1);

            direction_sum.add(c.getDirection());
            point_sum.add(c.getPoint());
        }

        direction_sum.scale(1.0 / ((double)n));
        point_sum.scale(1.0 / ((double)n));


        // calc mean and save
        meanLine = new Line(point_sum, direction_sum);
        dirty = false;
    }
}
