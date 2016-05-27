/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.evo;

/**
 *
 * @author Jan Quadflieg
 */
public interface Individual
extends Comparable<Individual>{

    public void copyFrom(Individual src, boolean copyMetaData)
            throws Exception;

    public Individual createIndividual(String s)
            throws Exception;

    public void evaluate(String host, int port, int maxTicks, boolean zip)
            throws Exception;

    public double getFitness();

    public String getPath();

    public EvoResults getResults();

    public void randomize(java.util.Random random)
            throws Exception;

    public void recombinate(java.util.Random random, Individual v1, Individual v2)
            throws Exception;

    public void mutate(java.util.Random random, double step)
            throws Exception;
}