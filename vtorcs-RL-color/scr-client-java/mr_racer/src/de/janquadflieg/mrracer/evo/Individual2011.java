/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.evo;

import java.util.HashMap;
import java.util.Properties;
import java.util.StringTokenizer;

/**
 *
 * @author quad
 */
public class Individual2011
implements Comparable<Individual2011>{
    public String source = "";
    public Properties properties;
    public HashMap<String, Double> fitness = new HashMap<String, Double>();
    public int indNR = 0;
    public int indID = 0;
    public int rankSum = 0;
    public double fitnessSum = 0.0;
    public HashMap<String, Integer> ranks = new HashMap<String, Integer>();

    public Individual2011(){
    }

    public Individual2011(String s, String[] tracks){
        StringTokenizer tokenizer = new StringTokenizer(s, " ");

        String token = tokenizer.nextToken();
        
        // id
        this.indID = Integer.parseInt(token);
        this.indNR = indID-1;

        // finess values for the various tracks
        for(int i=0; i < tracks.length; ++i){
            token = tokenizer.nextToken();
            fitness.put(tracks[i], Double.parseDouble(token));
        }

        // source
        source = tokenizer.nextToken();

        if(source.contains("Alpine")){
            source = "Alpine";
        }
        if(source.contains("Wheel2")){
            source = "Wheel2";
        }
    }

    @Override
    public int compareTo(Individual2011 o2){
        if(fitness.entrySet().size() > 1 || o2.fitness.entrySet().size() > 1){
            throw new ClassCastException("More than one fitness value!");
        }

        double f1 = fitness.get(fitness.keySet().iterator().next());
        double f2 = o2.fitness.get(o2.fitness.keySet().iterator().next());

        if(f1 < f2){
            return -1;

        } else if(f1 > f2){
            return 1;

        } else {
            return 0;
        }
    }    
}