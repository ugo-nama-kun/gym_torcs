/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.network;

/**
 *
 * @author Jan Quadflieg
 */
public interface ConnectionListener {
    public void newStatistics(ConnectionStatistics data);

    public void stopped(boolean requested);
}