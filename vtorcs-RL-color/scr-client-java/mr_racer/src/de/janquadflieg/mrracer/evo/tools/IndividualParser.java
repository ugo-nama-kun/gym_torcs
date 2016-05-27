/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.evo.tools;

import de.janquadflieg.mrracer.evo.Individual2011;

/**
 *
 * @author quad
 */
public interface IndividualParser {
    public Individual2011 parse(String line, String track);
}
