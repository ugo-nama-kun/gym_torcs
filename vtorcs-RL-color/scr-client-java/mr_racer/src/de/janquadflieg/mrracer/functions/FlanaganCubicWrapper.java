/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.functions;

import flanagan.interpolation.CubicSpline;

/**
 *
 * @author quad
 */
public class FlanaganCubicWrapper
implements Interpolator{
    private CubicSpline spline;

    public FlanaganCubicWrapper(CubicSpline s){
        this.spline = s;
    }

    @Override
    public double getXmin(){
        return this.spline.getXmin();
    }
    
    @Override
    public double getXmax(){
        return this.spline.getXmax();

    }

    @Override
    public double interpolate(double v){
        return this.spline.interpolate(v);        
    }
}
