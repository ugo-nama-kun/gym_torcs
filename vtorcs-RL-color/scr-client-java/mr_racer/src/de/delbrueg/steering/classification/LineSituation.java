/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.steering.classification;
import de.delbrueg.math.*;

/**
 *
 * @author Tim
 */
public class LineSituation {

    /** approximation Line for left track side **/
    private final de.delbrueg.math.Line leftLine;

    /** approximation Line for right track side **/
    private final de.delbrueg.math.Line rightLine;

    /** approximation Line for track center (pos=0) **/
    private final de.delbrueg.math.Line centerLine;

    /** approximation Line for track center weighted by current track position **/
    private final de.delbrueg.math.Line positionLine;

    /** sum quadratic failure **/
    private final double rating;

    public LineSituation(LineSituation sit) {
        this(sit.leftLine,sit.rightLine,sit.centerLine,sit.positionLine,sit.rating);
    }

    public LineSituation(Line leftLine, Line rightLine, Line centerLine, Line positionLine, double rating) {
        this.leftLine = leftLine;
        this.rightLine = rightLine;
        this.centerLine = centerLine;
        this.positionLine = positionLine;
        this.rating = rating;
    }

    public boolean isValid() {
        return leftLine != null &&
                rightLine != null &&
                centerLine != null &&
                positionLine != null;
    }


    /** approximation Line for left track side **/
    public Line getLeftLine() {
        return leftLine;
    }

    /** approximation Line for right track side **/
    public Line getRightLine() {
        return rightLine;
    }

    /** approximation Line for track center (pos=0) **/
    public Line getCenterLine() {
        return centerLine;
    }

    /** approximation Line for track center weighted by current track position (pos=0)**/
    public Line getPositionLine() {
        return positionLine;
    }

    /** sum quadratic failure **/
    public double getQuadraticFailure() {
        if( ! isValid() )
            return Double.MAX_VALUE;
        
        return rating;
    }

    
}
