/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.functions;

/**
 *
 * @author quad
 */
public interface Interpolator {
    public double getXmin();
    public double getXmax();
    public double interpolate(double v);
}