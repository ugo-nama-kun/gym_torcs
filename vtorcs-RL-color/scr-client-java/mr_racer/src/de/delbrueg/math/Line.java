/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.math;

import Jama.Matrix;
import javax.vecmath.Point2d;
import javax.vecmath.Vector2d;

/**
 *
 * @author Tim
 */
public class Line {

    private final Point2d point;
    private final Vector2d direction;

    public Line(final Line other) {
        this.point = new Point2d(other.point);
        this.direction = new Vector2d(other.direction);
    }

    public Line(final Point2d point, final Vector2d direction) {
        this.point = new Point2d(point);
        this.direction = new Vector2d(direction);
    }

    public Line(final Point2d pointA, final Point2d pointB) {
        this.point = new Point2d(pointA);
        this.direction = new Vector2d(pointB);
        this.direction.sub(pointA);
    }

    /**
     * calc a line orthogonal to this one with the same point
     * @return
     */
    public Line getOrthogonalLine(){
        return new Line(point, MathUtil.getOrthogonalDirection(direction));
    }

    /**
     * calc intersection of 2 lines
     * @param other
     * @return the intersection point (null if the lines are parallel/identical)
     */
    public Point2d intersectionWith(final Line other){
        if( this.isParallelTo(other))
            return null;

        // lambda coefficients
        double[][] coeff = {
            { direction.x, -other.direction.x },
            { direction.y, -other.direction.y}};
        Matrix left = new Matrix(coeff);

        // right side of equation system
        double[][] results = { {other.point.x - point.x}, {other.point.y - point.y}};
        Matrix right = new Matrix(results);

        // solution: 2x1 Matrix with 2 lambdas for 2 lines
        Matrix lambda = left.solve(right);

        // first entry should be our lambda
        Point2d intersection = getPointAt(lambda.get(0, 0));

        return intersection;
    }

    public Vector2d getDirection() {
        return direction;
    }

    public Point2d getPoint() {
        return point;
    }

    /**
     *
     * @param other
     * @return true if boths lines share a common direction
     */
    public boolean isParallelTo(final Line other){
        Vector2d a = MathUtil.normalize(direction);
        Vector2d b = MathUtil.normalize(other.getDirection());

        double angle = a.angle(b);

        return angle < 0.0001;
    }

    /**
     * evaluate point+lambda*dir
     * @param lambda
     * @return
     */
    public Point2d getPointAt(final double lambda){
        Point2d x = new Point2d( point );
        Vector2d diff = (Vector2d) direction.clone();
        diff.scale(lambda);
        x.add(diff);

        return x;
    }

    /**
     * test if a given point is on the left side
     *
     * @param point
     * @return
     */
    public boolean isPointOnTheLeft(Point2d point){
        return direction.y*(point.x-this.point.x)-direction.x*(point.y-this.point.y) < 0;
    }

    /**
     * returns a Line that is parallel to this, lies on the left side and has the given distance
     * (negative distance = right side)
     * 
     * @param distance
     * @return
     */
    public Line getLeftParallel(final double distance){

        Vector2d left = this.getOrthogonalLine().direction;
        left.normalize();

        // make sure its on the left side
        Point2d pPlusLeft = new Point2d(this.point);
        pPlusLeft.add(left);
        if( !isPointOnTheLeft( pPlusLeft ))
        {
            left.scale(-1);
        }

        // go distance further
        left.scale(distance);

        // add to point
        pPlusLeft.x = point.x; pPlusLeft.y = point.y;
        pPlusLeft.add(left);

        // make new Line
        Line parallel = new Line(pPlusLeft, direction);
        return parallel;
    }

    /**
     * cut 2 lines
     * 
     * @param other
     * @return null, if parallel, a point on boths lines otherwise
     */
    public Point2d cut(final Line other){
      double[][] arrayA = {{direction.x,other.direction.x},{direction.y,other.direction.y}};
      Matrix A = new Matrix(arrayA);

      double[][] arrayB = {{other.point.x - point.x},{other.point.y - point.y}};
      Matrix b = new Matrix(arrayB);

      try{
          Matrix x = A.solve(b);
          double lambda = x.get(0, 0);

          Vector2d d = new Vector2d(direction);
          d.scale(lambda);

          Point2d result = new Point2d(point);
          result.add(d);

          return result;
        }
      catch(Exception e){
          // Matrix is singular, no cut!

          return null;
      }
    }

    /**
     * calc the distance of this line to a given Point
     * @param p
     * @return
     */
    public double getDistance(final Point2d p){
        Point2d nearest = getNearestPointOnLine(p);
        return nearest.distance(p);
    }

    /**
     * get the point on this line with the lowest distance to p
     * @param p
     * @return
     */
    public Point2d getNearestPointOnLine(final Point2d p){

        Vector2d orth = MathUtil.getOrthogonalDirection(direction);
        Line other = new Line (p, orth);

        Point2d cut = this.cut(other);

        if( cut == null){
            System.out.println("Line.getNearestPointOnLine failed!!");
            System.out.println("big fail: line is" + this.point + "lambda*" + this.direction + "; point is: "+p);
        }

        return cut;
    }

    /**
     * check if a point lies on the line
     * @param p the point to test
     * @return true iff the Point lies on the Line OR very, very close
     */
    public boolean contains(final Point2d p){
        return this.getDistance(p) < 0.0000000001;
    }


}
