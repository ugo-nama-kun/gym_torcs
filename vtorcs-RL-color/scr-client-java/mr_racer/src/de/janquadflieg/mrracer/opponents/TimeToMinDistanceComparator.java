/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.opponents;

import java.util.Comparator;

/**
 *
 * @author quad
 */
public class TimeToMinDistanceComparator
implements Comparator<Opponent>{

    @Override
    public int compare(Opponent o1, Opponent o2){
        double f1 = o1.getTimeToMinDistance();
        double f2 = o2.getTimeToMinDistance();

        if(f1 < f2){
            return -1;

        } else if(f1 > f2){
            return 1;

        } else {
            return 0;
        }
    }
}
