/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.classification;

import de.delbrueg.math.Circle;

/**
 *
 * @author Tim
 */
public class CircleSituation {

    private final double rating;

    /** approximation circle for left track side **/
    private final de.delbrueg.math.Circle leftCircle;

    /** approximation circle for right track side **/
    private final de.delbrueg.math.Circle rightCircle;

    /** approximation circle for track center (pos=0) **/
    private final de.delbrueg.math.Circle centerCircle;

    /** approximation circle for track center weighted by current track position **/
    private final de.delbrueg.math.Circle positionCircle;

    public CircleSituation(CircleSituation sit) {
        this(sit.leftCircle,sit.rightCircle,sit.centerCircle,sit.positionCircle,sit.rating);
    }

    public CircleSituation(Circle leftCircle, Circle rightCircle, Circle centerCircle, Circle positionCircle, double rating) {
        this.leftCircle = leftCircle;
        this.rightCircle = rightCircle;
        this.centerCircle = centerCircle;
        this.positionCircle = positionCircle;
        this.rating = rating;
    }

    public boolean isValid() {
        return leftCircle != null &&
                rightCircle != null &&
                centerCircle != null &&
                positionCircle != null;
    }


    /** approximation circle for left track side **/
    public Circle getLeftCircle() {
        return new Circle(leftCircle);
    }

    /** approximation circle for right track side **/
    public Circle getRightCircle() {
        return new Circle(rightCircle);
    }

    /** approximation circle for track center (pos=0) **/
    public Circle getCenterCircle() {
        return new Circle(centerCircle);
    }

    /** approximation circle for track center weighted by current track position (pos=0) **/
    public Circle getPositionCircle() {
        return new Circle(positionCircle);
    }

    /** sum of squared errors **/
    public double getQuadraticFailure() {
        if( !isValid())
            return Double.MAX_VALUE;
        return rating;
    }

    
}
