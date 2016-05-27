/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.evo;

import java.util.*;

/**
 *
 * @author quad
 */
public class FitnessComparator
implements Comparator<Individual2011>{
    private String track = "";

    public FitnessComparator(String s){
        this.track = s;
    }

    @Override
    public int compare(Individual2011 i1, Individual2011 i2){
        double f1 = i1.fitness.get(track);
        double f2 = i2.fitness.get(track);

        if(f1 < f2){
            return -1;

        } else if(f1 > f2){
            return 1;

        } else {
            return 0;
        }
    }
}
