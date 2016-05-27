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
public class RankSumComparator
implements Comparator<Individual2011>{    

    @Override
    public int compare(Individual2011 i1, Individual2011 i2){
        int f1 = i1.rankSum;
        int f2 = i2.rankSum;

        if(f1 < f2){
            return -1;

        } else if(f1 > f2){
            return 1;

        } else {
            return 0;
        }
    }
}
