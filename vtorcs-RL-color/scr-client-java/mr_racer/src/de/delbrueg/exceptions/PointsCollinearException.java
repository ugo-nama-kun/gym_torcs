/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.delbrueg.exceptions;

/**
 * Occurs, if points are collinear so that a circle is not defined
 * @author Tim
 */
public class PointsCollinearException extends Exception{
    final double ax; final double ay;
    final double bx; final double by;
    final double cx; final double cy;

    /*
     * constructor with the 3 points that are collinear
     */
    public PointsCollinearException(double ax, double ay, double bx, double by, double cx, double cy) {
        this.ax = ax;
        this.ay = ay;
        this.bx = bx;
        this.by = by;
        this.cx = cx;
        this.cy = cy;
    }
}
