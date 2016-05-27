/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.math;

import Jama.Matrix;
import de.delbrueg.math.Circle.RotationDirection;
import java.security.InvalidParameterException;
import java.util.Collection;
import javax.vecmath.Point2d;
import javax.vecmath.Tuple2d;
import javax.vecmath.Vector2d;

/**
 *
 * @author Tim
 */
public final class MathUtil {

    /**
     * calc a normalised copy of vector
     *
     * @param p the vector to normalise
     * @return normalised version
     */
     public static <T extends Tuple2d> T normalize( final T p){
        double length = Math.sqrt(p.getX()*p.getX() + p.getY()*p.getY());

        T x = (T)p.clone();
        x.set(p.getX(), p.getY());
        x.scale(1/length);

        return x;
    }


    /**
     * calc a vector orthogonal to the given one
     *
     * @param dir the given vector
     * @return a normalised vector orthogonal to dir
     */
    public static <T extends Tuple2d> T getOrthogonalDirection(final T dir){

        T orth = (T)dir.clone();

        if(dir.x == 0)
            orth.set(1,0);
        else if (dir.y == 0)
            orth.set(0,1);
        else{
            orth.set( -dir.y / dir.x, 1);
        }

        return normalize(orth);
    }



    /**
     * rotate the given point around our center with given direction and angle
     * @param point the point to rotate
     * @param center the point around which we want to rotate
     * @param dir direction to take (clockwise, counter-clockwise)
     * @param angle in radians
     * @return the rotated point
     */
    public static Point2d rotatePoint(final Point2d point, final Point2d center, final RotationDirection dir, final double angle){

        // copy point
        Point2d p = new Point2d(point);

        // translate so that our center is the origin
        p.sub(center);

        // rotMatrix * point
        Matrix rotMatrix = getRotationMatrix(angle, dir);
        Matrix pointAsMatrix = tupleToMatrix(p);
        Matrix result = rotMatrix.times(pointAsMatrix);

        // translate back
        Point2d target = matrixToPoint(result);
        target.add(center);

        return target;
    }


    /**
     * rotate the given vector around our center with given direction and angle
     * @param vector the vector to rotate
     * @param dir direction to take (clockwise, counter-clockwise)
     * @param angle in radians
     * @return the rotated vector
     */
    public static Vector2d rotateVector(final Vector2d vector, final RotationDirection dir, final double angle){

        // rotMatrix * vector
        Matrix rotMatrix = getRotationMatrix(angle, dir);
        Matrix pointAsMatrix = tupleToMatrix(vector);
        Matrix result = rotMatrix.times(pointAsMatrix);

        // translate back
        Vector2d target = matrixToVector(result);

        return target;
    }



    /**
     * construct 2D rotation matrix with angle and direction
     *
     * (use Jama.Matrix because GMatrix cannot multiply with a vector - idiots!)
     *
     * @param angle radian rotation angle
     * @param dir (clockwise, counter-clockwise)
     * @return the 2x2 rotation matrix
     */
    public static Jama.Matrix getRotationMatrix( final double angle, final RotationDirection dir ){

        Matrix m;

        if(dir == RotationDirection.counter_clockwise){           // counter-clockwise
            double[][] values = {{Math.cos(angle),-Math.sin(angle)},{Math.sin(angle),Math.cos(angle)}};
            m = Matrix.constructWithCopy(values);
        }
        else{          // clockwise
            double[][] values = {{Math.cos(angle),Math.sin(angle)},{-Math.sin(angle),Math.cos(angle)}};
            m = Matrix.constructWithCopy(values);
        }

        return m;
    }


    /**
     *
     * @param m Matrix with 2 rows, one column (2x1)
     * @return Point with same coordinates as in Matrix
     * @throws InvalidParameterException if the Matrix has wrong dimensions
     */
    public static Point2d matrixToPoint(final Matrix m) throws InvalidParameterException{
        if( m.getColumnDimension() != 1)
            throw new InvalidParameterException("Circle.matrixToPoint: wrong number of columns: "+m.getColumnDimension());
        if( m.getRowDimension() != 2)
            throw new InvalidParameterException("Circle.matrixToPoint: wrong number of rows: "+m.getRowDimension());

        Point2d point = new Point2d(m.get(0, 0),m.get(1, 0));

        return point;
    }



    /**
     *
     * @param m Matrix with 2 rows, one column (2x1)
     * @return Vector2d with same coordinates as in Matrix
     * @throws InvalidParameterException if the Matrix has wrong dimensions
     */
    public static Vector2d matrixToVector(final Matrix m) throws InvalidParameterException{
        if( m.getColumnDimension() != 1)
            throw new InvalidParameterException("Circle.matrixToPoint: wrong number of columns: "+m.getColumnDimension());
        if( m.getRowDimension() != 2)
            throw new InvalidParameterException("Circle.matrixToPoint: wrong number of rows: "+m.getRowDimension());

        Vector2d vector = new Vector2d(m.get(0, 0),m.get(1, 0));

        return vector;
    }


    /**
     * constructs a Matrix with 2 rows, 1 column from a point (2x1)
     * @param p the point
     * @return 1x2 Matrix
     */
    public static Matrix tupleToMatrix(final Tuple2d p){
        double[][] ps = {{p.getX()},{p.getY()}};

        return new Matrix(ps);
    }

    /**
     * sums up all elements of list, returns as double
     * @param <T> The type of Summation elements
     * @param list A list of T
     * @return the sum
     */
    public static <T extends Number> double sum_double(final Collection<T> list){
        double _sum = 0;
        
        for( T t: list){
            _sum += t.doubleValue();
        }

        return _sum;
    }


    /**
     * sums up all elements of list, returns as int
     * @param <T> The type of Summation elements
     * @param list A list of T
     * @return the sum
     */
    public static <T extends Number> int sum_int(final Collection<T> list){
        int _sum = 0;

        for( T t: list){
            _sum += t.intValue();
        }

        return _sum;
    }

}
