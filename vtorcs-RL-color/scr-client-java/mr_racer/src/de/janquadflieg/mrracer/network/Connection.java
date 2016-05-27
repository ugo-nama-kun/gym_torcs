/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.network;

/**
 *
 * @author Jan Quadflieg
 */
public interface Connection {

    public void addConnectionListener(ConnectionListener l);
    public void removeConnectionListener(ConnectionListener l);
    public void stop();
}
